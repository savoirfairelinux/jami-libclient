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
    : linked(linked)
    , callId_(callId)
{
    update(infos);
}

QList<ParticipantInfos>
CallParticipants::getParticipants() const
{
    return participants_.values();
}

void
CallParticipants::update(const VectorMapStringString& infos)
{
    std::lock_guard<std::mutex> lk(streamMtx_);
    validUris_.clear();
    filterCandidates(infos);
    validUris_.sort();

    idx_ = 0;
    auto keys = participants_.keys();
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
    auto it = participants_.begin() + index;
    participants_.erase(it);
    Q_EMIT linked.participantRemoved(callId_, idx_);
}

void
CallParticipants::addParticipant(const ParticipantInfos& participant)
{
    auto it = participants_.find(participant.uri);
    if (it == participants_.end()) {
        participants_.insert(participants_.begin() + idx_, participant.uri, participant);
        Q_EMIT linked.participantAdded(callId_, idx_);
    } else {
        if (participant == (*it))
            return;
        (*it) = participant;
        Q_EMIT linked.participantUpdated(callId_, idx_);
    }
}

void
CallParticipants::filterCandidates(const VectorMapStringString& infos)
{
    candidates_.clear();
    for (const auto& candidate : infos) {
        auto peerId = candidate["uri"];
        peerId.truncate(peerId.lastIndexOf("@"));
        if (peerId.isEmpty()) {
            for (const auto& accId : linked.owner.accountModel->getAccountList()) {
                try {
                    auto& accountInfo = linked.owner.accountModel->getAccountInfo(accId);
                    if (accountInfo.callModel->hasCall(callId_)) {
                        peerId = accountInfo.profileInfo.uri;
                    }
                } catch (...) {
                }
            }
        }
        if (candidate["w"].toInt() != 0 && candidate["h"].toInt() != 0) {
            validUris_.append(peerId);
            candidates_.insert(peerId, ParticipantInfos(candidate, callId_, peerId));
        }
    }
}

bool
CallParticipants::checkModerator(const QString& uri) const
{
    return std::find_if(participants_.cbegin(),
                        participants_.cend(),
                        [&](auto participant) { return participant.uri == uri; })
           != participants_.cend();
}

QJsonObject
CallParticipants::toQJsonObject(uint index) const
{
    if (index >= participants_.size())
        return {};

    QJsonObject ret;
    const auto& participant = participants_.begin() + index;

    ret["uri"] = participant->uri;
    ret["device"] = participant->device;
    ret["sinkId"] = participant->sinkId;
    ret["bestName"] = participant->bestName;
    ret["avatar"] = participant->avatar;
    ret["active"] = participant->active;
    ret["x"] = participant->x;
    ret["y"] = participant->y;
    ret["width"] = participant->width;
    ret["height"] = participant->height;
    ret["audioLocalMuted"] = participant->audioLocalMuted;
    ret["audioModeratorMuted"] = participant->audioModeratorMuted;
    ret["videoMuted"] = participant->videoMuted;
    ret["isModerator"] = participant->isModerator;
    ret["islocal"] = participant->islocal;
    ret["isContact"] = participant->isContact;
    ret["handRaised"] = participant->handRaised;
    ret["callId"] = callId_;

    return ret;
}
} // end namespace api
} // end namespace lrc
