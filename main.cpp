#include <QtGui>
#include <QX11Info>
#include "Application.h"
#include "Container.h"
#include "TabWidget.h"
#include <X11/X.h>
#include <X11/Xlib.h>

int main(int argc, char *argv[])
{
    Application app(argc, argv);
    app.setOrganizationName("XTab");
    app.setApplicationName("XTab");
    app.setOrganizationDomain("https://github.com/Andersbakken/xtab");
    app.setWindowIcon(QIcon(":/terminal.png"));
    TabWidget window;
    window.show();
    return app.exec();
}
