/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#include "tlsmethodmodel.h"

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>

//Ring daemon
#include <account_const.h>

//Ring
#include <account.h>
#include <private/account_p.h>

class TlsMethodModelPrivate : public QObject {
   Q_OBJECT
public:

   TlsMethodModelPrivate(Account* a);

   class Name {
   public:
      static const QString         DEFAULT            ;
      constexpr static const char* TLSv1   = "TLSv1"  ;
      constexpr static const char* SSLv3   = "SSLv3"  ;
      constexpr static const char* SSLv23  = "SSLv23" ;
   };

   class DaemonName {
   public:
      constexpr static const char* DEFAULT = "Default";
      constexpr static const char* TLSv1   = "TLSv1"  ;
      constexpr static const char* SSLv3   = "SSLv3"  ;
      constexpr static const char* SSLv23  = "SSLv23" ;
   };

   static const char* toDaemonName(TlsMethodModel::Type type);
   static TlsMethodModel::Type fromDaemonName(const QString& name);
   bool isRing;

   mutable QItemSelectionModel* m_pSelectionModel;
   Account* m_pAccount;

public Q_SLOTS:
   void slotSelectionChanged(const QModelIndex& idx);
};

const QString TlsMethodModelPrivate::Name::DEFAULT = QObject::tr("Default", "Default TLS protocol version");

TlsMethodModelPrivate::TlsMethodModelPrivate(Account* a) : m_pSelectionModel(nullptr), m_pAccount(a), isRing(false)
{
   isRing = a->protocol() == Account::Protocol::RING;
}

TlsMethodModel::TlsMethodModel(Account* a) : QAbstractListModel(QCoreApplication::instance()),
d_ptr(new TlsMethodModelPrivate(a))
{

}

QHash<int,QByteArray> TlsMethodModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

//Model functions
QVariant TlsMethodModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   TlsMethodModel::Type method = static_cast<TlsMethodModel::Type>(index.row());
   if (role == Qt::DisplayRole) {

      if (d_ptr->isRing)
         return tr("Automatic");

      switch (method) {
         case TlsMethodModel::Type::DEFAULT:
            return TlsMethodModelPrivate::Name::DEFAULT;
         case TlsMethodModel::Type::TLSv1:
            return TlsMethodModelPrivate::Name::TLSv1;
         case TlsMethodModel::Type::SSLv3:
            return TlsMethodModelPrivate::Name::SSLv3;
         case TlsMethodModel::Type::SSLv23:
            return TlsMethodModelPrivate::Name::SSLv23;
      };
   }
   return QVariant();
}

int TlsMethodModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()?0: d_ptr->isRing ? 1 : 4;
}

Qt::ItemFlags TlsMethodModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid()) return Qt::NoItemFlags;
   return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

bool TlsMethodModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

///Translate enum type to QModelIndex
QModelIndex TlsMethodModel::toIndex(TlsMethodModel::Type type) const
{
   return index(static_cast<int>(type),0,QModelIndex());
}

QItemSelectionModel* TlsMethodModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<TlsMethodModel*>(this));
      const QString value    = d_ptr->m_pAccount->d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::METHOD);
      const QModelIndex& idx = toIndex(TlsMethodModelPrivate::fromDaemonName(value));
      d_ptr->m_pSelectionModel->setCurrentIndex(idx,QItemSelectionModel::ClearAndSelect);

      if (!d_ptr->isRing)
         connect(d_ptr->m_pSelectionModel,&QItemSelectionModel::currentChanged,d_ptr,&TlsMethodModelPrivate::slotSelectionChanged);
   }

   return d_ptr->m_pSelectionModel;
}

void TlsMethodModelPrivate::slotSelectionChanged(const QModelIndex& idx)
{
   if (!idx.isValid())
      return;

   const char* value = toDaemonName(static_cast<TlsMethodModel::Type>(idx.row()));
   if (value != m_pAccount->d_ptr->accountDetail(DRing::Account::ConfProperties::TLS::METHOD))
      m_pAccount->d_ptr->setAccountProperty(DRing::Account::ConfProperties::TLS::METHOD , value);
}

///Convert a TlsMethodModel::Type enum to the string expected by the daemon API
const char* TlsMethodModelPrivate::toDaemonName(TlsMethodModel::Type type)
{
   switch (type) {
      case TlsMethodModel::Type::DEFAULT:
         return TlsMethodModelPrivate::DaemonName::DEFAULT;
      case TlsMethodModel::Type::TLSv1:
         return TlsMethodModelPrivate::DaemonName::TLSv1;
      case TlsMethodModel::Type::SSLv3:
         return TlsMethodModelPrivate::DaemonName::SSLv3;
      case TlsMethodModel::Type::SSLv23:
         return TlsMethodModelPrivate::DaemonName::SSLv23;
   };
   return TlsMethodModelPrivate::DaemonName::DEFAULT;
}

///Convert a Daemon API string to a TlsMethodModel::Type enum
TlsMethodModel::Type TlsMethodModelPrivate::fromDaemonName(const QString& name)
{
   if (name.isEmpty() || name == TlsMethodModelPrivate::DaemonName::DEFAULT)
      return TlsMethodModel::Type::DEFAULT;
   else if (name == TlsMethodModelPrivate::DaemonName::TLSv1)
      return TlsMethodModel::Type::TLSv1;
   else if (name == TlsMethodModelPrivate::DaemonName::SSLv3)
      return TlsMethodModel::Type::SSLv3;
   else if (name == TlsMethodModelPrivate::DaemonName::SSLv23)
      return TlsMethodModel::Type::SSLv23;
   qDebug() << "Unknown TLS method" << name;
   return TlsMethodModel::Type::DEFAULT;
}

#include <tlsmethodmodel.moc>
