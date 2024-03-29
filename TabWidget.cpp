#include <QX11Info>
#include "TabWidget.h"
#include "Container.h"
#include "Application.h"

static inline Application* app()
{
    return static_cast<Application*>(qApp);
}

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
    connect(app(), SIGNAL(shortcutActivated(int)), this, SLOT(onShortcut(int)));
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
    mShowIndexes = settings.value("showIndexes", false).toBool();
    mTimerInterval = settings.value("timerInterval", 100).toInt();
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

QSize TabWidget::sizeHint() const
{
    return QSize(640, 480);
}

void TabWidget::focusInEvent(QFocusEvent *e)
{
    QTabWidget::focusInEvent(e);
}

void TabWidget::showEvent(QShowEvent *e)
{
    QTabWidget::showEvent(e);
    static bool first = true;
    if (first) {
        const QByteArray g = QSettings().value("geometry").toByteArray();
        if (!g.isEmpty())
            restoreGeometry(g);
        first = false;
    }
    if (isVisible()) {
        Container* c = container(currentIndex());
        c->setFocus();
    }
}

void TabWidget::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == mFocusTimer.timerId())
        enableFocus(true);
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
        mFocusTimer.start(mTimerInterval, this);
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
        QAction *newtab = menu.addAction("&New");
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
        } else if (action == newtab) {
            handleAction(NewTab);
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
    Container* c = container(idx);
    if (c)
        c->setXFocus();
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
    Container *t = new Container(p, this);
    connect(t, SIGNAL(titleBarChanged(Container*, QString)),
            this, SLOT(onTitleBarChanged(Container*, QString)));
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

void TabWidget::moveEvent(QMoveEvent *e)
{
    if (isVisible()) {
        const QByteArray g = saveGeometry();
        QSettings().setValue("geometry", g);
    }
    QTabWidget::moveEvent(e);
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

void TabWidget::enableShortcuts(bool enable)
{
    if (enable) {
        app()->clearShortcuts();
        mShortcutIds.clear();
        foreach(const KeyBinding &kb, mKeyBindings)
            mShortcutIds[app()->registerShortcut(kb.keyCode, kb.state)] = kb.action;
    } else {
        app()->clearShortcuts();
        mShortcutIds.clear();
    }
}

void TabWidget::enableFocus(bool enable)
{
    if (enable) {
        QApplication::setActiveWindow(this);
        container(currentIndex())->setXFocus();
    }
    enableShortcuts(enable);
}

WId TabWidget::currentTabHandle() const
{
    return container(currentIndex())->clientWinId();
}
