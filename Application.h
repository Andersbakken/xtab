#ifndef APPLICATION_H
#define APPLICATION_H

#include <QtGui>
#include <X11/Xlib.h>

typedef QPair<int, int> IntPair;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int& argc, char** argv);
    ~Application();

    virtual bool x11EventFilter(XEvent* event);

    int registerShortcut(int keycode, int modifier);
    void unregisterShortcut(int id);
    void clearShortcuts();

signals:
    void shortcutActivated(int id);

private:
    QHash<int, QList<IntPair> > m_shortcuts;
};

#endif
