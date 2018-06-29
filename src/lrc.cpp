/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                             *
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

// Models and database
#include "api/avmodel.h"
#include "api/behaviorcontroller.h"
#include "api/datatransfermodel.h"
#include "api/newaccountmodel.h"
#include "callbackshandler.h"
#include "database.h"
#include "dbus/callmanager.h"
#include "dbus/configurationmanager.h"
#include "dbus/instancemanager.h"
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class LrcPimpl
{

public:
    LrcPimpl(Lrc& linked);

    const Lrc& linked;
    std::unique_ptr<BehaviorController> behaviorController;
    std::unique_ptr<CallbacksHandler> callbackHandler;
    std::unique_ptr<Database> database;
    std::unique_ptr<NewAccountModel> accountModel;
    std::unique_ptr<DataTransferModel> dataTransferModel;
    std::unique_ptr<AVModel> AVModel_;
};

Lrc::Lrc()
{
    // Ensure Daemon is running/loaded (especially on non-DBus platforms)
    // before instantiating LRC and its members
    InstanceManager::instance();
    lrcPimpl_ = std::make_unique<LrcPimpl>(*this);
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

const NewAccountModel&
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

std::vector<std::string>
Lrc::activeCalls()
{
    QStringList callLists = CallManager::instance().getCallList();
    std::vector<std::string> result;
    result.reserve(callLists.size());
    for (const auto &call : callLists) {
        result.emplace_back(call.toStdString());
    }
    return result;
}

LrcPimpl::LrcPimpl(Lrc& linked)
: linked(linked)
, behaviorController(std::make_unique<BehaviorController>())
, callbackHandler(std::make_unique<CallbacksHandler>(linked))
, database(std::make_unique<Database>())
, accountModel(std::make_unique<NewAccountModel>(linked, *database, *callbackHandler, *behaviorController))
, dataTransferModel {std::make_unique<DataTransferModel>()}
, AVModel_ {std::make_unique<AVModel>(*callbackHandler)}
{
}

} // namespace lrc