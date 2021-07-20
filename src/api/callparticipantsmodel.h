/*!
 *   Copyright (C) 2022 Savoir-faire Linux Inc.
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

// std
#include <string>
#include <ctime>
#include <chrono>
#include <mutex>

// Qt
#include <QObject>
#include <QJsonObject>

#include "typedefs.h"
#include "call.h"

namespace lrc {

namespace api {
class NewCallModel;

namespace ParticipantsInfosStrings {
const QString URI = "uri";
const QString DEVICE = "device";
const QString ACTIVE = "active";
const QString AVATAR = "avatar";
const QString X = "x";
const QString Y = "y";
const QString W = "w";
const QString H = "h";
const QString WIDTH = "widht";
const QString HEIGHT = "height";
const QString VIDEOMUTED = "videoMuted";
const QString AUDIOLOCALMUTED = "audioLocalMuted";
const QString AUDIOMODERATORMUTED = "audioModeratorMuted";
const QString ISMODERATOR = "isModerator";
const QString HANDRAISED = "handRaised";
const QString SINKID = "sinkId";
const QString BESTNAME = "bestName";
const QString ISLOCAL = "isLocal";
const QString ISCONTACT = "isContact";
const QString CALLID = "callId";
} // namespace ParticipantsInfosStrings

struct ParticipantInfos
{
    ParticipantInfos() {}

    ParticipantInfos(const MapStringString& infos, const QString& callId, const QString& peerId)
    {
        uri = infos[ParticipantsInfosStrings::URI];
        if (uri.lastIndexOf("@") > 0)
            uri.truncate(uri.lastIndexOf("@"));
        if (uri.isEmpty())
            uri = peerId;
        device = infos[ParticipantsInfosStrings::DEVICE];
        active = infos[ParticipantsInfosStrings::ACTIVE] == "true";
        x = infos[ParticipantsInfosStrings::X].toInt();
        y = infos[ParticipantsInfosStrings::Y].toInt();
        width = infos[ParticipantsInfosStrings::W].toInt();
        height = infos[ParticipantsInfosStrings::H].toInt();
        videoMuted = infos[ParticipantsInfosStrings::VIDEOMUTED] == "true";
        audioLocalMuted = infos[ParticipantsInfosStrings::AUDIOLOCALMUTED] == "true";
        audioModeratorMuted = infos[ParticipantsInfosStrings::AUDIOMODERATORMUTED] == "true";
        isModerator = infos[ParticipantsInfosStrings::ISMODERATOR] == "true";
        handRaised = infos[ParticipantsInfosStrings::HANDRAISED] == "true";

        if (infos[ParticipantsInfosStrings::SINKID].isEmpty())
            sinkId = callId + uri + device;
        else
            sinkId = infos[ParticipantsInfosStrings::SINKID];

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

    /**
     * @return The list of participants that can have a widget in the client
     */
    QList<ParticipantInfos> getParticipants() const;

    /**
     * Update the participants
     */
    void update(const VectorMapStringString& infos);

    /**
     * Update conference layout value
     */
    void verifyLayout();

    /**
     * @param uri participant
     * @return True if participant is a moderator
     */
    bool checkModerator(const QString& uri) const;

    /**
     * @return the conference layout
     */
    call::Layout getLayout() const { return hostLayout_; }

    /**
     * @param index participant index
     * @return informations of the participant in index
     */
    QJsonObject toQJsonObject(uint index) const;

private:
    /**
     * Filter the participants that might appear for the end user
     */
    void filterCandidates(const VectorMapStringString& infos);

    void removeParticipant(int index);

    void addParticipant(const ParticipantInfos& participant);

    // Participants in the conference
    QMap<QString, ParticipantInfos> candidates_;
    // Participants ordered
    QMap<QString, ParticipantInfos> participants_;
    QList<QString> validUris_;
    int idx_ = 0;

    const NewCallModel& linked_;

    // Protects changes into the paticipants_ variable
    mutable std::mutex participantsMtx_ {};
    // Protects calls to the update function
    std::mutex updateMtx_ {};

    const QString callId_;
    call::Layout hostLayout_ = call::Layout::GRID;
};
} // end namespace api
} // end namespace lrc
Q_DECLARE_METATYPE(lrc::api::CallParticipants*)
