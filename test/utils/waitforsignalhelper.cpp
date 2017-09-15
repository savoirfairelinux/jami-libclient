// Credits: https://stackoverflow.com/questions/31103983/qt-waiting-for-a-signal-with-timeout-management

#include "waitforsignalhelper.h"

#include <QTimer>

WaitForSignalHelper::WaitForSignalHelper(QObject& object, const char* signal)
: timeout_(false)
{
    connect(&object, signal, &eventLoop_, SLOT(quit()));
}

bool
WaitForSignalHelper::wait(unsigned int timeoutMs)
{
    QTimer timeoutHelper;
    if (timeoutMs != 0) {
        timeoutHelper.setInterval(timeoutMs);
        timeoutHelper.start();
        connect(&timeoutHelper, SIGNAL(timeout()), this, SLOT(timeout()));
    }
    timeout_ = false;
    eventLoop_.exec();
    return timeout_;
}

void
WaitForSignalHelper::timeout()
{
    timeout_ = true;
    eventLoop_.quit();
}
