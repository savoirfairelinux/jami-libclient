/****************************************************************************
 *   Copyright (C) 2016-2017 Savoir-faire Linux                               *
 *   Author : Alexandre Viau <alexandre.viau@savoirfairelinux.com>          *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#pragma once

#include <typedefs.h>
#include <itembase.h>

class RingDevicePrivate;


class LIB_EXPORT RingDevice : public ItemBase
{
    Q_OBJECT

    friend class RingDeviceModel;
    friend class RingDeviceModelPrivate;

public:

    enum class Column {
       Id = 0,
       Name
   };

    Q_PROPERTY(QString  id     READ id  )
    Q_PROPERTY(QString  name   READ name)

    //Getters
    const QString        id         (        )   const;
    const QString        name       (        )   const;
    Q_INVOKABLE QVariant columnData (int column) const;


private:
    RingDevice(const QString& id, const QString& name);
    RingDevicePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RingDevice)
};

Q_DECLARE_METATYPE(RingDevice*)
