/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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
#include <string>
#include <map>

// Qt
#include <qobject.h>

// Data
#include "api/call.h"
#include "api/account.h"

namespace Video {
class Renderer;
}

namespace lrc
{

class CallbacksHandler;
class NewCallModelPimpl;

namespace api
{

namespace account { struct Info; }
namespace call { struct Info; }
class NewAccountModel;

class NewCallModel : public QObject {
    Q_OBJECT

    friend class NewAccountModel;
    friend class ConversationModel;

public:
    using CallInfoMap = std::map<std::string, std::shared_ptr<call::Info>>;

    const account::Info& owner;

    enum class Media {
        NONE,
        AUDIO,
        VIDEO
    };

    NewCallModel(const account::Info& owner, const CallbacksHandler& callbacksHandler);
    ~NewCallModel();

    const std::string createCall(const std::string& url);
    const call::Info& getCall(const std::string& uid) const;
    const call::Info& getCallFromURI(const std::string& uri) const;
    bool hasCall(const std::string& callId);
    void sendSipMessage(const std::string& callId, const std::string& body) const;

    void accept(const std::string& callId) const;
    void hangUp(const std::string& callId) const;
    void togglePause(const std::string& callId) const;
    void toggleMedia(const std::string& callId, const NewCallModel::Media media, bool flag) const;
    void toggleRecoringdAudio(const std::string& callId) const;
    void setQuality(const std::string& callId, const double quality) const;
    void transfer(const std::string& callId, const std::string& to) const;
    void addParticipant(const std::string& callId, const std::string& participant);
    void removeParticipant(const std::string& callId, const std::string& participant);

    Video::Renderer* getRenderer(const std::string& callId) const;

    std::string getFormattedCallDuration(const std::string& callId) const;

Q_SIGNALS:
    void callStatusChanged(const std::string& callId) const;
    void callStarted(const std::string& callId) const;
    void callEnded(const std::string& callId) const;
    void newIncomingCall(const std::string& callId, const std::string& fromId) const;
    void remotePreviewStarted(const std::string& callId, Video::Renderer* renderer) const;

private:
    std::unique_ptr<NewCallModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
