/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author: Olivier Grégoire <olivier.gregoire@savoirfairelinux.com>       *
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

/*
 * SmartInfo is an API who will retrieve a lot of information from the daemon.
 *    -Local and remote video codec
 *    -Local and remote frame rate
 *    -Remote audio codec
 *    -Remote resolution
 * The goal is to display all that to the clients.
 * That will help the user to debug his system and/or just show him what
 * happened in the system.
 */

#pragma once

#include <iostream>
#include <map>
#include <vector>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QMimeData>
#include <QtCore/QItemSelectionModel>
#include <QObject>
#include <QtCore/QUrl>
#include <QList>

#include "callmodel.h"
#include "typedefs.h"

class SmartInfoHubPrivate;

class SmartInfoHub final : public QObject
{
    Q_OBJECT
    public:
        // Signleton
        static SmartInfoHub& instance();

        void start();
        void stop();

        void setRefreshTime(int timeMS);

        /***********************************************************************
         *                                                                     *
         *                            Getter                                   *
         *                                                                     *
         ***********************************************************************/
         float          localFps();
         float          remoteFps();
         int            remoteWidth();
         int            remoteHeight();
         QString        localVideoCodec();
         QString        remoteVideoCodec();
         QString        remoteAudioCodec();
         QList<QString> keyChanged();

    signals:
        /* Signal emit every tine something change. He contain a list of update
         * You can connect it to a lambda function
         */
        void changed();

    private:

        SmartInfoHubPrivate* d_ptr;
        
};
