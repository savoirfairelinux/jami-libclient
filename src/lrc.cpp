/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
 *   Author : Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com> *
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
#include "../../daemon/compat/msvc/unistd.h"
#endif // !_MSC_VER

#include "call_const.h"

// Models and database
#include "api/avmodel.h"
#include "api/pluginmodel.h"
#include "api/behaviorcontroller.h"
#include "api/datatransfermodel.h"
#include "api/newaccountmodel.h"
#include "callbackshandler.h"
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/instancemanager.h"
#include "dbus/configurationmanager.h"
#include "authority/storagehelper.h"

namespace lrc {

using namespace api;

std::atomic_bool lrc::api::Lrc::holdConferences;

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
    std::unique_ptr<AVModel> AVModel_;
    std::unique_ptr<PluginModel> PluginModel_;
};

Lrc::Lrc(MigrationCb willDoMigrationCb, MigrationCb didDoMigrationCb, bool muteDring)
{
    lrc::api::Lrc::holdConferences.store(true);
#ifndef ENABLE_LIBWRAP
    // Replace locale for timestamps
    std::locale::global(std::locale(""));
#else
    if (!getenv("JAMI_USE_SHM"))
        setenv("JAMI_USE_SHM", "1", true);
#endif
    // Ensure Daemon is running/loaded (especially on non-DBus platforms)
    // before instantiating LRC and its members
    InstanceManager::instance(muteDring);
    lrcPimpl_ = std::make_unique<LrcPimpl>(*this, willDoMigrationCb, didDoMigrationCb);
}

Lrc::~Lrc()
{
    // Unregister from the daemon
    InstanceManagerInterface& instance = InstanceManager::instance();
    Q_NOREPLY instance.Unregister(getpid());
#ifndef ENABLE_LIBWRAP
    instance.connection().disconnectFromBus(instance.connection().baseService());
#endif // ENABLE_LIBWRAP
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

AVModel&
Lrc::getAVModel() const
{
    return *lrcPimpl_->AVModel_;
}

PluginModel&
Lrc::getPluginModel() const
{
    return *lrcPimpl_->PluginModel_;
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

VectorString
Lrc::activeCalls()
{
    VectorString result;
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();
    for (const auto& accId : accountIds) {
        QStringList callLists = CallManager::instance().getCallList(accId);
        for (const auto& call : callLists) {
            MapStringString callDetails = CallManager::instance().getCallDetails(accId, call);
            if (!isFinished(callDetails[QString(DRing::Call::Details::CALL_STATE)]))
                result.push_back(call);
        }
    }
    return result;
}

void
Lrc::hangupCallsAndConferences()
{
    const QStringList accountIds = ConfigurationManager::instance().getAccountList();
    for (const auto& accId : accountIds) {
        QStringList conferences = CallManager::instance().getConferenceList(accId);
        for (const auto& conf : conferences) {
            CallManager::instance().hangUpConference(accId, conf);
        }
        QStringList calls = CallManager::instance().getCallList(accId);
        for (const auto& call : calls) {
            CallManager::instance().hangUp(accId, call);
        }
    }
}

VectorString
Lrc::getCalls()
{
    QStringList callLists = CallManager::instance().getCallList("");
    VectorString result;
    result.reserve(callLists.size());
    for (const auto& call : callLists) {
        result.push_back(call);
    }
    return result;
}

VectorString
Lrc::getConferences(const QString& accountId)
{
    VectorString result;
    if (accountId.isEmpty()) {
        const QStringList accountIds = ConfigurationManager::instance().getAccountList();
        for (const auto& accId : accountIds) {
            QStringList conferencesList = CallManager::instance().getConferenceList(accId);
            for (const auto& conf : conferencesList)
                result.push_back(conf);
        }
    } else {
        QStringList conferencesList = CallManager::instance().getConferenceList(accountId);
        for (const auto& conf : conferencesList)
            result.push_back(conf);
    }
    return result;
}

bool
isFinished(const QString& callState)
{
    if (callState == QLatin1String(DRing::Call::StateEvent::HUNGUP)
        || callState == QLatin1String(DRing::Call::StateEvent::BUSY)
        || callState == QLatin1String(DRing::Call::StateEvent::PEER_BUSY)
        || callState == QLatin1String(DRing::Call::StateEvent::FAILURE)
        || callState == QLatin1String(DRing::Call::StateEvent::INACTIVE)
        || callState == QLatin1String(DRing::Call::StateEvent::OVER)) {
        return true;
    }
    return false;
}

void
Lrc::monitor(bool continuous)
{
    ConfigurationManager::instance().monitor(continuous);
}

LrcPimpl::LrcPimpl(Lrc& linked, MigrationCb& willMigrateCb, MigrationCb& didMigrateCb)
    : linked(linked)
    , behaviorController(std::make_unique<BehaviorController>())
    , callbackHandler(std::make_unique<CallbacksHandler>(linked))
    , accountModel(std::make_unique<NewAccountModel>(linked,
                                                     *callbackHandler,
                                                     *behaviorController,
                                                     willMigrateCb,
                                                     didMigrateCb))
    , AVModel_ {std::make_unique<AVModel>(*callbackHandler)}
    , PluginModel_ {std::make_unique<PluginModel>()}
{}

} // namespace lrc
