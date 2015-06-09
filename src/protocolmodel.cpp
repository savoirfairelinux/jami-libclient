/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#include "protocolmodel.h"

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>

//Ring daemon
#include <account_const.h>

//Ring
#include <account.h>
#include <private/account_p.h>

class ProtocolModelPrivate final : public QObject
{
   Q_OBJECT
public:

   ProtocolModelPrivate(Account* a);

   mutable QItemSelectionModel* m_pSelectionModel;
   Account* m_pAccount;

   //Constants
   struct ToolTips {
      static const QString RING_ACCOUNT_TOOLTIP;
      static const QString SIP_ACCOUNT_TOOLTIP ;
      static const QString IAX2_ACCOUNT_TOOLTIP;
   };

public Q_SLOTS:
   void slotSelectionChanged(const QModelIndex& idx);
};

const QString ProtocolModelPrivate::ToolTips::RING_ACCOUNT_TOOLTIP = QObject::tr("Ring Account");
const QString ProtocolModelPrivate::ToolTips::SIP_ACCOUNT_TOOLTIP  = QObject::tr("SIP Account" );
const QString ProtocolModelPrivate::ToolTips::IAX2_ACCOUNT_TOOLTIP = QObject::tr("IAX2 Account");


ProtocolModelPrivate::ProtocolModelPrivate(Account* a) : m_pSelectionModel(nullptr), m_pAccount(a)
{

}

ProtocolModel::ProtocolModel(Account* a) : QAbstractListModel(QCoreApplication::instance()),
d_ptr(new ProtocolModelPrivate(a))
{

}

ProtocolModel::~ProtocolModel()
{
   delete d_ptr;
}

QHash<int,QByteArray> ProtocolModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

//Model functions
QVariant ProtocolModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   Account::Protocol proto = static_cast<Account::Protocol>(index.row());
   if (role == Qt::DisplayRole) {
      switch (proto) {
         case Account::Protocol::SIP    :
            return DRing::Account::ProtocolNames::SIP;
         case Account::Protocol::IAX    :
            return DRing::Account::ProtocolNames::IAX;
         case Account::Protocol::RING   :
            return DRing::Account::ProtocolNames::RING;
         case Account::Protocol::COUNT__:
            break;
      };
   }
   else if (role == Qt::UserRole) {
      return QVariant::fromValue(proto);
   }
   return QVariant();
}

int ProtocolModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()?0:enum_class_size<Account::Protocol>();
}

Qt::ItemFlags ProtocolModel::flags( const QModelIndex& index ) const
{
   const bool isNew = d_ptr->m_pAccount ? d_ptr->m_pAccount->isNew() : true;
   if (!index.isValid() || (!isNew)) return Qt::NoItemFlags;

   //Account type cannot be changed, the daemon doesn't support that and crash
   //it was considered a client responsibility to disallow it. It is not worth
   //fixing
   return (static_cast<Account::Protocol>(index.row()) == Account::Protocol::IAX ?
   Qt::NoItemFlags : (Qt::ItemIsEnabled|Qt::ItemIsSelectable));
}

bool ProtocolModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

QItemSelectionModel* ProtocolModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<ProtocolModel*>(this));

      const Account::Protocol proto    = d_ptr->m_pAccount ? d_ptr->m_pAccount->protocol() : Account::Protocol::RING;

      const QModelIndex& idx = index(static_cast<int>(proto),0);
      d_ptr->m_pSelectionModel->setCurrentIndex(idx,QItemSelectionModel::ClearAndSelect);

      connect(d_ptr->m_pSelectionModel,&QItemSelectionModel::currentChanged,d_ptr,&ProtocolModelPrivate::slotSelectionChanged);
   }

   return d_ptr->m_pSelectionModel;
}

void ProtocolModelPrivate::slotSelectionChanged(const QModelIndex& idx)
{
   if (!m_pAccount || !idx.isValid())
      return;

   m_pAccount->setProtocol(static_cast<Account::Protocol>(idx.row()));
}

#include <protocolmodel.moc>
