#include <QtGui>
#include <QX11EmbedContainer>
#include <QX11Info>
#include "GlobalShortcut.h"
#include <X11/X.h>
#include <X11/Xlib.h>

class Container : public QX11EmbedContainer
{
    Q_OBJECT;
public:
    Container(QWidget *parent = 0)
        : QX11EmbedContainer(parent)
    {
        connect(this, SIGNAL(clientClosed()), this, SLOT(deleteLater()));
    }
    void focusInEvent(QFocusEvent *e)
    {
        QX11EmbedContainer::focusInEvent(e);
        setXFocus();
        timer.start(10, this);
    }
    void focusOutEvent(QFocusEvent *e)
    {
        QX11EmbedContainer::focusOutEvent(e);
        timer.stop();
    }
public slots:
    void setXFocus()
    {
        XSetInputFocus(x11Info().display(), clientWinId(), RevertToParent, CurrentTime);
    }

    void timerEvent(QTimerEvent *e)
    {
        if (e->timerId() == timer.timerId()) {
            setXFocus();
            timer.stop();
        }
    }
private:
    QBasicTimer timer;
};

class MainWindow : public QTabWidget
{
    Q_OBJECT;
public:
    enum Action {
        NoAction,
        NewTab,
        SelectLeft,
        SelectRight,
        Select1,
        Select2,
        Select3,
        Select4,
        Select5,
        Select6,
        Select7,
        Select8,
        Select9,
        Select10
    };
    MainWindow()
    {
        connect(&mShortcuts, SIGNAL(activated(int)), this, SLOT(onShortcut(int)));
        handleAction(NewTab);
        setFocusPolicy(Qt::NoFocus);
    }
    bool event(QEvent *e)
    {
        switch (e->type()) {
        case QEvent::WindowActivate:
            QTimer::singleShot(0, currentWidget(), SLOT(setFocus()));
            currentWidget()->setFocus();
            setShortcutsEnabled(true);
            break;
        case QEvent::WindowDeactivate:
            setShortcutsEnabled(false);
            break;
        default:
            break;
        }
        return QTabWidget::event(e);
    }
    void setShortcutsEnabled(bool on)
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
    QSize sizeHint() const
    {
        return QSize(640, 480);
    }

    void focusInEvent(QFocusEvent *e)
    {
        QTabWidget::focusInEvent(e);
        currentWidget()->setFocus();
        QTimer::singleShot(0, currentWidget(), SLOT(setFocus()));
    }
    void showEvent(QShowEvent *e)
    {
        QTabWidget::showEvent(e);
        QTimer::singleShot(0, currentWidget(), SLOT(setFocus()));
    }
    void handleAction(Action action)
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
    virtual void tabRemoved(int index)
    {
        QTabWidget::tabRemoved(index);
        if (!count())
            close();
    }

public slots:
    void onShortcut(int id)
    {
        handleAction(mShortcutIds.value(id, NoAction));
    }

    void newTab()
    {
        Container *t = new Container(this);
        addTab(t, QString::number(count()));
        setCurrentWidget(t);
        QProcess::startDetached("xterm", QStringList() << "-into" << QString::number(t->internalWinId()));
    }
private:
    GlobalShortcut mShortcuts;
    QHash<int, Action> mShortcutIds;
};

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow window;
    window.show();
    return app.exec();
}
