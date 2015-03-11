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
#include "availableaccountmodel.h"

//Qt
#include <QtCore/QItemSelectionModel>
#include <QtCore/QCoreApplication>

//DRing
#include <account_const.h>

//Ring
#include "private/accountmodel_p.h"
#include "contactmethod.h"
#include "uri.h"

class AvailableAccountModelPrivate : public QObject
{
   Q_OBJECT
public:
   AvailableAccountModelPrivate(AvailableAccountModel* parent);

   QItemSelectionModel*          m_pSelectionModel;
   static Account*               m_spPriorAccount ;
   static AvailableAccountModel* m_spInstance     ;

   static void     setPriorAccount       ( const Account* account );
   static Account* firstRegisteredAccount( URI::SchemeType type = URI::SchemeType::NONE );

   AvailableAccountModel* q_ptr;

public Q_SLOTS:
   void checkRemovedAccount(Account* a);
   void checkStateChanges(Account* account, const Account::RegistrationState state);
   void selectionChanged(const QModelIndex& idx, const QModelIndex& previous);
};

Account*               AvailableAccountModelPrivate::m_spPriorAccount  = nullptr;
AvailableAccountModel* AvailableAccountModelPrivate::m_spInstance      = nullptr;

AvailableAccountModelPrivate::AvailableAccountModelPrivate(AvailableAccountModel* parent) :m_pSelectionModel(nullptr),q_ptr(parent)
{
   connect(AccountModel::instance(), &AccountModel::accountRemoved     , this, &AvailableAccountModelPrivate::checkRemovedAccount );
   connect(AccountModel::instance(), &AccountModel::accountStateChanged, this, &AvailableAccountModelPrivate::checkStateChanges   );
}

AvailableAccountModel::AvailableAccountModel(QObject* parent) : QSortFilterProxyModel(parent),
d_ptr(new AvailableAccountModelPrivate(this))
{
   setSourceModel(AccountModel::instance());
}

AvailableAccountModel* AvailableAccountModel::instance()
{
   if (!AvailableAccountModelPrivate::m_spInstance)
      AvailableAccountModelPrivate::m_spInstance = new AvailableAccountModel(QCoreApplication::instance());

   return AvailableAccountModelPrivate::m_spInstance;
}

//Do not show the checkbox
QVariant AvailableAccountModel::data(const QModelIndex& idx,int role ) const
{
   return (role == Qt::CheckStateRole) ? QVariant() : mapToSource(idx).data(role);
}

///Disable the unavailable accounts
Qt::ItemFlags AvailableAccountModel::flags (const QModelIndex& idx) const
{
   const QModelIndex& src = mapToSource(idx);
   if (qvariant_cast<Account::RegistrationState>(src.data(static_cast<int>(Account::Role::RegistrationState))) != Account::RegistrationState::READY)
      return Qt::NoItemFlags;
   return sourceModel()->flags(idx);
}

//Do not display disabled account
bool AvailableAccountModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
   return sourceModel()->index(source_row,0,source_parent).data(Qt::CheckStateRole) == Qt::Checked;
}


///Return the current account
Account* AvailableAccountModel::currentDefaultAccount(ContactMethod* method)
{
   Account* priorAccount = AvailableAccountModelPrivate::m_spPriorAccount;
   URI::SchemeType type = (!method) ? URI::SchemeType::NONE : method->uri().schemeType();
   if(priorAccount
     && priorAccount->registrationState() == Account::RegistrationState::READY
     && priorAccount->isEnabled()
     && (priorAccount->supportScheme(type))
   ) {
      return priorAccount;
   }
   else {
      Account* a = AvailableAccountModelPrivate::firstRegisteredAccount(type);
      if (!a)
         a = AccountModel::instance()->getById(DRing::Account::ProtocolNames::IP2IP);

      AvailableAccountModelPrivate::setPriorAccount(a);
      return a;
   }
} //getCurrentAccount

///Set the previous account used
void AvailableAccountModelPrivate::setPriorAccount(const Account* account) {
   const bool changed = (account && m_spPriorAccount != account) || (!account && m_spPriorAccount);
   m_spPriorAccount = const_cast<Account*>(account);
   if (changed) {
      AvailableAccountModel* self = AvailableAccountModel::instance();
      Account* a = self->currentDefaultAccount();

      emit self->currentDefaultAccountChanged(a);

      if (self->d_ptr->m_pSelectionModel) {
         self->d_ptr->m_pSelectionModel->setCurrentIndex(self->mapFromSource(a->index()), QItemSelectionModel::ClearAndSelect);
      }
   }
}

///Get the first registerred account (default account)
Account* AvailableAccountModelPrivate::firstRegisteredAccount(URI::SchemeType type)
{
   for (Account* current : AccountModel::instance()->d_ptr->m_lAccounts) {
      if(current
        && current->registrationState() == Account::RegistrationState::READY
        && current->isEnabled()
        && current->supportScheme(type)
      )
         return current;
   }
   return firstRegisteredAccount(); //We need one
}

QItemSelectionModel* AvailableAccountModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<AvailableAccountModel*>(this));
      connect(d_ptr->m_pSelectionModel, &QItemSelectionModel::currentChanged,d_ptr,&AvailableAccountModelPrivate::selectionChanged);
      Account* a = d_ptr->firstRegisteredAccount();
      if (a)
         d_ptr->m_pSelectionModel->setCurrentIndex(mapFromSource(a->index()), QItemSelectionModel::ClearAndSelect);
   }
   return d_ptr->m_pSelectionModel;
}

void AvailableAccountModelPrivate::selectionChanged(const QModelIndex& idx, const QModelIndex& previous)
{
   Q_UNUSED(previous)
   Account* a = qvariant_cast<Account*>(idx.data(static_cast<int>(Account::Role::Object)));

   setPriorAccount(a);
}

void AvailableAccountModelPrivate::checkRemovedAccount(Account* a)
{
   if (a == m_spPriorAccount) {
      Account* a = firstRegisteredAccount();
      qDebug() << "The current default account has been removed, now defaulting to" << a;
      setPriorAccount(a);
   }
}

void AvailableAccountModelPrivate::checkStateChanges(Account* account, const Account::RegistrationState state)
{
   Q_UNUSED(account)
   Q_UNUSED(state)
   Account* a = firstRegisteredAccount();
   if ( m_spPriorAccount != a ) {
      qDebug() << "The current default account changed to" << a;
      setPriorAccount(a);
   }
}

#include <availableaccountmodel.moc>
