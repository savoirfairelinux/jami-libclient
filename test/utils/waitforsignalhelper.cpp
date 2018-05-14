/*
 *  Copyright (C) 2017-2018 Savoir-faire Linux Inc.
 *
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

#include "waitforsignalhelper.h"

#include <QTimer>

WaitForSignalHelper::WaitForSignalHelper(QObject& object, const char* signal)
: QObject(), timeout_(false)
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

////////////////////////////////////////////////////////////////
WaitForSignalHelper::WaitForSignalHelper(std::function<void()> f)
:f_(f)
{
}

WaitForSignalHelper&
WaitForSignalHelper::addSignal(const std::string& id, QObject& object, const char* signal)
{
    results_.insert({id , false});
    auto slot = std::bind(this, &WaitForSignalHelper::signalSlot, id, std::placeholders::_1);
    connect(&object, signal, SLOT(static_cast<void (WaitForSignalHelper::*)(void)>(slot)));
    return *this;
}

void
WaitForSignalHelper::signalSlot(const std::string& id)
{
    auto resultsSize = results_.size();
    unsigned signalsCaught = 0;
    // loop through results till we find the signal id and set to true
    // ... meanwhile testing the total caught signals and exiting the wait loop
    // if all the signals have come through
    for (auto it = results_.begin(); it != results_.end(); it++) {
        if ((*it).first.compare(id) == 0) {
            (*it).second = true;
        }
        signalsCaught = signalsCaught + static_cast<unsigned>((*it).second);
    }
    if (signalsCaught == resultsSize) {
        isRunning_.store(false);
    }
}

void
WaitForSignalHelper::timeout2()
{
    isRunning_.store(false);
}

std::map<std::string, bool>
WaitForSignalHelper::wait2(int timeoutMs)
{
    // connect timer to A::timeout() here
    printf("waiting %d\n", timeoutMs);
    std::future<void> resultsFuture = std::async([&]() {
        // block and fill timeout map
        auto start = std::chrono::high_resolution_clock::now();
        isRunning_.store(true);
        while(isRunning_.load() && std::chrono::high_resolution_clock::now() - start < std::chrono::milliseconds(timeoutMs)) {}
        return;
    });
    // wait till ready or timeout
    std::thread([this, timeoutMs] () {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait_for(lk, std::chrono::milliseconds(timeoutMs), [this, timeoutMs] {
            auto start = std::chrono::high_resolution_clock::now();
            while (!isRunning_.load() && std::chrono::high_resolution_clock::now() - start < std::chrono::milliseconds(timeoutMs)) {}
            return true;
        });
    }).join();
    // execute function
    f_();
    // wait for results...if they come or else time out
    resultsFuture.get();
    printf("done\n");
    return results_;
}
////////////////////////////////////////////////////////////////
