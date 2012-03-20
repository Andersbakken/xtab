#include <QX11Info>
#include "TabWidget.h"
#include "Container.h"

class TabBar : public QTabBar
{
public:
    TabBar(QWidget* parent = 0)
        : QTabBar(parent)
    {
    }

    QSize tabSizeHint(int idx) const
    {
        QSize size = QTabBar::tabSizeHint(idx);
        if (count())
            size.setWidth(width() / count());
        return size;
    }
};

static TabWidget *inst = 0;
TabWidget::TabWidget()
    : QTabWidget()
{
    Q_ASSERT(!inst);
    inst = this;
    setTabBar(new TabBar(this));
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onCloseRequested(int)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(onCurrentChanged(int)));
    connect(&mShortcuts, SIGNAL(activated(int)), this, SLOT(onShortcut(int)));
    connect(tabBar(), SIGNAL(tabMoved(int, int)), this, SLOT(updateTabIndexes()));
    setFocusPolicy(Qt::NoFocus);
    setElideMode(Qt::ElideRight);
    setStyleSheet("QTabWidget::pane { margin-top: 1px solid #000000 }");
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
    QSettings settings;
    const bool tabExpanding = settings.value("tabExpanding", true).toBool();
    tabBar()->setExpanding(tabExpanding);
    const bool includeDefaults = settings.value("includeDefaultBindings").toBool();
    mTimerInterval = settings.value("timerInterval", 100).toInt();
    mShowIndexes = settings.value("showIndexes", false).toBool();
    if (settings.value("hideTabBar").toBool())
        tabBar()->hide();
    settings.beginGroup("KeyBindings");
    foreach(const QString &key, settings.childKeys()) {
        KeyBinding kb = decodeKeyBinding(key, settings.value(key).toString());
        if (kb.action) {
            mKeyBindings.append(kb);
        }
    }
    if (mKeyBindings.isEmpty() || includeDefaults) {
        mKeyBindings.append(decodeKeyBinding("Control|Shift+N", "NewTab"));
        mKeyBindings.append(decodeKeyBinding("Control|Shift+T", "NewTab"));
        for (int i=0; i<10; ++i) {
            mKeyBindings.append(decodeKeyBinding("Alt+" + QString::number(i),
                                                 "Select" + QString::number(i == 0 ? 10 : i)));
        }
        mKeyBindings.append(decodeKeyBinding("Alt+Left", "SelectLeft"));
        mKeyBindings.append(decodeKeyBinding("Alt+Right", "SelectRight"));

    }
    handleAction(NewTab);
}

TabWidget::~TabWidget()
{
    inst = 0;
}

TabWidget *TabWidget::instance()
{
    return inst;
}
bool TabWidget::event(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowActivate:
        enableXTab(true);
        break;
    case QEvent::WindowDeactivate:
        enableXTab(false);
        break;
    default:
        break;
    }
    return QTabWidget::event(e);
}

QSize TabWidget::sizeHint() const
{
    return QSize(640, 480);
}

void TabWidget::focusInEvent(QFocusEvent *e)
{
    QTabWidget::focusInEvent(e);
    enableXTab(true);
}

void TabWidget::showEvent(QShowEvent *e)
{
    QTabWidget::showEvent(e);
    QTimer::singleShot(mTimerInterval, currentWidget(), SLOT(setFocus()));
    const QByteArray g = QSettings().value("geometry").toByteArray();
    if (!g.isEmpty())
        restoreGeometry(g);
}

void TabWidget::handleAction(Action action)
{
    int newIdx = -1;
    switch (action) {
    case NoAction:
        break;
    case NewTab:
        newTab();
        break;
    case SelectLeft:
        newIdx = currentIndex() == 0 ? count() - 1 : currentIndex() - 1;
        break;
    case SelectRight:
        newIdx = currentIndex() == count() - 1 ? 0  : currentIndex() + 1;
        break;
    case Select1:
        newIdx = 0;
        break;
    case Select2:
        newIdx = 1;
        break;
    case Select3:
        newIdx = 2;
        break;
    case Select4:
        newIdx = 3;
        break;
    case Select5:
        newIdx = 4;
        break;
    case Select6:
        newIdx = 5;
        break;
    case Select7:
        newIdx = 6;
        break;
    case Select8:
        newIdx = 7;
        break;
    case Select9:
        newIdx = 8;
        break;
    case Select10:
        newIdx = 9;
        break;
    }
    if (newIdx >= 0 && newIdx < count()) {
        setCurrentIndex(newIdx);
    }
}

void TabWidget::tabRemoved(int index)
{
    QTabWidget::tabRemoved(index);
    if (!count()) {
        close();
    } else {
        currentWidget()->setFocus();
    }
}

void TabWidget::onPropertyNotify(Window window)
{
    const int c = count();
    for (int i=0; i<c; ++i) {
        if (container(i)->updateTitleBar(window))
            break;
    }
}

Container *TabWidget::container(int idx) const
{
    return qobject_cast<Container*>(widget(idx));
}

void TabWidget::onCustomContextMenuRequested(const QPoint &pos)
{
    const int tab = tabBar()->tabAt(pos);
    if (tab != -1) {
        QMenu menu;
        QAction *rename = menu.addAction("&Rename");
        QAction *close = menu.addAction("&Close");
        QAction *action = menu.exec(tabBar()->mapToGlobal(pos));
        if (action == rename) {
            bool ok;
            Container *c = container(tab);
            const QString name = QInputDialog::getText(this, "Rename tab", "Rename tab",
                                                       QLineEdit::Normal, c->text(), &ok);
            if (ok) {
                c->setExplicitName(name);
            }
        } else if (action == close) {
            widget(tab)->deleteLater();
        }
    }
}

void TabWidget::onCloseRequested(int idx)
{
    delete widget(idx);
}

void TabWidget::onCurrentChanged(int idx)
{
    setWindowTitle(tabText(idx));
    QTimer::singleShot(mTimerInterval, this, SLOT(enableXTab()));
}

void TabWidget::onTitleBarChanged(Container *c, const QString &name)
{
    const int idx = indexOf(c);
    if (mShowIndexes) {
        updateTabIndexes();
    } else {
        setTabText(idx, name);
    }
    if (currentIndex() == idx)
        setWindowTitle(name);
}

void TabWidget::onShortcut(int id)
{
    handleAction(mShortcutIds.value(id, NoAction));
}

void TabWidget::newTab()
{
    QProcess *p = new QProcess;
    Container *t = new Container(p, mTimerInterval, this);
    connect(t, SIGNAL(titleBarChanged(Container*, QString)),
            this, SLOT(onTitleBarChanged(Container*, QString)));
    connect(t, SIGNAL(clientIsEmbedded()), this, SLOT(enableXTab()),
            Qt::QueuedConnection);
    connect(t, SIGNAL(destroyed()),
            this, SLOT(onContainerDestroyed()), Qt::QueuedConnection);
    addTab(t, QString::number(count()));
    setCurrentWidget(t);
    p->start("xterm", QStringList() << "-into" << QString::number(t->internalWinId()));
}

void TabWidget::onContainerDestroyed()
{
    QApplication::flush();
    QApplication::syncX();
    QApplication::processEvents();
    QApplication::setActiveWindow(this);
}

void TabWidget::resizeEvent(QResizeEvent *e)
{
    if (isVisible()) {
        const QByteArray g = saveGeometry();
        QSettings().setValue("geometry", g);
    }
    QTabWidget::resizeEvent(e);
}
TabWidget::KeyBinding TabWidget::decodeKeyBinding(const QString &key, const QString &value) const
{
    static QRegExp rx("^([A-Za-z|]*\\+)([A-Za-z0-9_-]+)$");
    if (!rx.exactMatch(key)) {
        qWarning("Can't decode key %s", qPrintable(key));
        return KeyBinding();
    }
    KeyBinding kb;
    QString modifiers = rx.cap(1);
    if (!modifiers.isEmpty()) {
        modifiers.chop(1);
        QMetaEnum enumerator = metaObject()->enumerator(metaObject()->indexOfEnumerator("KeyState"));
        foreach(const QString &mod, modifiers.split('|', QString::SkipEmptyParts)) {
            const int s = enumerator.keyToValue(qPrintable(mod));
            if (s == -1) {
                qWarning("Can't decode key state %s", qPrintable(mod));
                return KeyBinding();
            }
            kb.state |= s;
        }
    }
    const KeySym keySym = XStringToKeysym(qPrintable(rx.cap(2)));
    if (keySym) {
        kb.keyCode = XKeysymToKeycode(x11Info().display(), keySym);
    }
    if (!kb.keyCode)
        kb.keyCode = rx.cap(2).toInt();
    if (!kb.keyCode) {
        qWarning("Can't decode key code %s", qPrintable(rx.cap(2)));
        return KeyBinding();
    }

    QMetaEnum actionEnumerator = metaObject()->enumerator(metaObject()->indexOfEnumerator("Action"));
    const int action = actionEnumerator.keyToValue(qPrintable(value));
    if (action == -1) {
        qWarning("Can't decode action %s", qPrintable(value));
        return KeyBinding();
    }
    kb.action = static_cast<Action>(action);
    return kb;
}

void TabWidget::enableXTab(bool on)
{
    if (on) {
        if (mShortcutIds.isEmpty()) {
            foreach(const KeyBinding &kb, mKeyBindings)
                mShortcutIds[mShortcuts.registerShortcut(kb.keyCode, kb.state)] = kb.action;
        }

        if (currentWidget()) {
            QTimer::singleShot(mTimerInterval, currentWidget(), SLOT(setFocus()));
            currentWidget()->setFocus();
        }
    } else {
        for (QHash<int, Action>::const_iterator it = mShortcutIds.begin(); it != mShortcutIds.end(); ++it) {
            mShortcuts.unregisterShortcut(it.key());
        }
        mShortcutIds.clear();
        const int c = count();
        for (int i=0; i<c; ++i) {
            container(i)->stopFocusTimer();
        }
    }
}

void TabWidget::updateTabIndexes()
{
    if (mShowIndexes) {
        const int c = count();
        for (int i=0; i<c; ++i) {
            Container *c = container(i);
            Q_ASSERT(c);
            setTabText(i, QString("(%1) %2").arg(i + 1).arg(c->text()));
        }
    }
}
void TabWidget::tabInserted(int index)
{
    QTabWidget::tabInserted(index);
    updateTabIndexes();
}
