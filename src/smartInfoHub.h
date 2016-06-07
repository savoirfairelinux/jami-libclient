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

//variables contain on the map
#define LOCAL_FPS          "local FPS"
#define LOCAL_VIDEO_CODEC  "local video codec"
#define REMOTE_FPS         "remote FPS"
#define REMOTE_WIDTH       "remote width"
#define REMOTE_HEIGHT      "remote height"
#define REMOTE_VIDEO_CODEC "remote video codec"
#define REMOTE_AUDIO_CODEC "remote audio codec"

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
         float          localFps() const;
         float          remoteFps() const;
         int            remoteWidth() const;
         int            remoteHeight() const;
         QString        localVideoCodec() const;
         QString        remoteVideoCodec() const;
         QString        remoteAudioCodec() const;
         QList<QString> keyChanged();

    signals:
        /* Signal emit every tine something change. He contain a list of update
         * You can connect it to a lambda function
         */
        void changed();

    private:

        SmartInfoHubPrivate* d_ptr;
};
