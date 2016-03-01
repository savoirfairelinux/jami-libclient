/****************************************************************************
 *   Copyright (C) 2016 by Savoir-faire Linux                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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
#include "filetransfermodel.h"

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>
#include <QtCore/QStandardPaths>

//Ring daemon
#include <account_const.h>

//Ring
#include "dbus/datatransfermanager.h"
#include <account.h>
#include <transfer.h>
#include <accountmodel.h>
#include <private/account_p.h>
#include  <contactmethod.h>

class FileTransferModelPrivate final : public QObject {
    Q_OBJECT
public:
    FileTransferModelPrivate(FileTransferModel*);

    QVector<Transfer*>              m_lTransfers;
    QHash<QString, int>             m_shTransferId; // key: transferID value: index of transfer;
    QItemSelectionModel*            m_pSelectionModel;

public Q_SLOTS:
    void slotDataTransferStatusChanged(const QString& /*transferID*/,
                                        int /*code*/);

private:
    FileTransferModel* q_ptr;

    void insertNewTransfer(const QString& transferID, int code);
    void updateTransfer(const QString& transferID, int code);
    void removeTransfer(const QString& transferID);
    void updateIndexes();
};

FileTransferModelPrivate::FileTransferModelPrivate(FileTransferModel* parent): QObject(parent),q_ptr(parent),m_pSelectionModel(nullptr)
{
    auto& dataTransferManager = DataTransferManager::instance();
    connect(&dataTransferManager, &DataTransferManagerInterface::dataTransferStatusChanged,this ,
            &FileTransferModelPrivate::slotDataTransferStatusChanged, Qt::QueuedConnection);
}

void
FileTransferModelPrivate::slotDataTransferStatusChanged(const QString& transferID,
                                                        int code)
{
    qDebug() << "slotDataTransferStatusChanged" << transferID << "code:" << code;
    if (!m_shTransferId.contains(transferID)) {
        insertNewTransfer(transferID, code);
    } else
        updateTransfer(transferID, code);
}

void
FileTransferModelPrivate::updateIndexes()
{
    for (int i = 0; i < m_lTransfers.size(); i++) {
        m_shTransferId[m_lTransfers[i]->id()] = i;
    }
}

void
FileTransferModelPrivate::insertNewTransfer(const QString& transferID, int code) {
    auto& dataTransferManager = DataTransferManager::instance();

    DRing::DataTransferInfo tmp;
    dataTransferManager.dataTransferInfo(transferID, tmp);

    DRing::DataConnectionInfo tmp2;
    dataTransferManager.dataConnectionInfo(QString(tmp.connectionId.c_str()), tmp2);


    auto toInsert = new Transfer(AccountModel::instance().getById(tmp2.account.c_str()), transferID, code != DRing::DataTransferCode::CODE_NOTIFYING);
    connect(toInsert, &Transfer::changed, this, [this, toInsert] {
                                                    if (m_shTransferId.contains(toInsert->id()))
                                                        updateTransfer(toInsert->id(), toInsert->status());
                                            });

    toInsert->setStatus((DRing::DataTransferCode) code);
    toInsert->setFilename(QString(tmp.name.c_str()));
    toInsert->setSize(tmp.size);

    q_ptr->beginInsertRows(QModelIndex(), 0, 0);
    // prepend the new transfer
    m_lTransfers.insert(0,toInsert);
    updateIndexes();
    q_ptr->endInsertRows();

    if (!toInsert->isOutgoing()) {
        emit q_ptr->incomingTransfer();
    }
}

void
FileTransferModelPrivate::updateTransfer(const QString& transferID, int code) {
    auto& dataTransferManager = DataTransferManager::instance();
    int index = m_shTransferId[transferID];
    auto toUpdate = m_lTransfers[index];
    DRing::DataTransferInfo tmp;
    dataTransferManager.dataTransferInfo(transferID, tmp);
    toUpdate->setStatus((DRing::DataTransferCode) code);
    toUpdate->setSent(dataTransferManager.dataTransferSentBytes(transferID));

    qDebug() << "sent:" << toUpdate->sent() << "/" << toUpdate->size();

    emit q_ptr->dataChanged(q_ptr->index(index, 0), q_ptr->index(index, 0));
}

void
FileTransferModelPrivate::removeTransfer(const QString& transferID) {
    int index = m_shTransferId[transferID];
    q_ptr->beginRemoveRows(QModelIndex(),index,index);
    m_shTransferId.remove(transferID);
    m_lTransfers.remove(index);
    updateIndexes();
    m_lTransfers.squeeze();
    m_shTransferId.squeeze();
    q_ptr->endRemoveRows();
}

FileTransferModel::FileTransferModel(QObject* parent) : QAbstractListModel(parent ? parent : QCoreApplication::instance()),
d_ptr(new FileTransferModelPrivate(this))
{
    setObjectName("FileTransferModel");
}

FileTransferModel::~FileTransferModel()
{
    //TODO: check what needs to be freed
    //while (d_ptr->m_lTransfers.size()) {
    //   d_ptr->m_lTransfers.remove(0);
  // }
    delete d_ptr;
}

FileTransferModel& FileTransferModel::instance()
{
    static auto instance = new FileTransferModel();
    return *instance;
}

QHash<int,QByteArray> FileTransferModel::roleNames() const
{
    static QHash<int, QByteArray> roles = QAbstractListModel::roleNames();
    static bool initRoles = false;
    if (!initRoles) {
      initRoles = true;
      roles[ (int) Role::Status]    = "status"; //State for transfer
      roles[ (int) Role::Progress]  = "progress"; //Progress of transfer
      roles[ (int) Role::isWaitingAcceptance] = "isWaitingAcceptance";
    }
    return roles;
}

Transfer* FileTransferModel::getTransferByModelIndex(const QModelIndex& item) const
{
   if (!item.isValid())
      return nullptr;
   return d_ptr->m_lTransfers[item.row()];
}

//Model functions
QVariant FileTransferModel::data( const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    switch(role) {
        case (int)Ring::Role::DisplayRole:
            return d_ptr->m_lTransfers[index.row()]->filename();
        case (int)Ring::Role::State:
           return d_ptr->m_lTransfers[index.row()]->status();
        case (int)Ring::Role::FormattedState:
            return d_ptr->m_lTransfers[index.row()]->dataTransferCodeToString();
        case (int)Role::Progress:
            return d_ptr->m_lTransfers[index.row()]->progress();
        case (int)Role::isWaitingAcceptance:
            return d_ptr->m_lTransfers[index.row()]->isPending();
        case (int)Ring::Role::Number:
            return d_ptr->m_lTransfers[index.row()]->contactMethod() ? d_ptr->m_lTransfers[index.row()]->contactMethod()->uri() : tr("Unknown");
    };
    return QVariant();
}

void
FileTransferModel::sendFile(const Account* account, const QString& peerUri, const QString& filepath)
{
    QStringList pieces = filepath.split("/");
    QString displayname = pieces.value(pieces.length() - 1);
    qDebug() << displayname;
    auto& dataTransferManager = DataTransferManager::instance();
    dataTransferManager.sendFile(account->id(),peerUri,filepath, displayname);
}

void
FileTransferModel::acceptTransferByModelIndex(const QModelIndex& item, const QString& pathname)
{
    if (!item.isValid())
        return;

    QString finalPath = pathname;
    if (finalPath.isEmpty()) {
        finalPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    }

    if (auto transfer = d_ptr->m_lTransfers[item.row()]) {
        transfer->accept(pathname);
    }
}

void
FileTransferModel::cancelTransferByModelIndex(const QModelIndex& item) const
{
    if (!item.isValid())
        return;
    if (auto transfer = d_ptr->m_lTransfers[item.row()]) {
        transfer->cancel();
    }
}

int FileTransferModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid() ? 0 : d_ptr->m_lTransfers.size();
}

Qt::ItemFlags FileTransferModel::flags( const QModelIndex& index ) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
        return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QItemSelectionModel* FileTransferModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel)
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<FileTransferModel*>(this));

   return d_ptr->m_pSelectionModel;
}

bool FileTransferModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
    Q_UNUSED(index)
    Q_UNUSED(value)
    Q_UNUSED(role )
    return false;
}

#include <filetransfermodel.moc>
