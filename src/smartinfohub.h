/****************************************************************************
 *   Copyright (C) 2016-2017 Savoir-faire Linux                               *
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

#include <QObject>

class SmartInfoHubPrivate;

class SmartInfoHub final : public QObject
{
    Q_OBJECT
    public:
        // Singleton
        static SmartInfoHub& instance();

        void start();
        void stop();

        void setRefreshTime(uint32_t timeMS);

        //Getter
        float          localFps() const;
        float          remoteFps() const;
        int            remoteWidth() const;
        int            remoteHeight() const;
        int            localWidth() const;
        int            localHeight() const;
        QString        callID() const;
        QString        localVideoCodec() const;
        QString        localAudioCodec() const;
        QString        remoteVideoCodec() const;
        QString        remoteAudioCodec() const;
        bool           isConference() const;

    Q_SIGNALS:
        ///Emitted when informations have changed
        void changed();

    private:
        //use to initialise the connection between the Qsignal and the lambda function
        SmartInfoHub();
        virtual ~SmartInfoHub();

        SmartInfoHubPrivate* d_ptr;
};
