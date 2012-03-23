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
    Container(QProcess *proc, int timerInterval, QWidget *parent = 0);
    virtual ~Container();
    bool updateTitleBar(Window window);
    void setExplicitName(const QString &name);
    bool hasExplicitName() const { return mExplicitName; }

    QString text() const;

signals:
    void titleBarChanged(Container *container, const QString &name);

public slots:
    void onClientEmbedded();
    void setXFocus();

private:
    QProcess *mProcess;
    bool mExplicitName;
    const int mTimerInterval;
    QString mText;
};


#endif
