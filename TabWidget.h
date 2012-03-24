#ifndef TabWidget_h
#define TabWidget_h

#include <QtGui>
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
    virtual QSize sizeHint() const;
    virtual void focusInEvent(QFocusEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void tabRemoved(int index);
    virtual void tabInserted(int index);
    virtual void resizeEvent(QResizeEvent *e);
    virtual void moveEvent(QMoveEvent *e);
    virtual void timerEvent(QTimerEvent *e);
    void handleAction(Action action);
    void onPropertyNotify(Window window);
    Container *container(int idx) const;
    void setShowIndexes(bool on);

    void enableFocus(bool enable);
    WId currentTabHandle() const;

public slots:
    void updateTabIndexes();
    void onCustomContextMenuRequested(const QPoint &pos);
    void onCloseRequested(int idx);
    void onTitleBarChanged(Container *c, const QString &name);
    void onShortcut(int id);
    void onCurrentChanged(int idx);
    void onContainerDestroyed();
    void newTab();

private:
    void enableShortcuts(bool enable);

private:
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
    bool mShowIndexes;
    int mTimerInterval;
    QBasicTimer mFocusTimer;
};


#endif
