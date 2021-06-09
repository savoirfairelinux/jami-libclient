/****************************************************************************
 *    Copyright (C) 2018-2021 Savoir-faire Linux Inc.                                  *
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

// Dbus
#include "dbus/configurationmanager.h"

// DRing
#include <datatransfer_interface.h>

// Std
#include <map>
#include <stdexcept>
#include <type_traits>

// Qt
#include <QDir>
#include <QFileInfo>
#include <QtCore/QStandardPaths>
#include <QUuid>

namespace lrc {
namespace api {

// DRING to LRC event code conversion
static inline datatransfer::Status
convertDataTransferEvent(DRing::DataTransferEventCode event)
{
    switch (event) {
    case DRing::DataTransferEventCode::invalid:
        return datatransfer::Status::INVALID;
    case DRing::DataTransferEventCode::created:
        return datatransfer::Status::on_connection;
    case DRing::DataTransferEventCode::unsupported:
        return datatransfer::Status::unsupported;
    case DRing::DataTransferEventCode::wait_peer_acceptance:
        return datatransfer::Status::on_connection;
    case DRing::DataTransferEventCode::wait_host_acceptance:
        return datatransfer::Status::on_connection;
    case DRing::DataTransferEventCode::ongoing:
        return datatransfer::Status::on_progress;
    case DRing::DataTransferEventCode::finished:
        return datatransfer::Status::success;
    case DRing::DataTransferEventCode::closed_by_host:
        return datatransfer::Status::stop_by_host;
    case DRing::DataTransferEventCode::closed_by_peer:
        return datatransfer::Status::stop_by_peer;
    case DRing::DataTransferEventCode::invalid_pathname:
        return datatransfer::Status::invalid_pathname;
    case DRing::DataTransferEventCode::unjoinable_peer:
        return datatransfer::Status::unjoinable_peer;
    case DRing::DataTransferEventCode::timeout_expired:
        return datatransfer::Status::timeout_expired;
    }
    throw std::runtime_error("BUG: broken convertDataTransferEvent() switch");
}

class DataTransferModel::Impl : public QObject
{
    Q_OBJECT

public:
    Impl(DataTransferModel& up_link);

    QString getUniqueFilePath(const QString& filename);

    DataTransferModel& upLink;
    std::map<long long, int> jamid2lrcIdMap;
    std::map<int, long long> lrc2jamidIdMap; // stricly the reverse map of jamid2lrcIdMap
};

DataTransferModel::Impl::Impl(DataTransferModel& up_link)
    : QObject {}
    , upLink {up_link}
{}

QString
DataTransferModel::Impl::getUniqueFilePath(const QString& filename)
{
    if (!QFile::exists(filename)) {
        return filename;
    }
    QString base(filename);
    QString ext = QFileInfo(filename).completeSuffix();
    if (!ext.isEmpty()) {
        ext = ext.prepend(".");
    }
    base.chop(ext.size());
    QString ret;
    for (int suffix = 1;; suffix++) {
        ret = QString("%1 (%2)%3").arg(base).arg(suffix).arg(ext);
        if (!QFile::exists(ret)) {
            return ret;
        }
    }
}

void
DataTransferModel::registerTransferId(long long jamidId, int interactionId)
{
    pimpl_->jamid2lrcIdMap.emplace(jamidId, interactionId);
    pimpl_->lrc2jamidIdMap.erase(interactionId); // Because a file transfer can be retried
    pimpl_->lrc2jamidIdMap.emplace(interactionId, jamidId);
}

DataTransferModel::DataTransferModel()
    : QObject(nullptr)
    , pimpl_ {std::make_unique<Impl>(*this)}
{}

DataTransferModel::~DataTransferModel() = default;

void
DataTransferModel::transferInfo(long long ringId, datatransfer::Info& lrc_info)
{
    DataTransferInfo infoFromDaemon;
    if (ConfigurationManager::instance().dataTransferInfo(ringId, infoFromDaemon) == 0) {
        // lrc_info.uid = ?
        lrc_info.status = convertDataTransferEvent(
            DRing::DataTransferEventCode(infoFromDaemon.lastEvent));
        lrc_info.isOutgoing = !(infoFromDaemon.flags
                                & (1 << uint32_t(DRing::DataTransferFlags::direction)));
        lrc_info.totalSize = infoFromDaemon.totalSize;
        lrc_info.progress = infoFromDaemon.bytesProgress;
        lrc_info.path = infoFromDaemon.path;
        lrc_info.displayName = infoFromDaemon.displayName;
        lrc_info.accountId = infoFromDaemon.accountId;
        lrc_info.peerUri = infoFromDaemon.peer;
        // lrc_info.timestamp = ?
        return;
    }

    lrc_info.status = datatransfer::Status::INVALID;
}

void
DataTransferModel::sendFile(const QString& account_id,
                            const QString& peer_uri,
                            const QString& file_path,
                            const QString& display_name)
{
    DataTransferInfo info;
#ifdef ENABLE_LIBWRAP
    DRing::DataTransferId id;
#else
    qulonglong id;
#endif
    info.accountId = account_id;
    info.peer = peer_uri;
    info.path = file_path;
    info.displayName = display_name;
    info.bytesProgress = 0;
    if (ConfigurationManager::instance().sendFile(info, id) != 0) {
        qDebug() << "DataTransferModel::sendFile(), error";
        return;
    }
}

void
DataTransferModel::bytesProgress(int interactionId, int64_t& total, int64_t& progress)
{
    ConfigurationManager::instance()
#ifdef ENABLE_LIBWRAP
        .dataTransferBytesProgress(pimpl_->lrc2jamidIdMap.at(interactionId), total, progress);
#else
        .dataTransferBytesProgress(pimpl_->lrc2jamidIdMap.at(interactionId),
                                   reinterpret_cast<qlonglong&>(total),
                                   reinterpret_cast<qlonglong&>(progress));
#endif
}

QString
DataTransferModel::accept(int interactionId, const QString& file_path, std::size_t offset)
{
    auto unique_file_path = pimpl_->getUniqueFilePath(file_path);
    auto jamid_id = pimpl_->lrc2jamidIdMap.at(interactionId);
    ConfigurationManager::instance().acceptFileTransfer(jamid_id, unique_file_path, offset);
    return unique_file_path;
}

void
DataTransferModel::cancel(int interactionId)
{
    auto jamid_id = pimpl_->lrc2jamidIdMap.at(interactionId);
    ConfigurationManager::instance().cancelDataTransfer(jamid_id);
}

int
DataTransferModel::getInteractionIdFromDringId(long long jamidId)
{
    return pimpl_->jamid2lrcIdMap.at(jamidId);
}

long long
DataTransferModel::getDringIdFromInteractionId(int interactionId)
{
    return pimpl_->lrc2jamidIdMap.at(interactionId);
}

QString
DataTransferModel::createDefaultDirectory()
{
    auto defaultDirectory = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation)
                            + "/Jami";
    QDir dir(defaultDirectory);
    if (!dir.exists())
        dir.mkpath(".");
    return defaultDirectory;
}

} // namespace api
} // namespace lrc

#include "api/moc_datatransfermodel.cpp"
#include "datatransfermodel.moc"
