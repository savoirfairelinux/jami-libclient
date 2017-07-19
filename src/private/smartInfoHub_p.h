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

/* widget_p.h (_p means private) */
#include <QObject>
#include "../smartinfohub.h"
#include "typedefs.h"

#pragma once

//variables contain in the map information
static QString LOCAL_FPS           = QStringLiteral("local FPS");
static QString LOCAL_AUDIO_CODEC   = QStringLiteral("local audio codec");
static QString LOCAL_VIDEO_CODEC   = QStringLiteral("local video codec");
static QString LOCAL_WIDTH         = QStringLiteral("local width");
static QString LOCAL_HEIGHT        = QStringLiteral("local height");
static QString REMOTE_FPS          = QStringLiteral("remote FPS");
static QString REMOTE_WIDTH        = QStringLiteral("remote width");
static QString REMOTE_HEIGHT       = QStringLiteral("remote height");
static QString REMOTE_VIDEO_CODEC  = QStringLiteral("remote video codec");
static QString REMOTE_AUDIO_CODEC  = QStringLiteral("remote audio codec");
static QString CALL_ID             = QStringLiteral("callID");

class SmartInfoHubPrivate;
class SmartInfoHubPrivate final : public QObject
{
    Q_OBJECT

public:
    constexpr static const char* DEFAULT_RETURN_VALUE_QSTRING = "void";

    uint32_t m_refreshTimeInformationMS = 500;
    QMap<QString, QString> m_information;

    void setMapInfo(const MapStringString& info);

public slots:
    void slotSmartInfo(const MapStringString& info);
};
