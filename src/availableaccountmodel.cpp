/****************************************************************************
 *   Copyright (C) 2012-2018 Savoir-faire Linux                          *
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
#include "contactmethod.h"
#include "uri.h"

class AvailableAccountModelPrivate final : public QObject
{
   Q_OBJECT
public:
   AvailableAccountModelPrivate(AvailableAccountModel* parent);

   QItemSelectionModel* m_pSelectionModel;
   static Account*      m_spPriorAccount ;

   static void     setPriorAccount       ( const Account* account );
   static Account* firstRegisteredAccount( URI::SchemeType type = URI::SchemeType::NONE );

   AvailableAccountModel* q_ptr;

public Q_SLOTS:
   void checkRemovedAccount(Account* a);
   void checkStateChanges(Account* account, const Account::RegistrationState state);
   void selectionChanged(const QModelIndex& idx, const QModelIndex& previous);
};

Account* AvailableAccountModelPrivate::m_spPriorAccount = nullptr;

AvailableAccountModelPrivate::AvailableAccountModelPrivate(AvailableAccountModel* parent) :m_pSelectionModel(nullptr),q_ptr(parent)
{
    connect(&AccountModel::instance(), &AccountModel::accountRemoved     , this, &AvailableAccountModelPrivate::checkRemovedAccount );
    connect(&AccountModel::instance(), &AccountModel::accountStateChanged, this, &AvailableAccountModelPrivate::checkStateChanges   );
}

AvailableAccountModel::AvailableAccountModel(QObject* parent) : QSortFilterProxyModel(parent),
d_ptr(new AvailableAccountModelPrivate(this))
{
   setSourceModel(&AccountModel::instance());
}

AvailableAccountModel::~AvailableAccountModel()
{
   delete d_ptr;
}

AvailableAccountModel& AvailableAccountModel::instance()
{
    static auto instance = new AvailableAccountModel(QCoreApplication::instance());
    return *instance;
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
    // if no CM is give, we use the user chosen account, since no other parameters are available
    const auto idx = AvailableAccountModel::instance().selectionModel()->currentIndex();
    if (!method && idx.isValid()) {
        return idx.data(static_cast<int>(Account::Role::Object)).value<Account*>();
    }

    // Start by validating the scheme used by the ContactMethod
    URI::SchemeType type = (!method) ? URI::SchemeType::NONE : method->uri().schemeType();

    // If the scheme type could not be strictly determined, try using the protocol hint
    if (type == URI::SchemeType::NONE && method) {
       switch (method->protocolHint()) {
          case URI::ProtocolHint::SIP_OTHER:
          case URI::ProtocolHint::SIP_HOST:
          case URI::ProtocolHint::IP:
             type = URI::SchemeType::SIP;
             break;
          case URI::ProtocolHint::RING:
          case URI::ProtocolHint::RING_USERNAME:
             type = URI::SchemeType::RING;
             break;
        }
    }

    return currentDefaultAccount(type);

} //currentDefaultAccount

/// Validation method to check if the account is in a good state and support the scheme provided
bool AvailableAccountModel::validAccountForScheme(Account* account, URI::SchemeType scheme)
{
    return (account
      && account->registrationState() == Account::RegistrationState::READY
      && account->isEnabled()
      && (account->supportScheme(scheme)));
}

Account* AvailableAccountModel::currentDefaultAccount(URI::SchemeType schemeType)
{
    // Always try to respect user choice
    const auto idx = AvailableAccountModel::instance().selectionModel()->currentIndex();
    auto userChosenAccount = idx.data(static_cast<int>(Account::Role::Object)).value<Account*>();
    if (userChosenAccount && validAccountForScheme(userChosenAccount, schemeType)) {
        return userChosenAccount;
    }

    // If the current selected choice is not valid, try the previous account selected
    auto priorAccount = AvailableAccountModelPrivate::m_spPriorAccount;

    //we prefer not to use Ip2Ip if possible
    if (priorAccount && priorAccount->isIp2ip()) {
        priorAccount = nullptr;
    }

    if(validAccountForScheme(priorAccount, schemeType)) {
        return priorAccount;
    } else {
        auto account = AvailableAccountModelPrivate::firstRegisteredAccount(schemeType);

        // If there is only RING account, it will still be nullptr. Given there is
        // *only* RING accounts, then the user probably want a call using the
        // Ring protocol. This will happen when using the name directory instead
        // of the hash
        if (!account)
            account = AvailableAccountModelPrivate::firstRegisteredAccount(
                URI::SchemeType::RING
            );

        AvailableAccountModelPrivate::setPriorAccount(account);
        return account;
    }
}

///Set the previous account used
void AvailableAccountModelPrivate::setPriorAccount(const Account* account)
{
   const bool changed = (account && m_spPriorAccount != account) || (!account && m_spPriorAccount);
   m_spPriorAccount = const_cast<Account*>(account);
   if (changed) {
      auto& self = AvailableAccountModel::instance();

      Account* a = account ? const_cast<Account*>(account) : self.currentDefaultAccount();

      emit self.currentDefaultAccountChanged(a);

      if (self.d_ptr->m_pSelectionModel) {

         const QModelIndex idx = self.mapFromSource(a->index());

         if (idx.isValid())
            self.d_ptr->m_pSelectionModel->setCurrentIndex(self.mapFromSource(a->index()), QItemSelectionModel::ClearAndSelect);
         else
            self.d_ptr->m_pSelectionModel->clearSelection();
      }
   }
}

///Get the first registerred account (default account)
Account* AvailableAccountModelPrivate::firstRegisteredAccount(URI::SchemeType type)
{
    return AccountModel::instance().findAccountIf([&type](const Account& account) {
        return account.registrationState() == Account::RegistrationState::READY
            && account.isEnabled()
            && account.supportScheme(type);
    });
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
      Account* a2 = firstRegisteredAccount();
      qDebug() << "The current default account has been removed, now defaulting to" << a2;
      setPriorAccount(a2);
   }
}

void AvailableAccountModelPrivate::checkStateChanges(Account* account, const Account::RegistrationState state)
{
    // change PriorAccount if current PriorAccount became unavailable
    if(m_spPriorAccount != account || state == Account::RegistrationState::READY ||
       state == Account::RegistrationState::TRYING)
        return;
    Account* a = firstRegisteredAccount();
    setPriorAccount(a);
}

#include <availableaccountmodel.moc>
