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

#pragma once

// std
#include <mutex>
#include <string>
#include <future>
#include <map>
#include <functional>

// Qt
#include <QEventLoop>

/**
 * Class used to wait a Qt signal
 * @param object to listen from
 * @param signal to detect
 */
class WaitForSignalHelper: public QObject
{
    Q_OBJECT
public:
    WaitForSignalHelper(QObject& object, const char* signal);
    /**
     * Connect the signal to quit() slot
     * @param  timeoutMs the time to wait
     * @return if the signal was emitted
     */
    bool wait(unsigned int timeoutMs);

    ////////////////////////////////////////////////
    WaitForSignalHelper(std::function<void()> f);
    WaitForSignalHelper& addSignal(const std::string& id, QObject& object, const char* signal);
    std::map<std::string, bool> wait2(int timeoutMs);
    ////////////////////////////////////////////////

public Q_SLOTS:
    /**
     * Is activated if wait is finished
     */
    void timeout();

    ////////////////////////////////////////////////
    void signalSlot(const QString & id);
    void timeout2();
    ////////////////////////////////////////////////

private:
    bool timeout_;
    QEventLoop eventLoop_;

    ////////////////////////////////////////////////
    std::function<void()> f_;
    std::map<std::string, bool> results_;

    // represents execLoop_.isRunning()
    std::atomic<bool> isRunning_;

    std::mutex mutex_;
    std::condition_variable cv_;
    bool ready_;
    ////////////////////////////////////////////////
};
