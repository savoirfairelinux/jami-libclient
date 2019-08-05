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
#include "daemonproxy.h"
#include "authority/storagehelper.h"

namespace lrc
{

using namespace api;

// To judge whether the call is finished or not depending on callState
bool isFinished(const std::string& callState);

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
    // Ensure Daemon is running/loaded (especially on non-DBus platforms)
    // before instantiating LRC and its members
    DaemonProxy::instance();
    lrcPimpl_ = std::make_unique<LrcPimpl>(*this, willDoMigrationCb, didDoMigrationCb);
}

Lrc::~Lrc()
{
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
    DaemonProxy::instance().connectivityChanged();
}

bool
Lrc::isConnected()
{
    return true;
}

bool
Lrc::dbusIsValid()
{
    return true;
}

void
Lrc::subscribeToDebugReceived()
{
    lrcPimpl_->callbackHandler->subscribeToDebugReceived();
}

std::vector<std::string>
Lrc::activeCalls()
{
    std::vector<std::string> callLists = DaemonProxy::instance().getCallList();
    for (auto iter = callLists.begin(); iter != callLists.end(); ++iter) {
        std::map<std::string, std::string> callDetails = DaemonProxy::instance().getCallDetails(*iter);
        if(isFinished(callDetails.at(DRing::Call::Details::CALL_STATE)))
            callLists.erase(iter);
    }
    return callLists;
}

bool
isFinished(const std::string& callState)
{
    if (callState == DRing::Call::StateEvent::HUNGUP ||
        callState == DRing::Call::StateEvent::BUSY ||
        callState == DRing::Call::StateEvent::PEER_BUSY ||
        callState == DRing::Call::StateEvent::FAILURE ||
        callState == DRing::Call::StateEvent::INACTIVE ||
        callState == DRing::Call::StateEvent::OVER) {
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
