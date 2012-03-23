#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtGui>
#include <X11/Xlib.h>

class Application : public QApplication
{
public:
    Application(int& argc, char** argv);

    virtual bool x11EventFilter(XEvent* event);

protected:
    virtual void timerEvent(QTimerEvent* event);

private:
    QBasicTimer m_timer;
    int m_timerInterval;
};

#endif
