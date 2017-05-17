/****************************************************************************
 *   Copyright (C) 2017 by Savoir-faire Linux                               *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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

#include <QtCore/QAbstractTableModel>
#include <typedefs.h>

class Account;
class BannedContactModelPrivate;
class ContactMethod;
class ContactRequest;

class LIB_EXPORT BannedContactModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    BannedContactModel(Account* a);

    enum Columns {
        PEER_ID,
        COUNT__
    };

    // Model functions
    virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
    virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
    virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;

    // Helper
    void add(ContactMethod* cm);

private:
    virtual ~BannedContactModel();

    BannedContactModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(BannedContactModel)

};
