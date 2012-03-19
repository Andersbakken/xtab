#ifndef TabWidget_h
#define TabWidget_h

#include <QtGui>
#include "GlobalShortcut.h"
#include <X11/X.h>
#include <X11/Xlib.h>

class Container;
class TabWidget : public QTabWidget
{
    Q_OBJECT;
    Q_ENUMS(KeyState)
    Q_ENUMS(Action)
public:
    enum KeyState {
        Shift = 0x1,
        Control = 0x4,
        Alt = 0x8
    };

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
    TabWidget();
    virtual ~TabWidget();
    static TabWidget *instance();
    virtual bool event(QEvent *e);
    virtual QSize sizeHint() const;
    virtual void focusInEvent(QFocusEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void tabRemoved(int index);
    virtual void tabInserted(int index);
    virtual void resizeEvent(QResizeEvent *e);
    void handleAction(Action action);
    void onPropertyNotify(Window window);
    Container *container(int idx) const;
    void enableXTab(bool on);
    void setShowIndexes(bool on);
public slots:
    void updateTabIndexes();
    void enableXTab() { enableXTab(true); }
    void disableXTab() { enableXTab(false); }
    void onCustomContextMenuRequested(const QPoint &pos);
    void onCloseRequested(int idx);
    void onTitleBarChanged(Container *c, const QString &name);
    void onShortcut(int id);
    void onCurrentChanged(int idx);
    void onContainerDestroyed();
    void newTab();
private:
    GlobalShortcut mShortcuts;
    struct KeyBinding {
        KeyBinding(unsigned k = 0, unsigned s = 0, Action a = NoAction)
            : keyCode(k), state(s), action(a)
        {}
        unsigned keyCode, state;
        Action action;
    };
    KeyBinding decodeKeyBinding(const QString &key, const QString &value) const;

    QList<KeyBinding> mKeyBindings;
    QHash<int, Action> mShortcutIds;
    int mTimerInterval;
    bool mShowIndexes;
};


#endif
