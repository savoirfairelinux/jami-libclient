// Credits: https://stackoverflow.com/questions/31103983/qt-waiting-for-a-signal-with-timeout-management

#pragma once

// Qt
#include <QEventLoop>

class WaitForSignalHelper: public QObject
{
    Q_OBJECT
public:
    WaitForSignalHelper(QObject& object, const char* signal);
    bool wait(unsigned int timeoutMs);

public Q_SLOTS:
    void timeout();

private:
    bool timeout_;
    QEventLoop eventLoop_;
};
