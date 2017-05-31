/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
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

#include "ringdevice.h"
#include "ringdevicemodel.h"

typedef void (RingDeviceModelPrivate::*RingDeviceModelPrivateFct)();

class RingDeviceModelPrivate : public QObject
{
public:
   Q_OBJECT
   Q_DECLARE_PUBLIC(RingDeviceModel)
   RingDeviceModelPrivate(RingDeviceModel* q,Account* a);

   //Attributes
   Account*                   m_pAccount     ;
   QVector<RingDevice*>       m_lRingDevices ;
   RingDeviceModel*           q_ptr          ;

   void reload();
   void reload(MapStringString accountDevices);
   void clearLines();

public Q_SLOTS:   
   void slotDeviceRevocationEnded(const QString &accountID, const QString &deviceId, int status);

};
