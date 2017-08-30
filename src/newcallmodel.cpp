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

#include "api/newcallmodel.h"

#include "callbackshandler.h"
#include "api/conversationmodel.h"
#include "api/newaccountmodel.h"
#include "dbus/callmanager.h"

// Ring daemon
#include <media_const.h>

// Qt
#include <QObject>
#include <QString>

namespace lrc
{

namespace api
{

class NewCallModelPimpl
{
public:
    NewCallModelPimpl(NewAccountModel& parentr);
    ~NewCallModelPimpl();

    void sendMessage(const std::string& callId, const std::string& body) const;

    CallInfoMap calls;
    NewAccountModel& parent;

};

NewCallModel::NewCallModel(NewAccountModel& parent, const account::Info& info)
: owner(info)
, pimpl_(std::make_unique<NewCallModelPimpl>(parent))
{

}

NewCallModel::~NewCallModel()
{
}

const call::Info&
NewCallModel::getCall(const std::string& uid) const
{
    if (pimpl_->calls.find(uid) == pimpl_->calls.end()) {
        throw std::out_of_range("No call at UID " + uid);
    }
    return *pimpl_->calls[uid];
}

std::string
NewCallModel::humanReadableStatus(const call::Status& status)
{
    switch(status)
    {
    case call::Status::PAUSED:
        return tr("Hold").toStdString();
    case call::Status::IN_PROGRESS:
        return tr("Talking").toStdString();
    case call::Status::INVALID:
        return tr("ERROR").toStdString();
    case call::Status::OUTGOING_REQUESTED:
        return tr("Outgoing requested").toStdString();
    case call::Status::INCOMING_RINGING:
        return tr("Incoming").toStdString();
    case call::Status::OUTGOING_RINGING:
        return tr("Calling").toStdString();
    case call::Status::CONNECTING:
        return tr("Connecting").toStdString();
    case call::Status::SEARCHING:
        return tr("Searching").toStdString();
    case call::Status::PEER_PAUSED:
        return tr("Hold").toStdString();
    case call::Status::INACTIVE:
        return tr("Inactive").toStdString();
    case call::Status::ENDED:
        return tr("Finished").toStdString();
    case call::Status::TERMINATING:
        return tr("Finished").toStdString();
    case call::Status::CONNECTED:
        return tr("Communication established").toStdString();
    case call::Status::AUTO_ANSWERING:
        return tr("Auto answering").toStdString();
        break;
    }
}

const call::Info&
NewCallModel::createCall(const std::string& url)
{
    // do not use auto here (QDBusPendingReply<QString>)
    QString callId = CallManager::instance().placeCall(owner.id.c_str(), url.c_str());

    if (callId.isEmpty())
        qDebug() << "no call placed between (account :" << owner.id.c_str() << ", contact :" << url.c_str() << ")";

    auto callInfo = std::make_shared<call::Info>();
    callInfo->id = callId.toStdString();
    callInfo->status =  call::Status::SEARCHING;
    pimpl_->calls[callId.toStdString()] = callInfo;

    return *callInfo;
}

void
NewCallModel::accept(const std::string& callId) const
{
    CallManager::instance().accept(callId.c_str());
    // TODO change status
    emit callStatusChanged(callId);
}


void
NewCallModel::hangUp(const std::string& callId) const
{
    CallManager::instance().hangUp(callId.c_str());
    // TODO change status
    emit callStatusChanged(callId);
}

void
NewCallModel::togglePause(const std::string& callId) const
{
    auto call = pimpl_->calls.find(callId);
    if (call == pimpl_->calls.end()) return;
    switch(pimpl_->calls[callId]->status)
    {
    case call::Status::PAUSED:
        CallManager::instance().unhold(callId.c_str());
        break;
    case call::Status::IN_PROGRESS:
        CallManager::instance().hold(callId.c_str());
        break;
    case call::Status::INVALID:
    case call::Status::OUTGOING_REQUESTED:
    case call::Status::INCOMING_RINGING:
    case call::Status::OUTGOING_RINGING:
    case call::Status::CONNECTING:
    case call::Status::SEARCHING:
    case call::Status::PEER_PAUSED:
    case call::Status::INACTIVE:
    case call::Status::ENDED:
    case call::Status::TERMINATING:
    case call::Status::CONNECTED:
    case call::Status::AUTO_ANSWERING:
        break;
    }
    // TODO change status
    emit callStatusChanged(callId);
}

void
NewCallModel::toggleMedia(const std::string& callId, const NewCallModel::Media media, bool flag) const
{
    switch(media)
    {
    case NewCallModel::Media::AUDIO:
        CallManager::instance().muteLocalMedia(callId.c_str(),
                                               DRing::Media::Details::MEDIA_TYPE_AUDIO,
                                               flag);
        break;

    case NewCallModel::Media::VIDEO:
        CallManager::instance().muteLocalMedia(callId.c_str(),
                                               DRing::Media::Details::MEDIA_TYPE_AUDIO,
                                               flag);
        break;

    case NewCallModel::Media::NONE:
    default:
        break;
    }
    // TODO change status
    emit callStatusChanged(callId);
}

void
NewCallModel::toggleRecoringdAudio(const std::string& callId) const
{
    CallManager::instance().toggleRecording(callId.c_str());
    // TODO change status
    emit callStatusChanged(callId);
}

void
NewCallModel::setQuality(const std::string& callId, const double quality) const
{
    qDebug() << "setQuality, isn't yet implemented";
}

void
NewCallModel::transfer(const std::string& callId, const std::string& to) const
{
    qDebug() << "transfer, isn't yet implemented";
}

void
NewCallModel::addParticipant(const std::string& callId, const std::string& participant)
{

}

void
NewCallModel::removeParticipant(const std::string& callId, const std::string& participant)
{

}

NewCallModelPimpl::NewCallModelPimpl(NewAccountModel& p)
: parent(p)
{

}

NewCallModelPimpl::~NewCallModelPimpl()
{

}

void
NewCallModelPimpl::sendMessage(const std::string& callId, const std::string& body) const
{
    // cette fonction doit etre appellé depuis le sendmessage de conversation
    // la fonction sendmessage de conversation doit etre friend
    //
    QMap<QString, QString> payloads;
    payloads["text/plain"] = body.c_str();

    CallManager::instance().sendTextMessage(callId.c_str(), payloads, true /* not used */);

    // l'ajout du message dans la db doit se faire depuis conversatoin
}

} // namespace api
} // namespace lrc

#include "api/moc_newcallmodel.cpp"
