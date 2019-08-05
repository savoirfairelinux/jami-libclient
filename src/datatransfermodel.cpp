/****************************************************************************
 *    Copyright (C) 2018-2019 Savoir-faire Linux Inc.                                  *
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

#include "daemonproxy.h"

// DRing
#include <datatransfer_interface.h>

// Std
#include <map>
#include <stdexcept>
#include <type_traits>

// Qt
#include <QUuid>

namespace lrc { namespace api {

// DRING to LRC event code conversion
static inline
datatransfer::Status
convertDataTransferEvent(DRing::DataTransferEventCode event)
{
    switch (event) {
        case DRing::DataTransferEventCode::invalid: return datatransfer::Status::INVALID;
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
        case DRing::DataTransferEventCode::timeout_expired: return datatransfer::Status::timeout_expired;
    }
    throw std::runtime_error("BUG: broken convertDataTransferEvent() switch");
}

class DataTransferModel::Impl : public QObject
{
    Q_OBJECT

public:
    Impl(DataTransferModel& up_link);

    std::vector<std::string> transferIdList() const;

    DataTransferModel& upLink;
    std::map<long long, int> dring2lrcIdMap;
    std::map<int, long long> lrc2dringIdMap; // stricly the reverse map of dring2lrcIdMap
};

DataTransferModel::Impl::Impl(DataTransferModel& up_link)
    : QObject {}
    , upLink {up_link}
{}

void
DataTransferModel::registerTransferId(long long dringId, int interactionId)
{

    pimpl_->dring2lrcIdMap.emplace(dringId, interactionId);
    pimpl_->lrc2dringIdMap.emplace(interactionId, dringId);
}

DataTransferModel::DataTransferModel()
    : QObject()
    , pimpl_ { std::make_unique<Impl>(*this) }
{}

DataTransferModel::~DataTransferModel() = default;

std::vector<std::string>
DataTransferModel::Impl::transferIdList() const
{
    std::vector<DataTransferId> dring_list = DaemonProxy::instance().dataTransferList();
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
DataTransferModel::transferInfo(long long ringId, datatransfer::Info& lrc_info)
{
    DataTransferInfo infoFromDaemon;
    if (DaemonProxy::instance().dataTransferInfo(ringId, infoFromDaemon) == DRing::DataTransferError::success) {
        //lrc_info.uid = ?
        lrc_info.status = convertDataTransferEvent(DRing::DataTransferEventCode(infoFromDaemon.lastEvent));
        lrc_info.isOutgoing = !(infoFromDaemon.flags & (1 << uint32_t(DRing::DataTransferFlags::direction)));
        lrc_info.totalSize = infoFromDaemon.totalSize;
        lrc_info.progress = infoFromDaemon.bytesProgress;
        lrc_info.path = infoFromDaemon.path;
        lrc_info.displayName = infoFromDaemon.displayName;
        lrc_info.accountId = infoFromDaemon.accountId;
        lrc_info.peerUri = infoFromDaemon.peer;
        //lrc_info.timestamp = ?
        return;
    }

    lrc_info.status = datatransfer::Status::INVALID;
}

void
DataTransferModel::sendFile(const std::string& account_id, const std::string& peer_uri,
                            const std::string& file_path, const std::string& display_name)
{
    DataTransferInfo info;
    uint64_t id;
    info.accountId = account_id;
    info.peer = peer_uri;
    info.path = file_path;
    info.displayName = display_name;
    info.bytesProgress = 0;
    if (static_cast<uint32_t>(DaemonProxy::instance().sendFile(info, id)) != 0) {
        qDebug() << "DataTransferModel::sendFile(), error";
        return;
    }
}

void
DataTransferModel::bytesProgress(int interactionId, int64_t& total, int64_t& progress)
{
    DaemonProxy::instance().dataTransferBytesProgress(pimpl_->lrc2dringIdMap.at(interactionId),
                                                               total,
                                                               progress);
}

void
DataTransferModel::accept(int interactionId, const std::string& file_path, std::size_t offset)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(interactionId);
    DaemonProxy::instance().acceptFileTransfer(dring_id, file_path, offset);
}

void
DataTransferModel::cancel(int interactionId)
{
    auto dring_id = pimpl_->lrc2dringIdMap.at(interactionId);
    DaemonProxy::instance().cancelDataTransfer(dring_id);
}

int
DataTransferModel::getInteractionIdFromDringId(long long dringId)
{
    return pimpl_->dring2lrcIdMap.at(dringId);
}

long long
DataTransferModel::getDringIdFromInteractionId(int interactionId)
{
    return pimpl_->lrc2dringIdMap.at(interactionId);
}

}} // namespace lrc::api

#include "datatransfermodel.moc"
