/*!
 *    Copyright (C) 2021 Savoir-faire Linux Inc.
 *   Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// Qt
#include <QObject>
#include <QJsonObject>

// std
#include <string>
#include <ctime>
#include <chrono>
#include <mutex>

#include "typedefs.h"

namespace lrc {

namespace api {
class NewCallModel;

struct ParticipantInfos
{
    ParticipantInfos() {}

    ParticipantInfos(const MapStringString& infos, const QString& callId, const QString& peerId)
    {
        uri = infos["uri"];
        if (uri.lastIndexOf("@") > 0)
            uri.truncate(uri.lastIndexOf("@"));
        if (uri.isEmpty())
            uri = peerId;
        device = infos["device"];
        active = infos["active"] == "true";
        x = infos["x"].toInt();
        y = infos["y"].toInt();
        width = infos["w"].toInt();
        height = infos["h"].toInt();
        videoMuted = infos["videoMuted"] == "true";
        audioLocalMuted = infos["audioLocalMuted"] == "true";
        audioModeratorMuted = infos["audioModeratorMuted"] == "true";
        isModerator = infos["isModerator"] == "true";
        handRaised = infos["handRaised"] == "true";

        if (infos["sinkId"].isEmpty())
            sinkId = callId + uri + device;
        else
            sinkId = infos["sinkId"];

        bestName = "";
    }

    QString uri;
    QString device;
    QString sinkId;
    QString bestName;
    QString avatar;
    bool active {false};
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
    bool audioLocalMuted {false};
    bool audioModeratorMuted {false};
    bool videoMuted {false};
    bool isModerator {false};
    bool islocal {false};
    bool isContact {false};
    bool handRaised {false};

    bool operator==(const ParticipantInfos& other) const
    {
        return uri == other.uri && sinkId == other.sinkId && active == other.active
               && audioLocalMuted == other.audioLocalMuted
               && audioModeratorMuted == other.audioModeratorMuted && avatar == other.avatar
               && bestName == other.bestName && isContact == other.isContact
               && islocal == other.islocal && videoMuted == other.videoMuted
               && handRaised == other.handRaised;
    }
};

class LIB_EXPORT CallParticipants : public QObject
{
    Q_OBJECT

public:
    CallParticipants(const VectorMapStringString& infos,
                     const QString& callId,
                     const NewCallModel& linked);
    ~CallParticipants() {}

    QList<ParticipantInfos> getParticipants() const;
    void update(const VectorMapStringString& infos);

    QJsonObject toQJsonObject(uint index) const;

private:
    void filterCandidates(const VectorMapStringString& infos);
    void removeParticipant(int index);
    void addParticipant(const ParticipantInfos& participant);
    QMap<QString, ParticipantInfos> candidates_;
    QMap<QString, ParticipantInfos> participants_;
    QList<QString> validUris_;
    int idx_;
    const NewCallModel& linked;
    std::mutex streamMtx_ {};
    const QString callId_;
};
} // end namespace api
} // end namespace lrc
Q_DECLARE_METATYPE(lrc::api::CallParticipants*)
