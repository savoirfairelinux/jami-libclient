/*
 *  Copyright (C) 2017-2018 Savoir-faire Linux Inc.
 *
 *  Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>
 *  Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
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
#include "api/interaction.h"
#include "../mocks/configurationmanager_mock.h"

#include <QTimer>
#include <QSignalMapper>
#include <QMetaType>
#include <QString>
#include <QMap>

WaitForSignalHelper::WaitForSignalHelper(std::function<void()> f)
: f_(f)
{
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<uint32_t>("uint32_t");
    qRegisterMetaType<uint64_t>("uint64_t");
    qRegisterMetaType<lrc::api::interaction::Info>("interaction::Info");
    qRegisterMetaType<MapStringString>("MapStringString");
}

WaitForSignalHelper&
WaitForSignalHelper::addSignal(const std::string& id, QObject& object, const char* signal)
{
    results_.insert({id , 0});
    QSignalMapper* signalMapper = new QSignalMapper(this);
    auto connection = connect(&object, signal, signalMapper, SLOT(map()), Qt::QueuedConnection);
    connections_.emplace_back(connection);
    signalMapper->setMapping(&object, QString::fromStdString(id));
    connection = connect(signalMapper,
            SIGNAL(mapped(const QString&)),
            this,
            SLOT(signalSlot(const QString&)), Qt::QueuedConnection);
    connections_.emplace_back(connection);
    return *this;
}

void
WaitForSignalHelper::signalSlot(const QString & id)
{
    std::string signalId = id.toStdString();
    printf("Signal caught: %s\n", signalId.c_str());
    auto resultsSize = results_.size();
    unsigned signalsCaught = 0;
    // loop through results map till we find the id and increment the value,
    // meanwhile testing the total caught signals and exiting the wait loop
    // if all the signals have come through at least once
    for (auto it = results_.begin(); it != results_.end(); it++) {
        if ((*it).first.compare(signalId) == 0) {
            (*it).second = 1;
        }
        signalsCaught = signalsCaught + static_cast<unsigned>((*it).second > 0);
        if (signalsCaught == resultsSize) {
            printf("All signals caught\n");
            eventLoop_.quit();
            break;
        }
    }
}

void
WaitForSignalHelper::timeout()
{
    printf("Timed out! signal(s) missed\n");
    eventLoop_.quit();
}

std::map<std::string, int>
WaitForSignalHelper::wait(int timeoutMs)
{
    if (timeoutMs <= 0) {
        throw std::invalid_argument("Invalid time out value");
    }
    // wait till ready or timeout
    std::thread readyThread([this, timeoutMs] () {
        std::unique_lock<std::mutex> lk(mutex_);
        cv_.wait_for(lk, std::chrono::milliseconds(timeoutMs), [this, timeoutMs] {
            auto start = std::chrono::high_resolution_clock::now();
            while (!eventLoop_.isRunning() && std::chrono::high_resolution_clock::now() - start < std::chrono::milliseconds(timeoutMs)) {}
            return true;
        });
        // execute function expected to produce awaited signal(s)
        f_();
    });
    // connect timer to A::timeout() here… or use chrono and busy loop, cv, etc.
    QTimer timeoutHelper;
    timeoutHelper.setInterval(timeoutMs);
    timeoutHelper.start();
    connect(&timeoutHelper, SIGNAL(timeout()), this, SLOT(timeout()), Qt::QueuedConnection);
    cv_.notify_all();
    // wait for results… if they come, else time out
    eventLoop_.exec();
    for (auto& connection: connections_) {
        QObject::disconnect(connection);
    }
    readyThread.join();
    return results_;
}
