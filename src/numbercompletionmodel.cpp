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
#include "numbercompletionmodel.h"

//Qt
#include <QtCore/QCoreApplication>

//System
#include <cmath>

//DRing
#include <account_const.h>

//Ring
#include "phonedirectorymodel.h"
#include "contactmethod.h"
#include "call.h"
#include "uri.h"
#include "numbercategory.h"
#include "accountmodel.h"
#include "availableaccountmodel.h"
#include "numbercategorymodel.h"
#include "delegates/pixmapmanipulationdelegate.h"
#include "person.h"

//Private
#include "private/phonedirectorymodel_p.h"

class NumberCompletionModelPrivate : public QObject
{
   Q_OBJECT
public:

   enum class Columns {
      CONTENT = 0,
      NAME    = 1,
      ACCOUNT = 2,
      WEIGHT  = 3,
   };

   //Constructor
   NumberCompletionModelPrivate(NumberCompletionModel* parent);

   //Methods
   void updateModel();

   //Helper
   void locateNameRange  (const QString& prefix, QSet<ContactMethod*>& set);
   void locateNumberRange(const QString& prefix, QSet<ContactMethod*>& set);
   uint getWeight(ContactMethod* number);
   uint getWeight(Account* account);
   void getRange(QMap<QString,NumberWrapper*> map, const QString& prefix, QSet<ContactMethod*>& set) const;

   //Attributes
   QMultiMap<int,ContactMethod*> m_hNumbers              ;
   URI                           m_Prefix                ;
   Call*                         m_pCall                 ;
   bool                          m_Enabled               ;
   bool                          m_UseUnregisteredAccount;
   bool                          m_DisplayMostUsedNumbers;

   QHash<Account*,TemporaryContactMethod*> m_hSipIaxTemporaryNumbers;
   QHash<Account*,TemporaryContactMethod*> m_hRingTemporaryNumbers;

public Q_SLOTS:
   void setPrefix(const QString& str);

   bool accountAdded  (Account* a);
   void accountRemoved(Account* a);

private:
   NumberCompletionModel* q_ptr;
};


NumberCompletionModelPrivate::NumberCompletionModelPrivate(NumberCompletionModel* parent) : QObject(parent), q_ptr(parent),
m_pCall(nullptr),m_Enabled(false),m_UseUnregisteredAccount(true), m_Prefix(QString()),m_DisplayMostUsedNumbers(false)
{
   //Create the temporary number list
   bool     hasNonIp2Ip = false;
   Account* ip2ip       = AccountModel::instance()->ip2ip();

   for (int i =0; i < AccountModel::instance()->size();i++) {
      Account* a = (*AccountModel::instance())[i];
      if (a != ip2ip) {
         hasNonIp2Ip |= accountAdded(a);
      }
   }

   //If SIP accounts are present, IP2IP is not needed
   if (!hasNonIp2Ip) {
      TemporaryContactMethod* cm = new TemporaryContactMethod();
      cm->setAccount(ip2ip);
      m_hSipIaxTemporaryNumbers[ip2ip] = cm;
   }

   connect(AccountModel::instance(), &AccountModel::accountAdded  , this, &NumberCompletionModelPrivate::accountAdded  );
   connect(AccountModel::instance(), &AccountModel::accountRemoved, this, &NumberCompletionModelPrivate::accountRemoved);
}

NumberCompletionModel::NumberCompletionModel() : QAbstractTableModel(PhoneDirectoryModel::instance()), d_ptr(new NumberCompletionModelPrivate(this))
{
   setObjectName("NumberCompletionModel");
}

NumberCompletionModel::~NumberCompletionModel()
{
   QList<TemporaryContactMethod*> l = d_ptr->m_hSipIaxTemporaryNumbers.values();

   d_ptr->m_hSipIaxTemporaryNumbers.clear();

   while(l.size()) {
      TemporaryContactMethod* cm = l.takeAt(0);
      delete cm;
   }

   l = d_ptr->m_hRingTemporaryNumbers.values();
   d_ptr->m_hRingTemporaryNumbers.clear();

   while(l.size()) {
      TemporaryContactMethod* cm = l.takeAt(0);
      delete cm;
   }

   delete d_ptr;
}

QHash<int,QByteArray> NumberCompletionModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;

   if (!initRoles) {
      initRoles = true;
      roles[Role::ALTERNATE_ACCOUNT]= "AlternateAccount";
      roles[Role::FORCE_ACCOUNT    ]= "ForceAccount"    ;
      roles[Role::ACCOUNT          ]= "Account"         ;
   }

   return roles;
}

QVariant NumberCompletionModel::data(const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   const QMap<int,ContactMethod*>::iterator i = d_ptr->m_hNumbers.end()-1-index.row();
   const ContactMethod* n = i.value();
   const int weight     = i.key  ();

   bool needAcc = (role>=100 || role == Qt::UserRole) && n->account() /*&& n->account() != AvailableAccountModel::currentDefaultAccount()*/
                  && n->account()->alias() != DRing::Account::ProtocolNames::IP2IP;

   switch (static_cast<NumberCompletionModelPrivate::Columns>(index.column())) {
      case NumberCompletionModelPrivate::Columns::CONTENT:
         switch (role) {
            case Qt::DisplayRole:
               return n->uri();
               break;
            case Qt::ToolTipRole:
               return QString("<table><tr><td>%1</td></tr><tr><td>%2</td></tr></table>")
                  .arg(n->primaryName())
                  .arg(n->category() ? n->category()->name() : QString());
               break;
            case Qt::DecorationRole:
               return n->icon();
               break;
            case NumberCompletionModel::Role::ALTERNATE_ACCOUNT:
            case Qt::UserRole:
               if (needAcc)
                  return n->account()->alias();
               else
                  return QString();
            case NumberCompletionModel::Role::FORCE_ACCOUNT:
               return needAcc;
            case NumberCompletionModel::Role::PEER_NAME:
               return n->primaryName();
            case NumberCompletionModel::Role::ACCOUNT:
               if (needAcc)
                  return QVariant::fromValue(n->account());
               break;
         };
         break;
      case NumberCompletionModelPrivate::Columns::NAME:
         switch (role) {
            case Qt::DisplayRole:
               return n->primaryName();
            case Qt::DecorationRole:
               if (n->contact())
                  return n->contact()->photo();
         };
         break;
      case NumberCompletionModelPrivate::Columns::ACCOUNT:
         switch (role) {
            case Qt::DisplayRole:
               return n->account()?n->account()->alias():AvailableAccountModel::currentDefaultAccount()->alias();
         };
         break;
      case NumberCompletionModelPrivate::Columns::WEIGHT:
         switch (role) {
            case Qt::DisplayRole:
               return weight;
         };
         break;
   };

   return QVariant();
}

int NumberCompletionModel::rowCount(const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;

   return d_ptr->m_hNumbers.size();
}

int NumberCompletionModel::columnCount(const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;

   return 4;
}

Qt::ItemFlags NumberCompletionModel::flags(const QModelIndex& index ) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

QVariant NumberCompletionModel::headerData (int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(orientation)
   static const QString headers[] = {tr("URI"), tr("Name"), tr("Account"), tr("Weight")};

   if (role == Qt::DisplayRole)
      return headers[section];

   return QVariant();
}

bool NumberCompletionModel::setData(const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED( index )
   Q_UNUSED( value )
   Q_UNUSED( role  )
   return false;
}

//Set the current call
void NumberCompletionModel::setCall(Call* call)
{
   if (d_ptr->m_pCall)
      disconnect(d_ptr->m_pCall,SIGNAL(dialNumberChanged(QString)),d_ptr,SLOT(setPrefix(QString)));

   d_ptr->m_pCall = call;

   if (d_ptr->m_pCall)
      connect(d_ptr->m_pCall,SIGNAL(dialNumberChanged(QString)),d_ptr,SLOT(setPrefix(QString)));

   d_ptr->setPrefix(call?call->dialNumber():QString());
}

void NumberCompletionModelPrivate::setPrefix(const QString& str)
{
   m_Prefix = str;
   const bool e = ((m_pCall && m_pCall->lifeCycleState() == Call::LifeCycleState::CREATION) || (!m_pCall)) && ((m_DisplayMostUsedNumbers || !str.isEmpty()));

   if (m_Enabled != e) {
      m_Enabled = e;
      emit q_ptr->enabled(e);
   }

   if (m_Enabled)
      updateModel();
   else {
      q_ptr->beginRemoveRows(QModelIndex(), 0, m_hNumbers.size()-1);
      m_hNumbers.clear();
      q_ptr->endRemoveRows();
   }

   for(auto cm : m_hSipIaxTemporaryNumbers) {
      if (cm)
         cm->setUri(m_Prefix);
   }

   if (m_Prefix.protocolHint() == URI::ProtocolHint::RING) {
      for(TemporaryContactMethod* cm : m_hRingTemporaryNumbers) {
         cm->setUri(m_Prefix);
      }
   }
}

Call* NumberCompletionModel::call() const
{
   return d_ptr->m_pCall;
}

ContactMethod* NumberCompletionModel::number(const QModelIndex& idx) const
{
   if (idx.isValid()) {
      //Keep the temporary contact methods private, export a copy
      ContactMethod* m = (d_ptr->m_hNumbers.end()-1-idx.row()).value();
      return m->type() == ContactMethod::Type::TEMPORARY ? 
         PhoneDirectoryModel::instance()->fromTemporary(qobject_cast<TemporaryContactMethod*>(m))
         : m;
   }

   return nullptr;
}

void NumberCompletionModelPrivate::updateModel()
{
   QSet<ContactMethod*> numbers;
   q_ptr->beginRemoveRows(QModelIndex(), 0, m_hNumbers.size()-1);
   m_hNumbers.clear();
   q_ptr->endRemoveRows();

   if (!m_Prefix.isEmpty()) {
      locateNameRange  ( m_Prefix, numbers );
      locateNumberRange( m_Prefix, numbers );

      for (auto cm : m_hSipIaxTemporaryNumbers) {
         if (!cm) continue;
         const int weight = getWeight(cm->account());
         if (weight) {
            q_ptr->beginInsertRows(QModelIndex(), m_hNumbers.size(), m_hNumbers.size());
            m_hNumbers.insert(weight,cm);
            q_ptr->endInsertRows();
         }
      }

      if (m_Prefix.protocolHint() == URI::ProtocolHint::RING) {
         for (TemporaryContactMethod* cm : m_hRingTemporaryNumbers) {
            const int weight = getWeight(cm->account());
            if (weight) {
               q_ptr->beginInsertRows(QModelIndex(), m_hNumbers.size(), m_hNumbers.size());
               m_hNumbers.insert(weight,cm);
               q_ptr->endInsertRows();
            }
         }
      }

      for (ContactMethod* n : numbers) {
         if (m_UseUnregisteredAccount || ((n->account() && n->account()->registrationState() == Account::RegistrationState::READY)
          || !n->account())) {
            q_ptr->beginInsertRows(QModelIndex(), m_hNumbers.size(), m_hNumbers.size());
            m_hNumbers.insert(getWeight(n),n);
            q_ptr->endInsertRows();
         }
      }
   }
   else if (m_DisplayMostUsedNumbers) {
      //If enabled, display the most probable entries
      const QVector<ContactMethod*> cl = PhoneDirectoryModel::instance()->getNumbersByPopularity();

      for (int i=0;i<((cl.size()>=10)?10:cl.size());i++) {
         ContactMethod* n = cl[i];
         q_ptr->beginInsertRows(QModelIndex(), m_hNumbers.size(), m_hNumbers.size());
         m_hNumbers.insert(getWeight(n),n);
         q_ptr->endInsertRows();
      }
   }
}

void NumberCompletionModelPrivate::getRange(QMap<QString,NumberWrapper*> map, const QString& prefix, QSet<ContactMethod*>& set) const
{
   if (prefix.isEmpty() || map.isEmpty())
      return;

   QMap<QString,NumberWrapper*>::iterator iBeg = map.begin();
   QMap<QString,NumberWrapper*>::iterator iEnd = map.end  ()-1;

   const QString pref = prefix.toLower();

   const int prefixLen = pref.size();
   int size = map.size()/2;
   bool startOk(false),endOk(false);

   while (size > 1 && !(startOk&&endOk)) {
      QMap<QString,NumberWrapper*>::iterator mid;

      if (size > 7)
         mid = (iBeg+size);
      else {
         //We have to be careful with "::ceil" it may cause an overflow in some rare case
         int toAdd = size-1;
         mid = iBeg;

         while (toAdd && mid != map.end()) {
            ++mid;
            --toAdd;
         }

      }

      if (mid != map.end() && mid.key().left(prefixLen) == pref && iBeg.key().left(prefixLen) < pref) {
         //Too far, need to go back
         iBeg = mid;

         while ((iBeg-1).key().left(prefixLen) == pref && iBeg != map.begin())
            iBeg--;

         startOk = true;
      }
      else if ((!startOk) && mid != map.end() && mid.key().left(prefixLen) < pref) {
         iBeg = mid;
      }
      else if(!endOk) {
         iEnd = mid;
      }

      while ((iEnd).key().left(prefixLen) == pref && iEnd+1 != map.end()) {
         ++iEnd;
      }

      endOk = (iEnd.key().left(prefixLen) == pref);

      size = ::ceil(size/2.0f);
   }

   while (iBeg.key().left(prefixLen) != pref && iBeg != map.end() && iBeg != iEnd)
      ++iBeg;

   if (iEnd == iBeg && iBeg.key().left(prefixLen) != pref) {
      iEnd = map.end();
      iBeg = map.end();
   }

   while(iBeg != iEnd) {
      foreach(ContactMethod* n,iBeg.value()->numbers) {
         if (n) {
            set << n;
         }
      }

      ++iBeg;
   }
}

void NumberCompletionModelPrivate::locateNameRange(const QString& prefix, QSet<ContactMethod*>& set)
{
   getRange(PhoneDirectoryModel::instance()->d_ptr->m_lSortedNames,prefix,set);
}

void NumberCompletionModelPrivate::locateNumberRange(const QString& prefix, QSet<ContactMethod*>& set)
{
   getRange(PhoneDirectoryModel::instance()->d_ptr->m_hSortedNumbers,prefix,set);
}

uint NumberCompletionModelPrivate::getWeight(ContactMethod* number)
{
   uint weight = 1;

   weight += (number->weekCount()+1)*150;
   weight += (number->trimCount()+1)*75 ;
   weight += (number->callCount()+1)*35 ;
   weight *= (number->uri().indexOf(m_Prefix)!= -1?3:1);
   weight *= (number->isPresent()?2:1);

   return weight;
}

uint NumberCompletionModelPrivate::getWeight(Account* account)
{
   if ((!account) || account->registrationState() != Account::RegistrationState::READY)
      return 0; //TODO handle the case where the account get registered during dialing

   uint weight = 1;

   weight += (account->weekCallCount         ()+1)*15;
   weight += (account->trimesterCallCount    ()+1)*7 ;
   weight += (account->totalCallCount        ()+1)*3 ;
   weight *= (account->isUsedForOutgogingCall()?3:1 );

   return weight;
}

QString NumberCompletionModel::prefix() const
{
   return d_ptr->m_Prefix;
}

void NumberCompletionModel::setUseUnregisteredAccounts(bool value)
{
   d_ptr->m_UseUnregisteredAccount = value;
}

bool NumberCompletionModel::isUsingUnregisteredAccounts()
{
   return d_ptr->m_UseUnregisteredAccount;
}


void NumberCompletionModel::setDisplayMostUsedNumbers(bool value)
{
   d_ptr->m_DisplayMostUsedNumbers = value;
}

bool NumberCompletionModel::displayMostUsedNumbers() const
{
   return d_ptr->m_DisplayMostUsedNumbers;
}

bool NumberCompletionModelPrivate::accountAdded(Account* a)
{
   bool hasNonIp2Ip = false;

   switch(a->protocol()) {
      case Account::Protocol::SIP :
         hasNonIp2Ip = true;
         //no break
      case Account::Protocol::IAX : {
         TemporaryContactMethod* cm = new TemporaryContactMethod();
         cm->setAccount(a);
         m_hSipIaxTemporaryNumbers[a] = cm;
         }
         break;
      case Account::Protocol::RING: {
         TemporaryContactMethod* cm = new TemporaryContactMethod();
         cm->setAccount(a);
         m_hRingTemporaryNumbers[a] = cm;
         }
         break;
      case Account::Protocol::COUNT__:
         break;
   }

   return hasNonIp2Ip;
}

void NumberCompletionModelPrivate::accountRemoved(Account* a)
{
   TemporaryContactMethod* cm = m_hSipIaxTemporaryNumbers[a];

   if (!cm)
      cm = m_hRingTemporaryNumbers[a];

   m_hSipIaxTemporaryNumbers[a] = nullptr;
   m_hRingTemporaryNumbers  [a] = nullptr;

   setPrefix(q_ptr->prefix());

   if (cm) {
      delete cm;
   }
}

#include <numbercompletionmodel.moc>
