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
#include <type_traits>

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

    std::vector<std::string> transferIdList() const;

    DataTransferModel& upLink;
    std::map<long long, int> dring2lrcIdMap;
    std::map<int, long long> lrc2dringIdMap; // stricly the reverse map of dring2lrcIdMap
    Database& database;
    const CallbacksHandler& callbacksHandler;
};

DataTransferModel::Impl::Impl(DataTransferModel& up_link,
                              Database& database,
                              const CallbacksHandler& callbacksHandler)
    : QObject {}
    , callbacksHandler {callbacksHandler}
    , database {database}
    , upLink {up_link}
{}


void
DataTransferModel::registerTransferId(long long dringId, int interactionId)
{

    pimpl_->dring2lrcIdMap.emplace(dringId, interactionId);
    pimpl_->lrc2dringIdMap.emplace(interactionId, dringId);
}


DataTransferModel::DataTransferModel(Database& database,
                                     const CallbacksHandler& callbacksHandler)
    : QObject()
    , pimpl_ { std::make_unique<Impl>(*this, database, callbacksHandler) }
{}

DataTransferModel::~DataTransferModel() = default;

std::vector<std::string>
DataTransferModel::Impl::transferIdList() const
{
    VectorULongLong dring_list = ConfigurationManager::instance().dataTransferList();
    //~ for (auto dring_id : dring_list) {
         //~ pimpl_->registerTransferId(dring_id);
    //~ }
    std::vector<std::string> result;
    //~ result.reserve(dring_list.size());
    //~ for (auto& item : pimpl_->lrc2dringIdMap) {
        //~ result.push_back(item.first);
    //~ }
    return result;
}

void
DataTransferModel::sendFile(const std::string& account_id, const std::string& peer_uri,
                            const std::string& file_path, const std::string& display_name)
{
    DataTransferInfo info;
    qulonglong id;
    info.accountId = QString::fromStdString(account_id);
    info.peer = QString::fromStdString(peer_uri);
    info.path = QString::fromStdString(file_path);
    info.displayName = QString::fromStdString(display_name);
    if (ConfigurationManager::instance().sendFile(info, id) != 0) {
        qDebug() << "DataTransferModel::sendFile(), error";
        return;
    }
}

void
DataTransferModel::bytesProgress(int interactionId, int64_t& total, int64_t& progress)
{
    ConfigurationManager::instance().dataTransferBytesProgress(pimpl_->lrc2dringIdMap.at(interactionId),
                                                               reinterpret_cast<qlonglong&>(total),
                                                               reinterpret_cast<qlonglong&>(progress));
}

void
DataTransferModel::accept(int interactionId,
                          const std::string& file_path,
                          std::size_t offset)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(interactionId);
    ConfigurationManager::instance().acceptFileTransfer(dring_id, QString::fromStdString(file_path), offset);
}

void
DataTransferModel::cancel(int interactionId)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(interactionId);
    ConfigurationManager::instance().cancelDataTransfer(dring_id);
}

int
DataTransferModel::getInteractionIdFromDringId(long long dringId)
{
    return pimpl_->dring2lrcIdMap.at(dringId);
}

}} // namespace lrc::api

#include "api/moc_datatransfermodel.cpp"
#include "datatransfermodel.moc"
