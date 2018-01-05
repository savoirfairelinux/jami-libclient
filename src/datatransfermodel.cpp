/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
 *   Author: Guillaume Roguez <guillaume.roguez@savoirfairelinux.com>       *
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

// LRC
#include "api/datatransfermodel.h"
#include "api/behaviorcontroller.h"
#include "callbackshandler.h"
#include "database.h"

// Dbus
#include "dbus/configurationmanager.h"

// Std
#include <map>

namespace lrc { namespace api {

class DataTransferModel::Impl
{
public:
    Impl(Database& database,
         const CallbacksHandler& callbacksHandler,
         const api::BehaviorController& behaviorController);

    Database& database;
    const CallbacksHandler& callbacksHandler;
    const BehaviorController& behaviorController;
};

DataTransferModel::Impl::Impl(Database& database,
                              const CallbacksHandler& callbacksHandler,
                              const api::BehaviorController& behaviorController)
    : behaviorController(behaviorController)
    , callbacksHandler(callbacksHandler)
    , database(database)
{}

DataTransferModel::DataTransferModel(Database& database,
                                     const CallbacksHandler& callbacksHandler,
                                     const api::BehaviorController& behaviorController)
    : QObject()
    , pimpl_ { std::make_unique<Impl>(database, callbacksHandler, behaviorController) }
{}

DataTransferModel::~DataTransferModel()
{}

std::string
DataTransferModel::sendFile(const std::string& account_id, const std::string& peer_uri,
                            const std::string& file_path, const std::string& display_name)
{
    return "";
}

datatransfer::Info
DataTransferModel::transferInfo(const std::string& uid)
{
    return {};
}

}} // namespace lrc::api

#include "api/moc_datatransfermodel.cpp"
#include "datatransfermodel.moc"
