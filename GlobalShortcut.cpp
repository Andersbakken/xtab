#include "GlobalShortcut.h"
#include <QWidget>
#include <QHash>
#include <QSet>
#if defined(Q_WS_X11)
#    include <QApplication>
#    include <QTime>
#    include <QAbstractEventDispatcher>
#    include <QX11Info>
#    include <X11/Xlib.h>
#    define EMIT_TRESHOLD 100
#elif defined(Q_OS_MAC)
#    include <Carbon/Carbon.h>
#endif

struct Shortcut
{
    GlobalShortcut* creator;
#if defined(Q_WS_X11)
    unsigned int keycode, modifier;
    QTime lastemitted;
#elif defined(Q_OS_MAC)
    EventHotKeyRef* keyref;
#endif
};

class GlobalShortcutPrivate
{
public:
    GlobalShortcutPrivate(GlobalShortcut* s)
        : shortcut(s)
    {
    }
    ~GlobalShortcutPrivate()
    {
#if defined(Q_WS_X11)
        Display* dpy = QX11Info::display();
#endif

        QHash<int, Shortcut>::iterator it = shortcuts.begin();
        while (it != shortcuts.end()) {
            if (it.value().creator == shortcut) {
#if defined(Q_OS_MAC)
                UnregisterEventHotKey(*(it.value().keyref));
                delete it.value().keyref;
#elif defined(Q_WS_X11)
                XUngrabKey(dpy, it.value().keycode, it.value().modifier, root);
#endif
                it = shortcuts.erase(it);
            } else
                ++it;
        }
    }

    void notify(int code)
    {
        emit shortcut->activated(code);
    }

    GlobalShortcut* shortcut;
    static QHash<int, Shortcut> shortcuts;
#ifdef Q_WS_X11
    static bool dispatcherEventHandler(void* message);
    static Window root;
    static QAbstractEventDispatcher::EventFilter sFilter;
#endif
};

QHash<int, Shortcut> GlobalShortcutPrivate::shortcuts;

#if defined(Q_WS_X11)
Window GlobalShortcutPrivate::root;
QAbstractEventDispatcher::EventFilter GlobalShortcutPrivate::sFilter;

bool GlobalShortcutPrivate::dispatcherEventHandler(void* message)
{
    const XEvent* ev = static_cast<XEvent*>(message);
    if ((ev->type == KeyRelease || ev->type == KeyPress)
        && ev->xkey.window == root) {
        QHash<int, Shortcut>::iterator it = shortcuts.begin();
        QHash<int, Shortcut>::iterator itend = shortcuts.end();
        while (it != itend) {
            if (it.value().keycode == ev->xkey.keycode
                && (ev->xkey.state & it.value().modifier)) {
                if (it.value().lastemitted.elapsed() >= EMIT_TRESHOLD) {
                    if (ev->type == KeyPress) {
                        it.value().lastemitted.start();
                        emit it.value().creator->activated(it.key());
                    }
                }
                return true;
            }
            ++it;
        }
    }
    return sFilter ? sFilter(message) : false;
}
#elif defined(Q_OS_MAC)
static OSStatus hotKeyHandler(EventHandlerCallRef nextHandler, EventRef event, void *userData)
{
    Q_UNUSED(nextHandler)
    Q_UNUSED(userData)

    EventHotKeyID hotKeyID;
    GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(hotKeyID), NULL, &hotKeyID);

    GlobalShortcutPrivate* shortcut = reinterpret_cast<GlobalShortcutPrivate*>(userData);
    shortcut->notify(hotKeyID.id);

    return noErr;
}
#endif

GlobalShortcut::GlobalShortcut(QObject* parent)
    : QObject(parent), priv(new GlobalShortcutPrivate(this))
{
    static bool first = true;
    if (first) {
        first = false;

#if defined(Q_OS_MAC)
        EventTypeSpec spec;

        spec.eventClass = kEventClassKeyboard;
        spec.eventKind = kEventHotKeyPressed;

        InstallApplicationEventHandler(&hotKeyHandler, 1, &spec, priv, NULL);
#elif defined(Q_WS_X11)
        const int screen = qApp->topLevelWidgets().front()->x11Info().screen();
        GlobalShortcutPrivate::root = RootWindow(QX11Info::display(), screen);
        GlobalShortcutPrivate::sFilter = QAbstractEventDispatcher::instance()->setEventFilter(GlobalShortcutPrivate::dispatcherEventHandler);
#endif
    }
}

GlobalShortcut::~GlobalShortcut()
{
    delete priv;
}

int GlobalShortcut::registerShortcut(int keycode, int modifier)
{
    static int id = 1;

    Shortcut shortcut;
    shortcut.creator = this;
#if defined(Q_OS_MAC)
    while (GlobalShortcutPrivate::shortcuts.contains(id))
        ++id;

    EventHotKeyRef* ref = new EventHotKeyRef;
    EventHotKeyID keyid;
    keyid.signature = 1122;
    keyid.id = id;
    RegisterEventHotKey(keycode, modifier, keyid, GetEventDispatcherTarget(), 0, ref);

    shortcut.keyref = ref;
#elif defined(Q_WS_X11)
    shortcut.keycode = keycode;
    shortcut.modifier = modifier;
    shortcut.lastemitted.start();

    Display* dpy = QX11Info::display();
    XGrabKey(dpy, keycode, modifier, GlobalShortcutPrivate::root, False, GrabModeAsync, GrabModeAsync);
#endif
    GlobalShortcutPrivate::shortcuts[id] = shortcut;

    return id++;
}

void GlobalShortcut::unregisterShortcut(int id)
{
    QHash<int, Shortcut>::iterator it = GlobalShortcutPrivate::shortcuts.find(id);
    if (it != GlobalShortcutPrivate::shortcuts.end()) {
        if (it.value().creator != this)
            return;

#if defined(Q_OS_MAC)
        UnregisterEventHotKey(*(it.value().keyref));
        delete it.value().keyref;
#elif defined(Q_WS_X11)
        Display* dpy = QX11Info::display();
        XUngrabKey(dpy, it.value().keycode, it.value().modifier, GlobalShortcutPrivate::root);
#endif
        GlobalShortcutPrivate::shortcuts.erase(it);
    }
}
