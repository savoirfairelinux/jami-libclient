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
#pragma once

#include "typedefs.h"
#include "itemdataroles.h"
#include <QtCore/QAbstractListModel>

//Qt
class QItemSelectionModel;
class Account;
class Transfer;

//Ring
class FileTransferModelPrivate;

class LIB_EXPORT FileTransferModel : public QAbstractListModel {
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
    Q_OBJECT
    #pragma GCC diagnostic pop

public:

    enum Role {
        Status  = static_cast<int>(Ring::Role::UserRole) + 100,
    };

    //Model functions
    virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
    virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
    virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
    virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;
    virtual QHash<int,QByteArray> roleNames() const override;

    //Singleton
    static FileTransferModel& instance();

    //Getter
    QItemSelectionModel* selectionModel() const;

    void sendFile(const Account* account, const QString& peerUri, const QString& filepath);
    void cancelTransferByModelIndex(const QModelIndex& item) const;
    Transfer* getTransferByModelIndex(const QModelIndex& item) const;

private:

    explicit FileTransferModel(QObject* parent = nullptr);
    virtual ~FileTransferModel();

    FileTransferModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(FileTransferModel)

};
Q_DECLARE_METATYPE(FileTransferModel*)
