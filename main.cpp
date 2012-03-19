#include <QtGui>
#include <QX11Info>
#include "Container.h"
#include "TabWidget.h"
#include <X11/X.h>
#include <X11/Xlib.h>

bool x11EventFilter(void *message, long *)
{
    static Atom wmname = XInternAtom(QX11Info().display(), "WM_NAME", True);
    XEvent *event = reinterpret_cast<XEvent *>(message);
    if (event->type == PropertyNotify
        && event->xproperty.atom == wmname) {
        TabWidget::instance()->onPropertyNotify(event->xany.window);
    }
    return false;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("XTab");
    app.setApplicationName("XTab");
    app.setOrganizationDomain("https://github.com/Andersbakken/xtab");
    app.setWindowIcon(QIcon(":/terminal.png"));
    TabWidget window;
    app.setEventFilter(x11EventFilter);
    window.show();
    return app.exec();
}
