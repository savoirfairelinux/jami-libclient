/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
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
#include "api/lrc.h"

#include <locale>

#ifndef _MSC_VER
#include <unistd.h>
#else
#include "../../daemon/MSVC/unistd.h"
#endif // !_MSC_VER

#include "call_const.h"

// Models and database
#include "api/avmodel.h"
#include "api/behaviorcontroller.h"
#include "api/datatransfermodel.h"
#include "api/newaccountmodel.h"
#include "callbackshandler.h"
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/instancemanager.h"
#include "dbus/configurationmanager.h"
#include "authority/storagehelper.h"

namespace lrc
{

using namespace api;

// To judge whether the call is finished or not depending on callState
bool isFinished(const QString& callState);

class LrcPimpl
{

public:
    LrcPimpl(Lrc& linked, MigrationCb& willMigrateCb, MigrationCb& didMigrateCb);

    const Lrc& linked;
    std::unique_ptr<BehaviorController> behaviorController;
    std::unique_ptr<CallbacksHandler> callbackHandler;
    std::unique_ptr<NewAccountModel> accountModel;
    std::unique_ptr<DataTransferModel> dataTransferModel;
    std::unique_ptr<AVModel> AVModel_;

};

Lrc::Lrc(MigrationCb willDoMigrationCb, MigrationCb didDoMigrationCb)
{
    // Replace locale for timestamps
    std::locale::global(std::locale(""));
    // Ensure Daemon is running/loaded (especially on non-DBus platforms)
    // before instantiating LRC and its members
    InstanceManager::instance();
    lrcPimpl_ = std::make_unique<LrcPimpl>(*this, willDoMigrationCb, didDoMigrationCb);
}

Lrc::~Lrc()
{
    //Unregister from the daemon
    InstanceManagerInterface& instance = InstanceManager::instance();
    Q_NOREPLY instance.Unregister(getpid());
#ifndef ENABLE_LIBWRAP
    instance.connection().disconnectFromBus(instance.connection().baseService());
#endif //ENABLE_LIBWRAP
}

NewAccountModel&
Lrc::getAccountModel() const
{
    return *lrcPimpl_->accountModel;
}

BehaviorController&
Lrc::getBehaviorController() const
{
    return *lrcPimpl_->behaviorController;
}

DataTransferModel&
Lrc::getDataTransferModel() const
{
    return *lrcPimpl_->dataTransferModel;
}

AVModel&
Lrc::getAVModel() const
{
    return *lrcPimpl_->AVModel_;
}

void
Lrc::connectivityChanged() const
{
    ConfigurationManager::instance().connectivityChanged();
}

bool
Lrc::isConnected()
{
#ifdef ENABLE_LIBWRAP
    return true;
#else
    return ConfigurationManager::instance().connection().isConnected();
#endif
}

bool
Lrc::dbusIsValid()
{
#ifdef ENABLE_LIBWRAP
    return true;
#else
    return ConfigurationManager::instance().isValid();
#endif
}

void
Lrc::subscribeToDebugReceived()
{
    lrcPimpl_->callbackHandler->subscribeToDebugReceived();
}

std::vector<std::string>
Lrc::activeCalls()
{
    QStringList callLists = CallManager::instance().getCallList();
    std::vector<std::string> result;
    result.reserve(callLists.size());
    for (const auto &call : callLists) {
        MapStringString callDetails = CallManager::instance().getCallDetails(call);
        if(!isFinished(callDetails[QString(DRing::Call::Details::CALL_STATE)]))
            result.emplace_back(call.toStdString());
    }
    return result;
}

std::vector<std::string>
Lrc::getCalls()
{
    QStringList callLists = CallManager::instance().getCallList();
    std::vector<std::string> result;
    result.reserve(callLists.size());
    for (const auto &call : callLists) {
        result.emplace_back(call.toStdString());
    }
    return result;
}

std::vector<std::string>
Lrc::getConferences()
{
    QStringList conferencesList = CallManager::instance().getConferenceList();
    std::vector<std::string> result;
    result.reserve(conferencesList.size());
    for (const auto &conf : conferencesList) {
        result.emplace_back(conf.toStdString());
    }
    return result;
}

std::vector<std::string>
Lrc::getConferenceSubcalls(const std::string& cid)
{
    QStringList callList = CallManager::instance().getParticipantList(cid.c_str());
    std::vector<std::string> result;
    result.reserve(callList.size());
    foreach(const auto& callId, callList) {
        result.emplace_back(callId.toStdString());
    }
    return result;
}

bool
isFinished(const QString& callState)
{
    if (callState == QLatin1String(DRing::Call::StateEvent::HUNGUP) ||
        callState == QLatin1String(DRing::Call::StateEvent::BUSY) ||
        callState == QLatin1String(DRing::Call::StateEvent::PEER_BUSY) ||
        callState == QLatin1String(DRing::Call::StateEvent::FAILURE) ||
        callState == QLatin1String(DRing::Call::StateEvent::INACTIVE) ||
        callState == QLatin1String(DRing::Call::StateEvent::OVER)) {
        return true;
    }
    return false;
}

LrcPimpl::LrcPimpl(Lrc& linked, MigrationCb& willMigrateCb, MigrationCb& didMigrateCb)
: linked(linked)
, behaviorController(std::make_unique<BehaviorController>())
, callbackHandler(std::make_unique<CallbacksHandler>(linked))
, accountModel(std::make_unique<NewAccountModel>(linked, *callbackHandler, *behaviorController, willMigrateCb, didMigrateCb))
, dataTransferModel {std::make_unique<DataTransferModel>()}
, AVModel_ {std::make_unique<AVModel>(*callbackHandler)}
{
}

} // namespace lrc
