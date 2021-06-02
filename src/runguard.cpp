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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
// Based on: https://stackoverflow.com/a/28172162

#include "runguard.h"

#include "mainapplication.h"

#include <QCryptographicHash>
#include <QLocalSocket>

namespace {

QString
generateKeyHash(const QString& key, const QString& salt)
{
    QByteArray data;

    data.append(key.toUtf8());
    data.append(salt.toUtf8());
    data = QCryptographicHash::hash(data, QCryptographicHash::Sha1).toHex();

    return data;
}

} // namespace

RunGuard::RunGuard(const QString& key, MainApplication* mainApp)
    : key_(key)
    , memLockKey_(generateKeyHash(key, "_memLockKey"))
    , sharedmemKey_(generateKeyHash(key, "_sharedmemKey"))
    , sharedMem_(sharedmemKey_)
    , memLock_(memLockKey_, 1)
    , mainAppInstance_(mainApp)
{}

RunGuard::~RunGuard()
{
    release();
}

void
RunGuard::tryRestorePrimaryInstance()
{
    mainAppInstance_->restoreApp();
}

bool
RunGuard::isAnotherRunning()
{
    if (sharedMem_.isAttached())
        return false;

    memLock_.acquire();
    const bool isRunning = sharedMem_.attach();
    if (isRunning)
        sharedMem_.detach();
    memLock_.release();

    return isRunning;
}

bool
RunGuard::tryToRun()
{
    if (isAnotherRunning()) {
        /*
         * This is a secondary instance,
         * connect to the primary instance to trigger a restore
         * then fail.
         */
        if (!socket_)
            socket_ = new QLocalSocket();
        if (!socket_)
            return false;
        if (socket_->state() == QLocalSocket::UnconnectedState
            || socket_->state() == QLocalSocket::ClosingState) {
            socket_->connectToServer(key_);
        }
        if (socket_->state() == QLocalSocket::ConnectingState) {
            socket_->waitForConnected();
        }
        if (socket_->state() == QLocalSocket::ConnectedState) {
            return false;
        }
        // If not connected, this means that the server doesn't exists
        // and the app can be relaunched (can be the case after a client crash or Ctrl+C)
    }

    memLock_.acquire();
    const bool result = sharedMem_.create(sizeof(quint64));
    memLock_.release();
    if (!result) {
        release();
        return false;
    }

    /*
     * This is the primary instance,
     * listen for subsequent instances.
     */
    QLocalServer::removeServer(key_);
    server_ = new QLocalServer();
    server_->setSocketOptions(QLocalServer::UserAccessOption);
    server_->listen(key_);
    QObject::connect(server_,
                     &QLocalServer::newConnection,
                     this,
                     &RunGuard::tryRestorePrimaryInstance);

    return true;
}

void
RunGuard::release()
{
    memLock_.acquire();
    if (sharedMem_.isAttached())
        sharedMem_.detach();
    memLock_.release();
}
