#ifndef TabWidget_h
#define TabWidget_h

#include <QtGui>
#include "GlobalShortcut.h"
#include "Container.h"

class TabWidget : public QTabWidget
{
    Q_OBJECT;
public:
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
    void setShortcutsEnabled(bool on);
    virtual bool event(QEvent *e);
    virtual QSize sizeHint() const;
    virtual void focusInEvent(QFocusEvent *e);
    virtual void showEvent(QShowEvent *e);
    virtual void tabRemoved(int index);
    virtual void resizeEvent(QResizeEvent *e);
    void handleAction(Action action);
    void onPropertyNotify(Window window);
    Container *container(int idx) const;
public slots:
    void onCustomContextMenuRequested(const QPoint &pos);
    void onCloseRequested(int idx);
    void onTitleBarChanged(Container *c, const QString &name);
    void onShortcut(int id);
    void newTab();
private:
    GlobalShortcut mShortcuts;
    QHash<int, Action> mShortcutIds;
};


#endif
