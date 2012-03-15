#include "Container.h"
#include <QX11Info>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>


Container::Container(QProcess *proc, QWidget *parent)
    : QX11EmbedContainer(parent), mProcess(proc), mExplicitName(false)
{
    connect(this, SIGNAL(clientClosed()), this, SLOT(deleteLater()));
    connect(this, SIGNAL(clientIsEmbedded()), this, SLOT(onClientEmbedded()));
}

Container::~Container()
{
    if (mProcess->state() != QProcess::NotRunning) {
        mProcess->kill();
        if (!mProcess->waitForFinished(2000)) {
            mProcess->terminate();
        }
    }
    delete mProcess;
}

void Container::focusInEvent(QFocusEvent *e)
{
    QX11EmbedContainer::focusInEvent(e);
    onFocusIn();
}

void Container::onFocusIn()
{
    setXFocus();
    timer.start(10, this);
}

void Container::focusOutEvent(QFocusEvent *e)
{
    QX11EmbedContainer::focusOutEvent(e);
    timer.stop();
}

bool Container::updateTitleBar(Window window)
{
    if (window == clientWinId()) {
        if (!mExplicitName) {
            char* name;
            int status = XFetchName(x11Info().display(), window, &name);
            if (status && name) {
                emit titleBarChanged(this, QString::fromLocal8Bit(name));
                XFree(name);
            }
        }
        return true;
    }
    return false;
}

void Container::setExplicitName(const QString &name)
{
    mExplicitName = !name.isEmpty();
    if (mExplicitName) {
        emit titleBarChanged(this, name);
    } else {
        updateTitleBar(clientWinId());
    }
}

void Container::onClientEmbedded()
{
    XGrabServer(x11Info().display());
    XWindowAttributes attrib;
    Window id = clientWinId();
    if (!XGetWindowAttributes(x11Info().display(), id, &attrib)) {
        XUngrabServer(x11Info().display());
        qWarning("Couldn't get attributes");
        return;
    }
    XSelectInput(x11Info().display(), id, attrib.your_event_mask | PropertyChangeMask);
    XUngrabServer(x11Info().display());
    updateTitleBar(id);
    if (isVisible())
        onFocusIn();
}

void Container::setXFocus()
{
    XSetInputFocus(x11Info().display(), clientWinId(), RevertToParent, CurrentTime);
}

void Container::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == timer.timerId()) {
        setXFocus();
        timer.stop();
    }
}

