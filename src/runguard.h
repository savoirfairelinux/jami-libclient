/*
 * Copyright (C) 2019-2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
// Based on: https://stackoverflow.com/a/28172162

#pragma once

#include <QObject>
#include <QSharedMemory>
#include <QSystemSemaphore>
#include <QtNetwork/QLocalServer>
#include <QtNetwork/QLocalSocket>

class RunGuard : public QObject
{
    Q_OBJECT;

public:
    RunGuard(const QString &key);
    ~RunGuard();

    bool isAnotherRunning();
    bool tryToRun();
    void release();

private slots:
    void tryRestorePrimaryInstance();

private:
    const QString key_;
    const QString memLockKey_;
    const QString sharedmemKey_;

    QSharedMemory sharedMem_;
    QSystemSemaphore memLock_;

    QLocalSocket *socket_;
    QLocalServer *server_;

    Q_DISABLE_COPY(RunGuard)
};