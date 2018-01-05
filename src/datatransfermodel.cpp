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

// DRing
#include <datatransfer_interface.h>

// Std
#include <map>

// Qt
#include <QUuid>

namespace lrc { namespace api {

class DataTransferModel::Impl
{
public:
    Impl(Database& database,
         const CallbacksHandler& callbacksHandler,
         const api::BehaviorController& behaviorController);

    std::map<DRing::DataTransferId, std::string> dring2lrcIdMap;
    std::map<std::string, DRing::DataTransferId> lrc2dringIdMap; // stricly the reverse map of dring2lrcIdMap
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
    auto dring_id = static_cast<DRing::DataTransferId>(ConfigurationManager::instance().sendFile(
                                                           QString::fromStdString(account_id),
                                                           QString::fromStdString(peer_uri),
                                                           QString::fromStdString(file_path),
                                                           QString::fromStdString(display_name)));
    while (true) {
        auto res = pimpl_->dring2lrcIdMap.emplace(dring_id, QUuid::createUuid().toString().toStdString());
        if (res.second) {
            pimpl_->lrc2dringIdMap.emplace(res.first->second, dring_id);
            return res.first->second;
        }
    }
}

datatransfer::Info
DataTransferModel::transferInfo(const std::string& lrc_id)
{
    const auto& res = pimpl_->lrc2dringIdMap.find(lrc_id);
    if (res == std::cend(pimpl_->lrc2dringIdMap))
        return {};

    auto dring_info = static_cast<DataTransferInfo>(ConfigurationManager::instance().dataTransferInfo(res->second));
    datatransfer::Info lrc_info;
    lrc_info.uid = lrc_id;
    lrc_info.isOutgoing = dring_info.lastEvent;
    lrc_info.progress = dring_info.lastEvent;
    lrc_info.path = dring_info.displayName;
    lrc_info.displayName = dring_info.displayName;

    // DRING -> LRC event code conversion
    switch (dring_info.lastEvent) {
        case uint(DRing::DataTransferEventCode::created): lrc_info.status = datatransfer::Status::on_connection; break;
        case uint(DRing::DataTransferEventCode::unsupported): lrc_info.status = datatransfer::Status::unsupported; break;
        case uint(DRing::DataTransferEventCode::wait_peer_acceptance): lrc_info.status = datatransfer::Status::on_connection; break;
        case uint(DRing::DataTransferEventCode::wait_host_acceptance): lrc_info.status = datatransfer::Status::on_connection; break;
        case uint(DRing::DataTransferEventCode::ongoing): lrc_info.status = datatransfer::Status::on_progress; break;
        case uint(DRing::DataTransferEventCode::finished): lrc_info.status = datatransfer::Status::success; break;
        case uint(DRing::DataTransferEventCode::closed_by_host): lrc_info.status = datatransfer::Status::stop_by_host; break;
        case uint(DRing::DataTransferEventCode::closed_by_peer): lrc_info.status = datatransfer::Status::stop_by_peer; break;
        case uint(DRing::DataTransferEventCode::invalid_pathname): lrc_info.status = datatransfer::Status::invalid_pathname; break;
        case uint(DRing::DataTransferEventCode::unjoinable_peer): lrc_info.status = datatransfer::Status::unjoinable_peer; break;
    }
    return lrc_info;
}

}} // namespace lrc::api

#include "api/moc_datatransfermodel.cpp"
#include "datatransfermodel.moc"
