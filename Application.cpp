#include <QX11Info>
#include <QDateTime>
#include "Application.h"
#include "TabWidget.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
}

Application::~Application()
{
    clearShortcuts();
}

bool Application::x11EventFilter(XEvent* event)
{
    switch (event->type) {
    case KeyPress: {
        const QHash<int, QList<IntPair> >::const_iterator it = m_shortcuts.find(event->xkey.keycode);
        if (it == m_shortcuts.end())
            break;
        foreach(const IntPair& p, it.value()) {
            if (event->xkey.state & p.first) {
                emit shortcutActivated(p.second);
                break;
            }
        }
        break; }
    case PropertyNotify: {
        static Atom wmname = XInternAtom(QX11Info().display(), "WM_NAME", True);
        if (event->xproperty.atom == wmname)
            TabWidget::instance()->onPropertyNotify(event->xany.window);
        break; }
    case FocusIn:
    case FocusOut: {
        const bool in = (event->type == FocusIn);
        //qDebug() << "focus " << (in ? "in" : "out") << QDateTime::currentDateTime().toString() << event->xfocus.window << event->xfocus.mode;
        if (event->xfocus.window == TabWidget::instance()->currentWidget()->internalWinId()) {
            if (!in && event->xfocus.mode == NotifyGrab)
                break;
            TabWidget::instance()->enableFocus(in);
        }
        break; }
    default:
        break;
    }

    return QApplication::x11EventFilter(event);
}

int Application::registerShortcut(int keycode, int modifier)
{
    static int nextId = 0;
    ++nextId;

    Q_ASSERT(qApp->topLevelWidgets().size() == 1);
    Window top = qApp->topLevelWidgets().front()->internalWinId();
    XGrabKey(QX11Info::display(), keycode, modifier, top,
             False, GrabModeAsync, GrabModeAsync);
    m_shortcuts[keycode].append(qMakePair(modifier, nextId));
    return nextId;
}

void Application::unregisterShortcut(int id)
{
    if (qApp->topLevelWidgets().isEmpty())
        return;

    QHash<int, QList<IntPair> >::iterator it = m_shortcuts.begin();
    const QHash<int, QList<IntPair> >::const_iterator end = m_shortcuts.end();
    while (it != end) {
        QList<IntPair>::iterator nit = it.value().begin();
        const QList<IntPair>::const_iterator nend = it.value().end();
        while (nit != nend) {
            if ((*nit).second == id) {
                // hit
                Q_ASSERT(qApp->topLevelWidgets().size() == 1);
                Window top = qApp->topLevelWidgets().front()->internalWinId();
                XUngrabKey(QX11Info::display(), it.key(), (*nit).first, top);
                it.value().erase(nit);
                return;
            }
            ++nit;
        }
        ++it;
    }
}

void Application::clearShortcuts()
{
    if (qApp->topLevelWidgets().isEmpty())
        return;

    Q_ASSERT(qApp->topLevelWidgets().size() == 1);
    Window top = qApp->topLevelWidgets().front()->internalWinId();
    Display* dpy = QX11Info::display();

    QHash<int, QList<IntPair> >::const_iterator it = m_shortcuts.begin();
    const QHash<int, QList<IntPair> >::const_iterator end = m_shortcuts.end();
    while (it != end) {
        foreach(const IntPair& p, it.value()) {
            XUngrabKey(dpy, it.key(), p.first, top);
        }
        ++it;
    }

    m_shortcuts.clear();
}
