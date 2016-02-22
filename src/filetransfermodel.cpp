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
#include <private/account_p.h>

class TransferItemNode
{
public:

   TransferItemNode(Account* a, QString name);
   TransferItemNode(){};
   Account* m_pAccount;
   QString m_Filename;

};

class FileTransferModelPrivate final : public QObject {
    Q_OBJECT
public:
    FileTransferModelPrivate(FileTransferModel*);
    QVector<TransferItemNode> m_lTransfers;
    QItemSelectionModel* m_pSelectionModel;

public Q_SLOTS:
    void slotDataConnectionStatusChanged(const QString& /*accountID*/,
                                    const QString& /*connectionID*/,
                                    DRing::DataConnectionStatus /*status*/,
                                    DRing::DataTransferError /*error*/);
    void slotFileTransferStatusChanged(const QString& /*accountID*/,
                                    const QString& /*connectionID*/,
                                    DRing::FileTransferStatus /*status*/,
                                    DRing::DataTransferError /*error*/);

private:
    FileTransferModel* q_ptr;
};

TransferItemNode::TransferItemNode(Account* a, QString name) : m_pAccount(a), m_Filename(name)
{

}

FileTransferModelPrivate::FileTransferModelPrivate(FileTransferModel* parent): QObject(parent),q_ptr(parent),m_pSelectionModel(nullptr)
{
    m_lTransfers << TransferItemNode(nullptr, "test");
    m_lTransfers << TransferItemNode(nullptr, "OK");
    m_lTransfers << TransferItemNode(nullptr, "cool.png");
    auto& dataTransferManager = DataTransferManager::instance();
    connect(&dataTransferManager, &DataTransferManagerInterface::dataConnectionStatusChanged,this ,
            &FileTransferModelPrivate::slotDataConnectionStatusChanged);
    connect(&dataTransferManager, &DataTransferManagerInterface::fileTransferStatusChanged,this ,
            &FileTransferModelPrivate::slotFileTransferStatusChanged);
}

void
FileTransferModelPrivate::slotDataConnectionStatusChanged(const QString& accountID,
                                const QString& connectionID,
                                DRing::DataConnectionStatus status,
                                DRing::DataTransferError error)
{
    qDebug() << "slotDataConnectionStatusChanged" << accountID << connectionID << "status:" << (int)status << "error:" << (int)error;
}

void
FileTransferModelPrivate::slotFileTransferStatusChanged(const QString& accountID,
                                const QString& connectionID,
                                DRing::FileTransferStatus status,
                                DRing::DataTransferError error)
{
    qDebug() << "slotFileTransferStatusChanged" << accountID << connectionID << "status:" << (int)status << "error:" << (int)error;
}

FileTransferModel::FileTransferModel(QObject* parent) : QAbstractListModel(parent ? parent : QCoreApplication::instance()),
d_ptr(new FileTransferModelPrivate(this))
{
    setObjectName("FileTransferModel");
}

FileTransferModel::~FileTransferModel()
{
    while (d_ptr->m_lTransfers.size()) {
       d_ptr->m_lTransfers.remove(0);
   }
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
          return d_ptr->m_lTransfers[index.row()].m_Filename;
    };
    return QVariant();
}

QString
FileTransferModel::sendFile(const Account* account, const QString& peerUri, const QString& filename)
{
    auto& dataTransferManager = DataTransferManager::instance();
    dataTransferManager.sendFile(account->id(),peerUri,filename);
    return "";
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
