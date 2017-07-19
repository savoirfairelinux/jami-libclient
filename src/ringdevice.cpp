/****************************************************************************
 *   Copyright (C) 2016-2017 Savoir-faire Linux                               *
 *   Author : Alexandre Viau <alexandre.viau@savoirfairelinux.com>          *
 *                                                                          *
 *   This library is free software you can redistribute it and/or           *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation either            *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/

// Qt
#include <QtCore/QObject>

#include "ringdevice.h"


class RingDevicePrivate final : public QObject
{
public:
    Q_OBJECT
    Q_DECLARE_PUBLIC(RingDevice)

    RingDevice*   q_ptr   ;
    QString       m_ID   ;
    QString       m_Name ;

    // Constructor
    explicit RingDevicePrivate(RingDevice* device);
};

RingDevicePrivate::RingDevicePrivate(RingDevice* device) : QObject(device), q_ptr(device)
{
}

 ///Constructors
RingDevice::RingDevice(const QString& id, const QString& name)
{
    d_ptr = new RingDevicePrivate(this);
    d_ptr->m_ID = id;
    d_ptr->m_Name = name;
}

const QString RingDevice::id() const
{
    return d_ptr->m_ID;
}

const QString RingDevice::name() const
{
    return d_ptr->m_Name;
}

#define CAST(item) static_cast<int>(item)
QVariant RingDevice::columnData(int column) const
{
    switch(column) {
        case CAST(RingDevice::Column::Id):
            return id();
        case CAST(RingDevice::Column::Name):
            return name();
        default:
            return QVariant();
   }
}
#undef CAST


#include <ringdevice.moc>
