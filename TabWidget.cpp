#include "TabWidget.h"

static TabWidget *inst = 0;
TabWidget::TabWidget()
{
    Q_ASSERT(!inst);
    inst = this;
    tabBar()->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(tabBar(), SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(onCustomContextMenuRequested(QPoint)));

    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(onCloseRequested(int)));
    connect(&mShortcuts, SIGNAL(activated(int)), this, SLOT(onShortcut(int)));
    handleAction(NewTab);
    setFocusPolicy(Qt::NoFocus);
    setElideMode(Qt::ElideRight);
    setStyleSheet("QTabWidget::pane { margin-top: 1px solid #000000 }");
    setDocumentMode(true);
    setTabsClosable(true);
    setMovable(true);
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
        if (currentWidget()) {
            QTimer::singleShot(0, currentWidget(), SLOT(setFocus()));
            currentWidget()->setFocus();
        }
        setShortcutsEnabled(true);
        break;
    case QEvent::WindowDeactivate: {
        setShortcutsEnabled(false);
        const int c = count();
        for (int i=0; i<c; ++i) {
            container(i)->stopFocusTimer();
        }
        break; }
    default:
        break;
    }
    return QTabWidget::event(e);
}

void TabWidget::setShortcutsEnabled(bool on)
{
    if (on == !mShortcutIds.isEmpty())
        return;
    if (on) {
        mShortcutIds[mShortcuts.registerShortcut(57, 0x5)] = NewTab;
        mShortcutIds[mShortcuts.registerShortcut(28, 0x5)] = NewTab;
        for (int i=0; i<10; ++i) {
            mShortcutIds[mShortcuts.registerShortcut(10 + i, 0x8)] = static_cast<Action>(Select1 + i);
        }
        mShortcutIds[mShortcuts.registerShortcut(113, 0x8)] = SelectLeft;
        mShortcutIds[mShortcuts.registerShortcut(114, 0x8)] = SelectRight;
    } else {
        for (QHash<int, Action>::const_iterator it = mShortcutIds.begin(); it != mShortcutIds.end(); ++it) {
            mShortcuts.unregisterShortcut(it.key());
        }
        mShortcutIds.clear();
    }
}

QSize TabWidget::sizeHint() const
{
    return QSize(640, 480);
}

void TabWidget::focusInEvent(QFocusEvent *e)
{
    QTabWidget::focusInEvent(e);
    currentWidget()->setFocus();
    QTimer::singleShot(0, currentWidget(), SLOT(setFocus()));
}

void TabWidget::showEvent(QShowEvent *e)
{
    QTabWidget::showEvent(e);
    QTimer::singleShot(0, currentWidget(), SLOT(setFocus()));
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
            const QString name = QInputDialog::getText(this, "Rename tab", "Rename tab",
                                                       QLineEdit::Normal, tabBar()->tabText(tab), &ok);
            if (ok) {
                container(tab)->setExplicitName(name);
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

void TabWidget::onTitleBarChanged(Container *c, const QString &name)
{
    setTabText(indexOf(c), name);
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
