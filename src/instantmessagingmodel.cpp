/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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
#include "instantmessagingmodel.h"

#include <callmodel.h>
#include "dbus/callmanager.h"
#include <call.h>
#include <media/textrecording.h>
#include <person.h>
#include <contactmethod.h>
#include "private/instantmessagingmodel_p.h"
#include "private/textrecording_p.h"

InstantMessagingModelPrivate::InstantMessagingModelPrivate(InstantMessagingModel* parent) : QObject(parent), q_ptr(parent),
m_pRecording(nullptr)
{

}

///Constructor
InstantMessagingModel::InstantMessagingModel(Media::TextRecording* recording) : QAbstractListModel(recording), d_ptr(new InstantMessagingModelPrivate(this))
{
   d_ptr->m_pRecording = recording;
}

InstantMessagingModel::~InstantMessagingModel()
{
//    delete d_ptr;
}

QHash<int,QByteArray> InstantMessagingModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert(InstantMessagingModel::Role::TYPE    ,QByteArray( "type"    ));
      roles.insert(InstantMessagingModel::Role::FROM    ,QByteArray( "from"    ));
      roles.insert(InstantMessagingModel::Role::TEXT    ,QByteArray( "text"    ));
      roles.insert(InstantMessagingModel::Role::IMAGE   ,QByteArray( "image"   ));
      roles.insert(InstantMessagingModel::Role::CONTACT ,QByteArray( "contact" ));
   }
   return roles;
}

///Get data from the model
QVariant InstantMessagingModel::data( const QModelIndex& idx, int role) const
{
   if (idx.column() == 0) {
      switch (role) {
         case Qt::DisplayRole:
            return QVariant(d_ptr->m_pRecording->d_ptr->m_lNodes[idx.row()]->m_pMessage->payload);
         case InstantMessagingModel::Role::TYPE:
            return QVariant(d_ptr->m_pRecording->d_ptr->m_lNodes[idx.row()]->m_pMessage->payload);
         case InstantMessagingModel::Role::FROM:
            return QVariant();//d_ptr->m_pRecording->d_ptr->m_lNodes[idx.row()]->from);
         case InstantMessagingModel::Role::TEXT:
            return static_cast<int>(MessageRole::INCOMMING_IM);
         case InstantMessagingModel::Role::CONTACT:
            return QVariant();
         case InstantMessagingModel::Role::IMAGE: {
            return QVariant();
         }
         default:
            break;
      }
   }
   return QVariant();
}

///Number of row
int InstantMessagingModel::rowCount(const QModelIndex& parentIdx) const
{
   Q_UNUSED(parentIdx)
   return d_ptr->m_pRecording->d_ptr->m_lNodes.size();
}

///Model flags
Qt::ItemFlags InstantMessagingModel::flags(const QModelIndex& idx) const
{
   Q_UNUSED(idx)
   return Qt::ItemIsEnabled;
}

///Set model data
bool InstantMessagingModel::setData(const QModelIndex& idx, const QVariant &value, int role)
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

void InstantMessagingModelPrivate::addRowBegin()
{
   const int rc = q_ptr->rowCount();
   q_ptr->beginInsertRows(QModelIndex(),rc,rc);
}

void InstantMessagingModelPrivate::addRowEnd()
{
   q_ptr->endInsertRows();
}
