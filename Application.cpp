#include <QX11Info>
#include <QDateTime>
#include "Application.h"
#include "TabWidget.h"

Application::Application(int& argc, char** argv)
    : QApplication(argc, argv)
{
}

bool Application::x11EventFilter(XEvent* event)
{
    //qDebug() << QDateTime::currentDateTime().toString() << "event filtering" << event->type;

    switch (event->type) {
    case PropertyNotify: {
        static Atom wmname = XInternAtom(QX11Info().display(), "WM_NAME", True);
        if (event->xproperty.atom == wmname)
            TabWidget::instance()->onPropertyNotify(event->xany.window);
        break; }
    case FocusIn:
    case EnterNotify:
        m_timer.start(50, this);
        break;
    default:
        break;
    }

    return QApplication::x11EventFilter(event);
}

void Application::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == m_timer.timerId()) {
        m_timer.stop();
        //qDebug() << "timer fired";
        TabWidget::instance()->ensureFocus();
    }
}
