/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Mingrui Zhang   <mingrui.zhang@savoirfairelinux.com>
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

/*
 * The main purpose of this class is to operate on qml objects,
 * or provide api calls to qml objects that cannot be done directly in qml.
 */
class QmlAdapterBase : public QObject
{
    Q_OBJECT
public:
    explicit QmlAdapterBase(QObject *parent = nullptr)
        : QObject(parent)
        , qmlObj_(nullptr) {};

    virtual ~QmlAdapterBase() = default;

    /*
     * This function should be called in the Component.onCompleted slot
     * in the qml component that this adapter should attach to.
     */
    Q_INVOKABLE void setQmlObject(QObject *obj)
    {
        qmlObj_ = obj;
        safeInit();
    };

protected:
    /*
     * Once the qml object is set, Qml method invokation can be done
     */
    virtual void safeInit() = 0;

    /*
     * Object pointer.
     */
    QObject *qmlObj_;
};
