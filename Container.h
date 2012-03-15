#ifndef Container_h
#define Container_h

#include <QtGui>
#include <QX11EmbedContainer>
#include <X11/X.h>
#include <X11/Xlib.h>

class Container : public QX11EmbedContainer
{
    Q_OBJECT;
public:
    Container(QProcess *proc, QWidget *parent = 0);
    virtual ~Container();
    void onFocusIn();
    bool updateTitleBar(Window window);
    void setExplicitName(const QString &name);
    void stopFocusTimer();

    virtual void focusInEvent(QFocusEvent *e);
    virtual void focusOutEvent(QFocusEvent *e);
    virtual void timerEvent(QTimerEvent *e);
signals:
    void titleBarChanged(Container *container, const QString &name);
public slots:
    void onClientEmbedded();
    void setXFocus();
private:
    QBasicTimer timer;
    QProcess *mProcess;
    bool mExplicitName;
};


#endif
