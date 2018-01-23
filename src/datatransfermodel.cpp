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
#include "callbackshandler.h"
#include "database.h"

// Dbus
#include "dbus/configurationmanager.h"

// DRing
#include <datatransfer_interface.h>

// Std
#include <map>
#include <stdexcept>

// Qt
#include <QUuid>

namespace lrc { namespace api {

/// DRING to LRC event code conversion
static inline
datatransfer::Status
convertDataTransferEvent(DRing::DataTransferEventCode event)
{
    switch (event) {
        case DRing::DataTransferEventCode::created: return datatransfer::Status::on_connection;
        case DRing::DataTransferEventCode::unsupported: return datatransfer::Status::unsupported;
        case DRing::DataTransferEventCode::wait_peer_acceptance: return datatransfer::Status::on_connection;
        case DRing::DataTransferEventCode::wait_host_acceptance: return datatransfer::Status::on_connection;
        case DRing::DataTransferEventCode::ongoing: return datatransfer::Status::on_progress;
        case DRing::DataTransferEventCode::finished: return datatransfer::Status::success;
        case DRing::DataTransferEventCode::closed_by_host: return datatransfer::Status::stop_by_host;
        case DRing::DataTransferEventCode::closed_by_peer: return datatransfer::Status::stop_by_peer;
        case DRing::DataTransferEventCode::invalid_pathname: return datatransfer::Status::invalid_pathname;
        case DRing::DataTransferEventCode::unjoinable_peer: return datatransfer::Status::unjoinable_peer;
    }
    throw std::runtime_error("BUG: broken convertDataTransferEvent() switch");
}

class DataTransferModel::Impl : public QObject
{
    Q_OBJECT

public:
    Impl(DataTransferModel& up_link,
         Database& database,
         const CallbacksHandler& callbacksHandler);

    DataTransferModel& upLink;
    std::map<DRing::DataTransferId, std::string> dring2lrcIdMap;
    std::map<std::string, DRing::DataTransferId> lrc2dringIdMap; // stricly the reverse map of dring2lrcIdMap
    Database& database;
    const CallbacksHandler& callbacksHandler;

    std::string registerTransferId(DRing::DataTransferId id);

public Q_SLOTS:
    void slotDataTransferEvent(long long dring_id, uint code);
};

DataTransferModel::Impl::Impl(DataTransferModel& up_link,
                              Database& database,
                              const CallbacksHandler& callbacksHandler)
    : QObject {}
    , callbacksHandler {callbacksHandler}
    , database {database}
    , upLink {up_link}
{
    connect(&callbacksHandler, &CallbacksHandler::incomingTransfer,
            this, &DataTransferModel::Impl::slotDataTransferEvent);
}

std::string
DataTransferModel::Impl::registerTransferId(DRing::DataTransferId dring_id)
{
    const auto& iter = dring2lrcIdMap.find(dring_id);
    if (iter != std::cend(dring2lrcIdMap))
        return iter->second;
    while (true) {
        auto res = dring2lrcIdMap.emplace(dring_id, QUuid::createUuid().toString().toStdString());
        if (res.second) {
            lrc2dringIdMap.emplace(res.first->second, dring_id);
            return res.first->second;
        }
    }
}

void
DataTransferModel::Impl::slotDataTransferEvent(long long dring_id, uint code)
{
    emit upLink.incomingTransfer("", "", 0, 0);
}

DataTransferModel::DataTransferModel(Database& database,
                                     const CallbacksHandler& callbacksHandler)
    : QObject()
    , pimpl_ { std::make_unique<Impl>(*this, database, callbacksHandler) }
{}

DataTransferModel::~DataTransferModel() = default;

std::vector<std::string>
DataTransferModel::transferIdList() const
{
    VectorULongLong dring_list = ConfigurationManager::instance().dataTransferList();
    for (auto dring_id : dring_list) {
         pimpl_->registerTransferId(dring_id);
    }
    std::vector<std::string> result;
    result.reserve(dring_list.size());
    for (auto& item : pimpl_->lrc2dringIdMap) {
        result.push_back(item.first);
    }
    return result;
}

std::string
DataTransferModel::sendFile(const std::string& account_id, const std::string& peer_uri,
                            const std::string& file_path, const std::string& display_name)
{
    auto dring_id = static_cast<DRing::DataTransferId>(ConfigurationManager::instance().sendFile(
                                                           QString::fromStdString(account_id),
                                                           QString::fromStdString(peer_uri),
                                                           QString::fromStdString(file_path),
                                                           QString::fromStdString(display_name)));
    return pimpl_->registerTransferId(dring_id);
}

datatransfer::Info
DataTransferModel::transferInfo(const std::string& lrc_id)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(lrc_id);
    auto dring_info = static_cast<DataTransferInfo>(ConfigurationManager::instance().dataTransferInfo(dring_id));
    datatransfer::Info lrc_info;
    lrc_info.uid = lrc_id;
    lrc_info.isOutgoing = dring_info.isOutgoing;
    lrc_info.totalSize = dring_info.totalSize;
    lrc_info.progress = dring_info.lastEvent;
    lrc_info.path = dring_info.displayName.toStdString();
    lrc_info.displayName = dring_info.displayName.toStdString();
    lrc_info.status = convertDataTransferEvent(DRing::DataTransferEventCode(dring_info.lastEvent));
    lrc_info.accountId = dring_info.accountId.toStdString();
    lrc_info.peerUri = dring_info.peer.toStdString();
    return lrc_info;
}

std::streamsize
DataTransferModel::bytesProgress(const std::string& lrc_id)
{
    return ConfigurationManager::instance().dataTransferBytesProgress(pimpl_->lrc2dringIdMap.at(lrc_id));
}

void
DataTransferModel::acceptFile(const std::string& lrc_id,
                                      const std::string& file_path,
                                      std::size_t offset)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(lrc_id);
    ConfigurationManager::instance().acceptFileTransfer(dring_id, QString::fromStdString(file_path), offset);
}

void
DataTransferModel::cancel(const std::string& lrc_id)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(lrc_id);
    ConfigurationManager::instance().cancelDataTransfer(dring_id);
}

}} // namespace lrc::api

#include "api/moc_datatransfermodel.cpp"
#include "datatransfermodel.moc"
