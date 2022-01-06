/****************************************************************************
 *    Copyright (C) 2018-2022 Savoir-faire Linux Inc.                       *
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

    QString getUniqueFilePath(const QString& filename, const QString& path = "");

    DataTransferModel& upLink;
    MapStringString file2InteractionId;
    MapStringString interactionToFileId; // stricly the reverse map of file2InteractionId
};

DataTransferModel::Impl::Impl(DataTransferModel& up_link)
    : QObject {}
    , upLink {up_link}
{}

QString
DataTransferModel::Impl::getUniqueFilePath(const QString& filename, const QString& path)
{
    auto base = filename;
    QString ext = QFileInfo(base).completeSuffix();
    if (!ext.isEmpty())
        ext = ext.prepend(".");

    QFileInfo fi(filename);
    auto p = !path.isEmpty() ? path : fi.dir().path();
    base = QDir(p).filePath(fi.baseName() + ext);
    if (!QFile::exists(base))
        return base;

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
DataTransferModel::registerTransferId(const QString& fileId, const QString& interactionId)
{
    pimpl_->file2InteractionId[fileId] = interactionId;
    pimpl_->interactionToFileId.remove(interactionId); // Because a file transfer can be retried
    pimpl_->interactionToFileId[interactionId] = fileId;
}

DataTransferModel::DataTransferModel()
    : QObject(nullptr)
    , pimpl_ {std::make_unique<Impl>(*this)}
{}

DataTransferModel::~DataTransferModel() = default;

void
DataTransferModel::transferInfo(const QString& accountId,
                                const QString& fileId,
                                datatransfer::Info& lrc_info)
{
    DataTransferInfo infoFromDaemon;
    if (ConfigurationManager::instance().dataTransferInfo(accountId, fileId, infoFromDaemon) == 0) {
        lrc_info.uid = fileId;
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
        lrc_info.conversationId = infoFromDaemon.conversationId;
        // lrc_info.timestamp = ?
        return;
    }

    lrc_info.status = datatransfer::Status::INVALID;
}

void
DataTransferModel::sendFile(const QString& accountId,
                            const QString& peer_uri,
                            const QString& conversationId,
                            const QString& filePath,
                            const QString& displayName)
{
    if (conversationId.isEmpty()) {
        // Fallback
        DataTransferInfo info;
#ifdef ENABLE_LIBWRAP
        DRing::DataTransferId id;
#else
        qulonglong id;
#endif
        info.accountId = accountId;
        info.peer = peer_uri;
        info.path = filePath;
        info.conversationId = conversationId;
        info.displayName = displayName;
        info.bytesProgress = 0;
        if (ConfigurationManager::instance().sendFileLegacy(info, id) != 0)
            qWarning() << "DataTransferModel::sendFile(), error";
        return;
    }

    ConfigurationManager::instance().sendFile(accountId,
                                              conversationId,
                                              filePath,
                                              displayName,
                                              {} /* TODO parent */);
}

void
DataTransferModel::fileTransferInfo(const QString& accountId,
                                    const QString& conversationId,
                                    const QString& fileId,
                                    QString& path,
                                    qlonglong& total,
                                    qlonglong& progress)
{
    ConfigurationManager::instance()
        .fileTransferInfo(accountId, conversationId, fileId, path, total, progress);
}

QString
DataTransferModel::accept(const QString& accountId, const QString& fileId, const QString& filePath)
{
    auto uniqueFilePath = pimpl_->getUniqueFilePath(filePath);
    auto daemonFileId = pimpl_->interactionToFileId[fileId];
    ConfigurationManager::instance().acceptFileTransfer(accountId, daemonFileId, uniqueFilePath);
    return uniqueFilePath;
}

void
DataTransferModel::download(const QString& accountId,
                            const QString& convId,
                            const QString& interactionId,
                            const QString& fileId,
                            const QString& path)
{
    ConfigurationManager::instance().downloadFile(accountId, convId, interactionId, fileId, path);
}

void
DataTransferModel::copyTo(const QString& accountId,
                          const QString& convId,
                          const QString& interactionId,
                          const QString& destPath,
                          const QString& displayName)
{
    auto fileId = getFileIdFromInteractionId(interactionId);
    if (fileId.isEmpty()) {
        qWarning() << "Cannot find any file for " << interactionId;
        return;
    }
    QString path;
    qlonglong total, progress;

    fileTransferInfo(accountId, convId, fileId, path, total, progress);

    auto src = QFile(path);
    auto srcfi = QFileInfo(path);
    if (!src.exists())
        return;

    auto filename = displayName;
    if (displayName.isEmpty())
        filename = srcfi.isSymLink() ? srcfi.symLinkTarget() : path;
    auto dest = pimpl_->getUniqueFilePath(filename, destPath);
    qDebug() << "Copy to " << dest;
    src.copy(dest);
}

void
DataTransferModel::cancel(const QString& accountId,
                          const QString& conversationId,
                          const QString& interactionId)
{
    ConfigurationManager::instance().cancelDataTransfer(accountId,
                                                        conversationId,
                                                        getFileIdFromInteractionId(interactionId));
}

QString
DataTransferModel::getInteractionIdFromFileId(const QString& fileId)
{
    return pimpl_->file2InteractionId[fileId];
}

QString
DataTransferModel::getFileIdFromInteractionId(const QString& interactionId)
{
    return pimpl_->interactionToFileId[interactionId];
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
