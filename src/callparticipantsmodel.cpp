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

#include "api/callparticipantsmodel.h"

#include "api/account.h"
#include "api/contactmodel.h"
#include "api/contact.h"
#include "api/newcallmodel.h"
#include "api/newaccountmodel.h"

namespace lrc {

namespace api {

CallParticipants::CallParticipants(const VectorMapStringString& infos,
                                   const QString& callId,
                                   const NewCallModel& linked)
    : linked_(linked)
    , callId_(callId)
{
    update(infos);
}

QList<ParticipantInfos>
CallParticipants::getParticipants() const
{
    std::lock_guard<std::mutex> lk(participantsMtx_);
    return participants_.values();
}

void
CallParticipants::update(const VectorMapStringString& infos)
{
    std::lock_guard<std::mutex> lk(updateMtx_);
    validUris_.clear();
    filterCandidates(infos);
    validUris_.sort();

    idx_ = 0;
    QList<QString> keys {};
    {
        std::lock_guard<std::mutex> lk(participantsMtx_);
        keys = participants_.keys();
    }
    for (const auto& key : keys) {
        auto keyIdx = validUris_.indexOf(key);
        if (keyIdx < 0 || keyIdx >= validUris_.size())
            removeParticipant(idx_);
        else
            idx_++;
    }

    idx_ = 0;
    for (const auto& partUri : validUris_) {
        addParticipant(candidates_[partUri]);
        idx_++;
    }

    verifyLayout();
}

void
CallParticipants::verifyLayout()
{
    std::lock_guard<std::mutex> lk(participantsMtx_);
    auto it = std::find_if(participants_.begin(),
                           participants_.end(),
                           [](const lrc::api::ParticipantInfos& participant) -> bool {
                               return participant.active;
                           });

    auto newLayout = call::Layout::GRID;
    if (it != participants_.end())
        if (participants_.size() == 1)
            newLayout = call::Layout::ONE;
        else
            newLayout = call::Layout::ONE_WITH_SMALL;
    else
        newLayout = call::Layout::GRID;

    if (newLayout != hostLayout_)
        hostLayout_ = newLayout;
}

void
CallParticipants::removeParticipant(int index)
{
    {
        std::lock_guard<std::mutex> lk(participantsMtx_);
        auto it = participants_.begin() + index;
        participants_.erase(it);
    }
    Q_EMIT linked_.participantRemoved(callId_, idx_);
}

void
CallParticipants::addParticipant(const ParticipantInfos& participant)
{
    bool added {false};
    {
        std::lock_guard<std::mutex> lk(participantsMtx_);
        auto it = participants_.find(participant.uri);
        if (it == participants_.end()) {
            participants_.insert(participants_.begin() + idx_, participant.uri, participant);
            added = true;
        } else {
            if (participant == (*it))
                return;
            (*it) = participant;
        }
    }
    if (added)
        Q_EMIT linked_.participantAdded(callId_, idx_);
    else
        Q_EMIT linked_.participantUpdated(callId_, idx_);
}

void
CallParticipants::filterCandidates(const VectorMapStringString& infos)
{
    std::lock_guard<std::mutex> lk(participantsMtx_);
    candidates_.clear();
    for (const auto& candidate : infos) {
        auto peerId = candidate[ParticipantsInfosStrings::URI];
        peerId.truncate(peerId.lastIndexOf("@"));
        if (peerId.isEmpty()) {
            for (const auto& accId : linked_.owner.accountModel->getAccountList()) {
                try {
                    auto& accountInfo = linked_.owner.accountModel->getAccountInfo(accId);
                    if (accountInfo.callModel->hasCall(callId_)) {
                        peerId = accountInfo.profileInfo.uri;
                    }
                } catch (...) {
                }
            }
        }
        if (candidate[ParticipantsInfosStrings::W].toInt() != 0
            && candidate[ParticipantsInfosStrings::H].toInt() != 0) {
            validUris_.append(peerId);
            candidates_.insert(peerId, ParticipantInfos(candidate, callId_, peerId));
        }
    }
}

bool
CallParticipants::checkModerator(const QString& uri) const
{
    std::lock_guard<std::mutex> lk(participantsMtx_);
    return std::find_if(participants_.cbegin(),
                        participants_.cend(),
                        [&](auto participant) {
                            return participant.uri == uri && participant.isModerator;
                        })
           != participants_.cend();
}

QJsonObject
CallParticipants::toQJsonObject(uint index) const
{
    std::lock_guard<std::mutex> lk(participantsMtx_);
    if (index >= participants_.size())
        return {};

    QJsonObject ret;
    const auto& participant = participants_.begin() + index;

    ret[ParticipantsInfosStrings::URI] = participant->uri;
    ret[ParticipantsInfosStrings::DEVICE] = participant->device;
    ret[ParticipantsInfosStrings::SINKID] = participant->sinkId;
    ret[ParticipantsInfosStrings::BESTNAME] = participant->bestName;
    ret[ParticipantsInfosStrings::AVATAR] = participant->avatar;
    ret[ParticipantsInfosStrings::ACTIVE] = participant->active;
    ret[ParticipantsInfosStrings::X] = participant->x;
    ret[ParticipantsInfosStrings::Y] = participant->y;
    ret[ParticipantsInfosStrings::WIDTH] = participant->width;
    ret[ParticipantsInfosStrings::HEIGHT] = participant->height;
    ret[ParticipantsInfosStrings::AUDIOLOCALMUTED] = participant->audioLocalMuted;
    ret[ParticipantsInfosStrings::AUDIOMODERATORMUTED] = participant->audioModeratorMuted;
    ret[ParticipantsInfosStrings::VIDEOMUTED] = participant->videoMuted;
    ret[ParticipantsInfosStrings::ISMODERATOR] = participant->isModerator;
    ret[ParticipantsInfosStrings::ISLOCAL] = participant->islocal;
    ret[ParticipantsInfosStrings::ISCONTACT] = participant->isContact;
    ret[ParticipantsInfosStrings::HANDRAISED] = participant->handRaised;
    ret[ParticipantsInfosStrings::CALLID] = callId_;

    return ret;
}
} // end namespace api
} // end namespace lrc
