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
#include "bannedcontactmodel.h"

//Qt
#include <QtCore/QDateTime>

// LRC
#include "dbus/configurationmanager.h"
#include "contactmethod.h"
#include "phonedirectorymodel.h"

#include <contactrequest.h>
#include <certificate.h>
#include <account.h>
#include "private/pendingcontactrequestmodel_p.h"
#include "person.h"
#include "contactmethod.h"

class BannedContactModelPrivate
{
public:
    //Constructor
    BannedContactModelPrivate(BannedContactModel* parent);

    //Attributes
    QList<ContactMethod*> m_lBanned;
    Account*              m_pAccount ;

private:
    BannedContactModel* q_ptr;
};

/**
 * constructor of BannedContactModelPrivate.
 */
BannedContactModelPrivate::BannedContactModelPrivate(BannedContactModel* p) : q_ptr(p)
{
}

/**
 * constructor of BannedContactModel.
 */
BannedContactModel::BannedContactModel(Account* a) : QAbstractTableModel(a),
d_ptr(new BannedContactModelPrivate(this))
{
    d_ptr->m_pAccount = a;

    // Load the contacts associated from the daemon and create the cms.
    const auto account_contacts
        = static_cast<QVector<QMap<QString, QString>>>(ConfigurationManager::instance().getContacts(a->id().data()));

    if (a->protocol() == Account::Protocol::RING) {
        for (auto contact_info : account_contacts) {
            if (contact_info["banned"] == "true") {
                auto cm = PhoneDirectoryModel::instance().getNumber(contact_info["id"], a);
                add(cm);
            }
        }
    }
}

/**
 * destructor of BannedContactModel.
 */
BannedContactModel::~BannedContactModel()
{
    delete d_ptr;
}

/**
 * QAbstractTableModel function used to return the data.
 */
QVariant
BannedContactModel::data( const QModelIndex& index, int role ) const
{
    if (!index.isValid())
        return QVariant();

    switch(index.column()) {
    case Columns::PEER_ID:
        switch(role) {
        case Qt::DisplayRole:
            return d_ptr->m_lBanned[index.row()]->bestId();
        case static_cast<int>(ContactMethod::Role::Object):
            return QVariant::fromValue(d_ptr->m_lBanned[index.row()]);
        }
    break;
    case Columns::COUNT__:
        switch(role) {
        case Qt::DisplayRole:
            return static_cast<int>(BannedContactModel::Columns::COUNT__);
        }
    break;
    }

   return QVariant();
}

/**
 * return the number of rows from the model.
 */
int
BannedContactModel::rowCount( const QModelIndex& parent ) const
{
    return parent.isValid()? 0 : d_ptr->m_lBanned.size();
}

/**
 * return the number of columns from the model.
 */
int
BannedContactModel::columnCount( const QModelIndex& parent ) const
{
    return parent.isValid()? 0 : static_cast<int>(BannedContactModel::Columns::COUNT__);
}

/**
 * this function add a ContactMethod to the banned list.
 * @param cm, the ContactMethod to add to the list.
 */
void
BannedContactModel::add(ContactMethod* cm)
{
    if (d_ptr->m_lBanned.contains(cm))
        return;
    beginInsertRows(QModelIndex(),d_ptr->m_lBanned.size(),d_ptr->m_lBanned.size());
    d_ptr->m_lBanned << cm;
    endInsertRows();
}

/**
 * this function removes a ContactMethod from the banned list.
 * @param cm, the ContactMethod to remove from the list.
 */
void
BannedContactModel::remove(ContactMethod* cm)
{
    auto rowIndex = d_ptr->m_lBanned.indexOf(cm);
    if (rowIndex < 0)
        return;

    beginRemoveRows(QModelIndex(), rowIndex, rowIndex);
    d_ptr->m_lBanned.removeAt(rowIndex);
    endRemoveRows();

    if (!cm->account()) {
        qWarning() << "BannedContactModel, cannot remove. cm->account is nullptr";
        return;
    }

    ConfigurationManager::instance().addContact(cm->account()->id(), cm->uri());
}
