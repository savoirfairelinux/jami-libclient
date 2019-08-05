/****************************************************************************
 *    Copyright (C) 2016-2019 Savoir-faire Linux Inc.                               *
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
static const std::string LOCAL_FPS           ("local FPS");
static const std::string LOCAL_AUDIO_CODEC   ("local audio codec");
static const std::string LOCAL_VIDEO_CODEC   ("local video codec");
static const std::string LOCAL_WIDTH         ("local width");
static const std::string LOCAL_HEIGHT        ("local height");
static const std::string REMOTE_FPS          ("remote FPS");
static const std::string REMOTE_WIDTH        ("remote width");
static const std::string REMOTE_HEIGHT       ("remote height");
static const std::string REMOTE_VIDEO_CODEC  ("remote video codec");
static const std::string REMOTE_AUDIO_CODEC  ("remote audio codec");
static const std::string CALL_ID             ("callID");

class SmartInfoHubPrivate;
class SmartInfoHubPrivate final : public QObject
{
    Q_OBJECT

public:
    constexpr static const char* DEFAULT_RETURN_VALUE_QSTRING = "void";

    uint32_t m_refreshTimeInformationMS = 500;
    std::map<std::string, std::string> m_information;

    //void setMapInfo(const QMap<QString, QString>& info);

public Q_SLOTS:
    void slotSmartInfo(const std::map<std::string, std::string>& info);
};
