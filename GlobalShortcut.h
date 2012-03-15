#ifndef GLOBALSHORTCUT_H
#define GLOBALSHORTCUT_H

#include <QObject>

// This class is not reentrant!

class GlobalShortcutPrivate;

class GlobalShortcut : public QObject
{
    Q_OBJECT
public:
    GlobalShortcut(QObject* parent = 0);
    ~GlobalShortcut();

    int registerShortcut(int keycode, int modifier);
    void unregisterShortcut(int id);

signals:
    void activated(int id);

private:
    GlobalShortcutPrivate* priv;

    friend class GlobalShortcutPrivate;
};

#endif // GLOBALSHORTCUT_H
