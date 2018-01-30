/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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
#include "keyexchangemodel.h"

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QItemSelectionModel>

//Ring daemon
#include <account_const.h>

//Ring
#include "account.h"
#include "private/matrixutils.h"

class KeyExchangeModelPrivate final : public QObject
{
   Q_OBJECT
public:

   class Name {
   public:
      constexpr static const char* NONE = "None";
      constexpr static const char* SDES = "SDES";
   };

   class DaemonName {
   public:
      constexpr static const char* NONE = ""    ;
      constexpr static const char* SDES = "sdes";
   };

   KeyExchangeModelPrivate(KeyExchangeModel* parent);

   //Attributes
   Account* m_pAccount;
   QItemSelectionModel* m_pSelectionModel;

   //Getters
   QModelIndex                   toIndex       (KeyExchangeModel::Type type) const;
   static const char*            toDaemonName  (KeyExchangeModel::Type type)      ;
   static KeyExchangeModel::Type fromDaemonName(const QString& name        )      ;


   //Helper
   void setKeyExchange(KeyExchangeModel::Type detail);
   KeyExchangeModel::Type keyExchange() const;

public Q_SLOTS:
   void slotCurrentIndexChanged(const QModelIndex& idx, const QModelIndex& prev);

private:
   KeyExchangeModel* q_ptr;
};

KeyExchangeModelPrivate::KeyExchangeModelPrivate(KeyExchangeModel* parent) : QObject(parent), q_ptr(parent),m_pAccount(nullptr), m_pSelectionModel(nullptr)
{
}

KeyExchangeModel::KeyExchangeModel(Account* account) : QAbstractListModel(account),d_ptr(new KeyExchangeModelPrivate(this))
{
   d_ptr->m_pAccount = account;
}

KeyExchangeModel::~KeyExchangeModel()
{
//    delete d_ptr;
}

QHash<int,QByteArray> KeyExchangeModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      roles[static_cast<int>(KeyExchangeModel::Role::TYPE)] = "type";
      initRoles = true;

   }
   return roles;
}

//Model functions
QVariant KeyExchangeModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid()) return QVariant();
   KeyExchangeModel::Type method = static_cast<KeyExchangeModel::Type>(index.row());
   if (role == Qt::DisplayRole) {
      switch (method) {
         case KeyExchangeModel::Type::NONE:
            return KeyExchangeModelPrivate::Name::NONE;
         case KeyExchangeModel::Type::SDES:
            return KeyExchangeModelPrivate::Name::SDES;
         case KeyExchangeModel::Type::COUNT__:
            break;
      };
   }
   else if (role == static_cast<int>(KeyExchangeModel::Role::TYPE))
      return QVariant::fromValue(method);

   return QVariant();
}

int KeyExchangeModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()?0:2;
}

bool KeyExchangeModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

///Translate enum type to QModelIndex
QModelIndex KeyExchangeModelPrivate::toIndex(KeyExchangeModel::Type type) const
{
   return q_ptr->index(static_cast<int>(type),0,QModelIndex());
}

///Translate enum to daemon name
const char* KeyExchangeModelPrivate::toDaemonName(KeyExchangeModel::Type type)
{
   switch (type) {
      case KeyExchangeModel::Type::NONE:
         return KeyExchangeModelPrivate::DaemonName::NONE;
      case KeyExchangeModel::Type::SDES:
         return KeyExchangeModelPrivate::DaemonName::SDES;
      case KeyExchangeModel::Type::COUNT__:
         break;
   };
   return nullptr; //Cannot heppen
}

KeyExchangeModel::Type KeyExchangeModelPrivate::fromDaemonName(const QString& name)
{
   if (name.isEmpty())
      return KeyExchangeModel::Type::NONE;
   else if (name == KeyExchangeModelPrivate::DaemonName::SDES)
      return KeyExchangeModel::Type::SDES;
   qDebug() << "Undefined Key exchange mechanism" << name;
   return KeyExchangeModel::Type::NONE;
}

void KeyExchangeModel::enableSRTP(bool enable)
{
   if (enable && d_ptr->keyExchange() == KeyExchangeModel::Type::NONE) {
      d_ptr->setKeyExchange(KeyExchangeModel::Type::SDES);
   }
   else if (!enable) {
      d_ptr->setKeyExchange(KeyExchangeModel::Type::NONE);
   }
}

QItemSelectionModel* KeyExchangeModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<KeyExchangeModel*>(this));
      const KeyExchangeModel::Type current = d_ptr->keyExchange();
      d_ptr->m_pSelectionModel->setCurrentIndex(d_ptr->toIndex(current), QItemSelectionModel::ClearAndSelect);

      connect(d_ptr->m_pSelectionModel, &QItemSelectionModel::currentChanged, d_ptr.data(), &KeyExchangeModelPrivate::slotCurrentIndexChanged);
   }

   return d_ptr->m_pSelectionModel;
}

void KeyExchangeModelPrivate::slotCurrentIndexChanged(const QModelIndex& idx, const QModelIndex& prev)
{
   Q_UNUSED(prev)

   if (!idx.isValid())
      return;

   const KeyExchangeModel::Type t = static_cast<KeyExchangeModel::Type>(idx.row());
   setKeyExchange(t);
}

///Return the key exchange mechanism
KeyExchangeModel::Type KeyExchangeModelPrivate::keyExchange() const
{
   return KeyExchangeModelPrivate::fromDaemonName(m_pAccount->accountDetail(DRing::Account::ConfProperties::SRTP::KEY_EXCHANGE));
}

///Set the Tls method
void KeyExchangeModelPrivate::setKeyExchange(KeyExchangeModel::Type detail)
{
   m_pAccount->setAccountProperty(DRing::Account::ConfProperties::SRTP::KEY_EXCHANGE ,KeyExchangeModelPrivate::toDaemonName(detail));
   m_pAccount->regenSecurityValidation();
}

#include <keyexchangemodel.moc>
