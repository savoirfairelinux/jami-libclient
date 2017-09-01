/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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

// Std
#include <memory>

// Qt
#include <qobject.h>

// Interface
#include "api/newcallmodeli.h"

// Data
#include "api/call.h"
#include "api/account.h"

namespace lrc
{

class NewAccountModel;

using CallInfoMap = std::map<std::string, std::shared_ptr<api::call::Info>>;

class NewCallModel : public api::NewCallModelI {
public:
    const api::account::Info& owner;

    NewCallModel(NewAccountModel& parent, const api::account::Info& info);
    NewCallModel(const NewCallModel& newCallModel);
    ~NewCallModel();

    const api::call::Info& createCall(const std::string& contactUri);

    void hangUp(const std::string& callId) const override;
    void togglePause(const std::string& callId) const override;
    void toggleMedia(const std::string& callId, const api::NewCallModelI::Media media) const override;
    void toggleRecoringdAudio(const std::string& callId) const override;
    void setQuality(const std::string& callId, const double quality) const override;
    void transfer(const std::string& callId, const std::string& to) const override;
    void addParticipant(const std::string& callId, const std::string& participant) override;
    void removeParticipant(const std::string& callId, const std::string& participant) override;

private:
    void sendMessage(const std::string& callId, const std::string& body) const;

    CallInfoMap calls_;
    NewAccountModel& parent_;
};

} // namespace lrc
