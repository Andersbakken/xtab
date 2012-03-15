#include <QtGui>
#include <QX11EmbedContainer>
#include <QX11Info>
#include "GlobalShortcut.h"
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

template<typename T>
static inline bool readProperty(Display* dpy, Window w, Atom property, QList<T>& data)
{
    data.clear();

    Atom retatom;
    int retfmt;
    unsigned long retnitems, retbytes;
    unsigned char* retprop;

    unsigned long offset = 0;
    int r;
    do {
        r = XGetWindowProperty(dpy, w, property, offset, 200, False, AnyPropertyType,
                               &retatom, &retfmt, &retnitems, &retbytes, &retprop);
        if (r != Success || retatom == None)
            return false;

        Q_ASSERT(retfmt == (sizeof(T) * 8));

        const T* retdata = reinterpret_cast<T*>(retprop);
        for (unsigned long i = 0; i < retnitems; ++i)
            data << retdata[i];
        XFree(retprop);

        switch(retfmt) {
        case 8:
            offset += retnitems * 4;
            break;
        case 16:
            offset += retnitems * 2;
            break;
        case 32:
            offset += retnitems;
            break;
        default:
            qFatal("Invalid format returned in readProperty: %d", retfmt);
        }
    } while (retbytes > 0);

    return true;
}

static inline QByteArray windowName(Display* dpy, Window w)
{
    QByteArray res;

    XClassHint hint;
    if (XGetClassHint(dpy, w, &hint) != 0) {
        if (hint.res_name) {
            res = QByteArray(hint.res_name, strnlen(hint.res_name, 100)).toLower();
            XFree(hint.res_name);
        }
        if (hint.res_class)
            XFree(hint.res_class);
    }

    return res;
}

class Container : public QX11EmbedContainer
{
    Q_OBJECT;
public:
    Container(QProcess *proc, QWidget *parent = 0)
        : QX11EmbedContainer(parent), mProcess(proc)
    {
        connect(this, SIGNAL(clientClosed()), this, SLOT(deleteLater()));
        connect(this, SIGNAL(clientIsEmbedded()), this, SLOT(onClientEmbedded()));
    }
    ~Container()
    {
        if (mProcess->state() != QProcess::NotRunning) {
            mProcess->kill();
            if (!mProcess->waitForFinished(2000)) {
                mProcess->terminate();
            }
        }
        delete mProcess;
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
    bool updateTitleBar(Window window)
    {
        if (window == clientWinId()) {
            const QString name = QString::fromLocal8Bit(windowName(x11Info().display(), window));
            emit titleBarChanged(this, name);
            return true;
        }
        return false;
    }
signals:
    void titleBarChanged(Container *container, const QString &name);
public slots:
    void onClientEmbedded()
    {
        XGrabServer(x11Info().display());
        XWindowAttributes attrib;
        Window id = clientWinId();
        if (!XGetWindowAttributes(x11Info().display(), id, &attrib)) {
            XUngrabServer(x11Info().display());
            qWarning("Couldn't get attributes");
            return;
        }
        XSelectInput(x11Info().display(), id, attrib.your_event_mask | PropertyChangeMask);
        XUngrabServer(x11Info().display());
        updateTitleBar(id);
    }
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
    QProcess *mProcess;
};

class TabWidget : public QTabWidget
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
    TabWidget()
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

    void onPropertyNotify(Window window)
    {
        const int c = count();
        for (int i=0; i<c; ++i) {
            if (qobject_cast<Container*>(widget(i))->updateTitleBar(window))
                break;
        }
    }

public slots:
    void onTitleBarChanged(Container *c, const QString &name)
    {
        printf("[%s] %s:%d: void onTitleBarChanged(Container *c, const QString &name)\n", __func__, __FILE__, __LINE__);
        setTabText(indexOf(c), name);
    }
    void onShortcut(int id)
    {
        handleAction(mShortcutIds.value(id, NoAction));
    }

    void newTab()
    {
        QProcess *p = new QProcess;
        Container *t = new Container(p, this);
        connect(t, SIGNAL(titleBarChanged(Container*, QString)),
                this, SLOT(onTitleBarChanged(Container*, QString)));
        addTab(t, QString::number(count()));
        setCurrentWidget(t);
        p->start("xterm", QStringList() << "-into" << QString::number(t->internalWinId()));
    }
private:
    GlobalShortcut mShortcuts;
    QHash<int, Action> mShortcutIds;
};

TabWidget *tabWidget = 0;
bool x11EventFilter(void *message, long *)
{
    XEvent *event = reinterpret_cast<XEvent *>(message);
    if (event->type == PropertyNotify) {
        tabWidget->onPropertyNotify(event->xany.window);
    }
    return false;
}

#include "main.moc"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TabWidget window;
    tabWidget = &window;
    app.setEventFilter(x11EventFilter);
    window.show();
    return app.exec();
}
