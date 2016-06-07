/*
 *  Copyright (C) 2015-2016 Savoir-faire Linux Inc.
 *  Author: Olivier Grégoire <olivier.gregoire@savoirfairelinux.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA.
 */

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

class SmartInfoHub final : public QObject
{
    Q_OBJECT
    public:
        /* This method control the update of the variable of this class.
         * The input refreshTimeMS set the update time of the information.
         * If refreshTimeMS=0, al the tool smartInfo stop.
         */

        void start();
        void stop();

        // Signleton
        static SmartInfoHub& instance();



        void setRefreshTime(int timeMS);

        /***********************************************************************
         *                                                                     *
         *                            Getter                                   *
         *                                                                     *
         ***********************************************************************/
         float   localFps();
         float   remoteFps();
         int     remoteWidth();
         int     remoteHeight();
         QString localVideoCodec();
         QString remoteVideoCodec();
         QString remoteAudioCodec();

    signals:
        /* Signal emit every tine something change. He contain a list of update
         * You can connect it to a lambda function
         */
        void newInfo(QList<QString>);

    public slots:
        void sSmartInfo(MapStringString);

    private:
        void setMapInfo(QMap<QString, QString> info);
        void smartInfo(int refreshTimeMS);
        int refreshTimeInformationMS_ =500;
        SmartInfoHub();
        QMap<QString, QString> information_;
        float remoteFps_,localFps_;
        int remoteWidth_, remoteHeight_;
        QString localVideoCodec_, remoteVideoCodec_, remoteAudioCodec_;
        const QString DEFAULT_RETURN_VALUE_QSTRING  = "void";
};
