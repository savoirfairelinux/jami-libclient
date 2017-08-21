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

// Data
#include "data/call.h"
#include "data/account.h"

namespace lrc
{

class NewAccountModel;
class ConversationModel;

class NewCallModel : public QObject {
    Q_OBJECT

    friend class NewAccountModel;
    friend class ConversationModel;

public:
    const account::Info& owner;

    enum class Media {
        NONE,
        AUDIO,
        VIDEO
    };

    ~NewCallModel();

    void hangUp(const std::string& callId) const;
    void togglePause(const std::string& callId) const;
    void toggleMedia(const std::string& callId, const Media media) const;
    void toggleRecoringdAudio(const std::string& callId) const;
    void setQuality(const std::string& callId, const double quality) const;
    void transfer(const std::string& callId, const std::string& to) const;
    void addParticipant(const std::string& callId, const std::string& participant);
    void removeParticipant(const std::string& callId, const std::string& participant);

private:
    explicit NewCallModel(NewAccountModel& parent, const account::Info& info);
    NewCallModel(const NewCallModel& newCallModel);
    const call::Info& createCall(const std::string& contactUri);
    void sendMessage(const std::string& callId, const std::string& body) const;

    CallsInfoMap calls_;
    NewAccountModel& parent_;

};

} // namespace lrc
