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

//Ring daemon
#include <account_const.h>

//Ring
#include "dbus/datatransfermanager.h"
#include <account.h>
#include <transfer.h>
#include <accountmodel.h>
#include <private/account_p.h>

class FileTransferModelPrivate final : public QObject {
    Q_OBJECT
public:
    FileTransferModelPrivate(FileTransferModel*);

    QVector<Transfer*>              m_lTransfers;
    QHash<QString, int>             m_shTransferId; // key: transferID value: index of transfer;
    QItemSelectionModel*            m_pSelectionModel;

public Q_SLOTS:
    void slotFileTransferStatusChanged(const QString& /*accountID*/,
                                    const QString& /*transferID*/,
                                    DRing::FileTransferStatus /*status*/,
                                    DRing::DataTransferError /*error*/);

private:
    FileTransferModel* q_ptr;

    void insertNewTransfer(const QString& accountID, const QString& transferID, DRing::FileTransferStatus status);
    void updateTransfer(const QString& transferID, DRing::FileTransferStatus status);
    void removeTransfer(const QString& transferID);
    QString dataTransferErrorToString(DRing::DataTransferError error);
};

FileTransferModelPrivate::FileTransferModelPrivate(FileTransferModel* parent): QObject(parent),q_ptr(parent),m_pSelectionModel(nullptr)
{
    auto& dataTransferManager = DataTransferManager::instance();
    connect(&dataTransferManager, &DataTransferManagerInterface::fileTransferStatusChanged,this ,
            &FileTransferModelPrivate::slotFileTransferStatusChanged);
}

void
FileTransferModelPrivate::slotFileTransferStatusChanged(const QString& accountID,
                                const QString& transferID,
                                DRing::FileTransferStatus status,
                                DRing::DataTransferError error)
{

    if (error != DRing::DataTransferError::NONE) {
        qWarning() << "File transfer error" << dataTransferErrorToString(error);
    }

    qDebug() << "slotFileTransferStatusChanged" << accountID << transferID << "status:" << (int)status << "error:" << (int)error;
    if (!m_shTransferId.contains(transferID)) {
        insertNewTransfer(accountID, transferID, status);
    } else
        updateTransfer(transferID, status);
}

QString
FileTransferModelPrivate::dataTransferErrorToString(DRing::DataTransferError error)
{
    using DRing::DataTransferError;
    switch (error) {
        case DataTransferError::NONE:
            return tr("NONE");
        case DataTransferError::SYSTEM:
            return tr("SYSTEM");
        case DataTransferError::IO:
            return tr("IO");
        case DataTransferError::REFUSED:
            return tr("REFUSED");
        case DataTransferError::LOST:
            return tr("LOST");
        default:
            return tr("UNKNOWN");
    };
}

void
FileTransferModelPrivate::insertNewTransfer(const QString& accountID, const QString& transferID, DRing::FileTransferStatus status) {
    auto& dataTransferManager = DataTransferManager::instance();
    auto toInsert = new Transfer(AccountModel::instance().getById(accountID.toLatin1()), transferID);
    toInsert->setStatus(status);
    DRing::FileTransferInfo tmp;
    dataTransferManager.fileTransferInfo(transferID, tmp);
    toInsert->setFilename(QString(tmp.filename.c_str()));
    toInsert->setSize(tmp.size);

    q_ptr->beginInsertRows(QModelIndex(), m_lTransfers.size(), m_lTransfers.size());
    m_lTransfers << toInsert;
    q_ptr->endInsertRows();
}

void
FileTransferModelPrivate::updateTransfer(const QString& transferID, DRing::FileTransferStatus status) {
    auto& dataTransferManager = DataTransferManager::instance();
    int index = m_shTransferId[transferID];
    auto toUpdate = m_lTransfers[index];
    DRing::FileTransferInfo tmp;
    dataTransferManager.fileTransferInfo(transferID, tmp);
    toUpdate->setStatus(status);

    emit q_ptr->dataChanged(q_ptr->index(index, 0), q_ptr->index(index, 0));
}

void
FileTransferModelPrivate::removeTransfer(const QString& transferID) {
    int index = m_shTransferId[transferID];
    q_ptr->beginRemoveRows(QModelIndex(),index,index);
    m_shTransferId.remove(transferID);
    m_lTransfers.remove(index);
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
      roles[ (int) Role::Status] = "status"; //State for transfer
    }
    return roles;
}

//Model functions
QVariant FileTransferModel::data( const QModelIndex& index, int role) const
{
    switch(role) {
       case (int)Ring::Role::DisplayRole:
          return d_ptr->m_lTransfers[index.row()]->filename();
    };
    return QVariant();
}

void
FileTransferModel::sendFile(const Account* account, const QString& peerUri, const QString& filename)
{
    auto& dataTransferManager = DataTransferManager::instance();
    dataTransferManager.sendFile(account->id(),peerUri,filename);
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
