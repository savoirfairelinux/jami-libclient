/*!
 * Copyright (C) 2015-2020 by Savoir-faire Linux
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 * Author: Isa Nanic <isa.nanic@savoirfairelinux.com>
 * Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>
 * Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
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

#pragma once

#include <QObject>

namespace Utils {

template<typename Func1, typename Func2>
void
oneShotConnect(const typename QtPrivate::FunctionPointer<Func1>::Object* sender,
               Func1 signal,
               Func2 slot)
{
    QMetaObject::Connection* const connection = new QMetaObject::Connection;
    *connection = QObject::connect(sender, signal, slot);
    QMetaObject::Connection* const disconnectConnection = new QMetaObject::Connection;
    *disconnectConnection = QObject::connect(sender, signal, [connection, disconnectConnection] {
        if (connection) {
            QObject::disconnect(*connection);
            delete connection;
        }
        if (disconnectConnection) {
            QObject::disconnect(*disconnectConnection);
            delete disconnectConnection;
        }
    });
}

template<typename Func1, typename Func2>
void
oneShotConnect(const typename QtPrivate::FunctionPointer<Func1>::Object* sender,
               Func1 signal,
               const typename QtPrivate::FunctionPointer<Func2>::Object* receiver,
               Func2 slot)
{
    QMetaObject::Connection* const connection = new QMetaObject::Connection;
    *connection = QObject::connect(sender, signal, receiver, slot);
    QMetaObject::Connection* const disconnectConnection = new QMetaObject::Connection;
    *disconnectConnection = QObject::connect(sender, signal, [connection, disconnectConnection] {
        if (connection) {
            QObject::disconnect(*connection);
            delete connection;
        }
        if (disconnectConnection) {
            QObject::disconnect(*disconnectConnection);
            delete disconnectConnection;
        }
    });
}

class OneShotConnection final : public QObject
{
    Q_OBJECT
public:
    explicit OneShotConnection(const QObject* sender,
                               const char* signal,
                               QMetaObject::Connection* connection,
                               QObject* parent = nullptr)
        : QObject(parent)
    {
        connection_ = connection;
        disconnectConnection_ = new QMetaObject::Connection;
        *disconnectConnection_ = QObject::connect(sender, signal, this, SLOT(onTriggered()));
    }
    ~OneShotConnection() = default;

public slots:
    void onTriggered()
    {
        if (connection_) {
            QObject::disconnect(*connection_);
            delete connection_;
        }
        if (disconnectConnection_) {
            QObject::disconnect(*disconnectConnection_);
            delete disconnectConnection_;
        }
        deleteLater();
    }

private:
    QMetaObject::Connection* connection_;
    QMetaObject::Connection* disconnectConnection_;
};

inline void
oneShotConnect(const QObject* sender, const char* signal, const QObject* receiver, const char* slot)
{
    QMetaObject::Connection* const connection = new QMetaObject::Connection;
    *connection = QObject::connect(sender, signal, receiver, slot);
    new OneShotConnection(sender, signal, connection);
}

} // namespace Utils