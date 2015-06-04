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
#include "phonedirectorymodel.h"

//Qt
#include <QtCore/QCoreApplication>

//DRing
#include <account_const.h>

//Ring
#include "contactmethod.h"
#include "call.h"
#include "uri.h"
#include "account.h"
#include "person.h"
#include "accountmodel.h"
#include "numbercategory.h"
#include "numbercategorymodel.h"
#include "collectioninterface.h"
#include "dbus/presencemanager.h"
#include "delegates/pixmapmanipulationdelegate.h"
#include "personmodel.h"

//Private
#include "private/phonedirectorymodel_p.h"

PhoneDirectoryModel* PhoneDirectoryModel::m_spInstance = nullptr;

PhoneDirectoryModelPrivate::PhoneDirectoryModelPrivate(PhoneDirectoryModel* parent) : QObject(parent), q_ptr(parent),
m_CallWithAccount(false),m_pPopularModel(nullptr)
{
}

PhoneDirectoryModel::PhoneDirectoryModel(QObject* parent) :
   QAbstractTableModel(parent?parent:QCoreApplication::instance()), d_ptr(new PhoneDirectoryModelPrivate(this))
{
   setObjectName("PhoneDirectoryModel");
   connect(&DBus::PresenceManager::instance(),SIGNAL(newBuddyNotification(QString,QString,bool,QString)),d_ptr.data(),
           SLOT(slotNewBuddySubscription(QString,QString,bool,QString)));
}

PhoneDirectoryModel::~PhoneDirectoryModel()
{
   QList<NumberWrapper*> vals = d_ptr->m_hNumbersByNames.values();
   //Used by indexes
   d_ptr->m_hNumbersByNames.clear();
   d_ptr->m_lSortedNames.clear();
   while (vals.size()) {
      NumberWrapper* w = vals[0];
      vals.removeAt(0);
      delete w;
   }

   //Used by auto completion
   vals = d_ptr->m_hSortedNumbers.values();
   d_ptr->m_hSortedNumbers.clear();
   d_ptr->m_hDirectory.clear();
   while (vals.size()) {
      NumberWrapper* w = vals[0];
      vals.removeAt(0);
      delete w;
   }
}

PhoneDirectoryModel* PhoneDirectoryModel::instance()
{
   if (!m_spInstance) {
      m_spInstance = new PhoneDirectoryModel();
   }
   return m_spInstance;
}

QHash<int,QByteArray> PhoneDirectoryModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

QVariant PhoneDirectoryModel::data(const QModelIndex& index, int role ) const
{
   if (!index.isValid() || index.row() >= d_ptr->m_lNumbers.size()) return QVariant();
   const ContactMethod* number = d_ptr->m_lNumbers[index.row()];
   switch (static_cast<PhoneDirectoryModelPrivate::Columns>(index.column())) {
      case PhoneDirectoryModelPrivate::Columns::URI:
         switch (role) {
            case Qt::DisplayRole:
               return number->uri();
            case Qt::DecorationRole :
               return PixmapManipulationDelegate::instance()->callPhoto(number,QSize(16,16));
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::TYPE:
         switch (role) {
            case Qt::DisplayRole:
               return number->category()->name();
            case Qt::DecorationRole:
               return number->icon();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::CONTACT:
         switch (role) {
            case Qt::DisplayRole:
               return number->contact()?number->contact()->formattedName():QVariant();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::ACCOUNT:
         switch (role) {
            case Qt::DisplayRole:
               return number->account()?number->account()->id():QVariant();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::STATE:
         switch (role) {
            case Qt::DisplayRole:
               return (int)number->type();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::CALL_COUNT:
         switch (role) {
            case Qt::DisplayRole:
               return number->callCount();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::LAST_USED:
         switch (role) {
            case Qt::DisplayRole:
               return (int)number->lastUsed();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::NAME_COUNT:
         switch (role) {
            case Qt::DisplayRole:
               return number->alternativeNames().size();
               break;
            case Qt::ToolTipRole: {
               QString out = "<table>";
               QHashIterator<QString, int> iter(number->alternativeNames());
               while (iter.hasNext()) {
                  iter.next();
                  out += QString("<tr><td>%1</td><td>%2</td></tr>").arg(iter.value()).arg(iter.key());
               }
               out += "</table>";
               return out;
            }
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::TOTAL_SECONDS:
         switch (role) {
            case Qt::DisplayRole:
               return number->totalSpentTime();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::WEEK_COUNT:
         switch (role) {
            case Qt::DisplayRole:
               return number->weekCount();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::TRIM_COUNT:
         switch (role) {
            case Qt::DisplayRole:
               return number->trimCount();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::HAVE_CALLED:
         switch (role) {
            case Qt::DisplayRole:
               return number->haveCalled();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::POPULARITY_INDEX:
         switch (role) {
            case Qt::DisplayRole:
               return number->popularityIndex();
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::BOOKMARED:
         switch (role) {
            case Qt::CheckStateRole:
               return number->isBookmarked()?Qt::Checked:Qt::Unchecked;
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::TRACKED:
         switch (role) {
            case Qt::CheckStateRole:
               if (number->account() && number->account()->supportPresenceSubscribe())
                  return number->isTracked()?Qt::Checked:Qt::Unchecked;
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::PRESENT:
         switch (role) {
            case Qt::CheckStateRole:
               return number->isPresent()?Qt::Checked:Qt::Unchecked;
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::PRESENCE_MESSAGE:
         switch (role) {
            case Qt::DisplayRole: {
               if ((index.column() == static_cast<int>(PhoneDirectoryModelPrivate::Columns::TRACKED)
                  || static_cast<int>(PhoneDirectoryModelPrivate::Columns::PRESENT))
                  && number->account() && (!number->account()->supportPresenceSubscribe())) {
                  return tr("This account does not support presence tracking");
               }
               else if (!number->account())
                  return tr("No associated account");
               else
                  return number->presenceMessage();
            }
         }
         break;
      case PhoneDirectoryModelPrivate::Columns::UID:
         switch (role) {
            case Qt::DisplayRole:
            case Qt::ToolTipRole:
               return number->uid();
         }
         break;
   }
   return QVariant();
}

int PhoneDirectoryModel::rowCount(const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return d_ptr->m_lNumbers.size();
}

int PhoneDirectoryModel::columnCount(const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return 18;
}

Qt::ItemFlags PhoneDirectoryModel::flags(const QModelIndex& index ) const
{
   Q_UNUSED(index)

   const ContactMethod* number = d_ptr->m_lNumbers[index.row()];
   const bool enabled = !((index.column() == static_cast<int>(PhoneDirectoryModelPrivate::Columns::TRACKED)
      || static_cast<int>(PhoneDirectoryModelPrivate::Columns::PRESENT))
      && number->account() && (!number->account()->supportPresenceSubscribe()));

   return Qt::ItemIsEnabled
      | Qt::ItemIsSelectable 
      | (index.column() == static_cast<int>(PhoneDirectoryModelPrivate::Columns::TRACKED)&&enabled?Qt::ItemIsUserCheckable:Qt::NoItemFlags);
}

///This model is read and for debug purpose
bool PhoneDirectoryModel::setData(const QModelIndex& index, const QVariant &value, int role )
{
   ContactMethod* number = d_ptr->m_lNumbers[index.row()];
   if (static_cast<PhoneDirectoryModelPrivate::Columns>(index.column())==PhoneDirectoryModelPrivate::Columns::TRACKED) {
      if (role == Qt::CheckStateRole && number) {
         number->setTracked(value.toBool());
      }
   }
   return false;
}

QVariant PhoneDirectoryModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
   Q_UNUSED(section)
   Q_UNUSED(orientation)
   static const QString headers[] = {tr("URI"), tr("Type"), tr("Person"), tr("Account"), tr("State"), tr("Call count"), tr("Week count"),
   tr("Trimester count"), tr("Have Called"), tr("Last used"), tr("Name_count"),tr("Total (in seconds)"), tr("Popularity_index"), tr("Bookmarked"), tr("Tracked"), tr("Present"),
   tr("Presence message"), tr("Uid") };
   if (role == Qt::DisplayRole) return headers[section];
   return QVariant();
}

/**
 * This helper method make sure that number without an account get registered
 * correctly with their alternate URIs. In case there is an obvious duplication,
 * it will try to merge both numbers.
 */
void PhoneDirectoryModelPrivate::setAccount(ContactMethod* number, Account* account ) {
   const URI& strippedUri = number->uri();
   const bool hasAtSign = strippedUri.hasHostname();
   number->setAccount(account);

   if (!hasAtSign) {
      NumberWrapper* wrap = m_hDirectory[strippedUri];

      //Let make sure none is created in the future for nothing
      if (!wrap) {
         //It wont be a duplicate as none exist for this URI
         const QString extendedUri = strippedUri+'@'+account->hostname();
         wrap = new NumberWrapper();
         m_hDirectory    [extendedUri] = wrap;
         m_hSortedNumbers[extendedUri] = wrap;

      }
      else {
         //After all this, it is possible the number is now a duplicate
         foreach(ContactMethod* n, wrap->numbers) {
            if (n != number && n->account() && n->account() == number->account()) {
               number->merge(n);
            }
         }
      }
      wrap->numbers << number;

   }
}

/**
 * This version of getNumber() try to get a phone number with a contact from an URI and account
 * It will also try to attach an account to existing numbers. This is not 100% reliable, but
 * it is correct often enough to do it.
 */
ContactMethod* PhoneDirectoryModel::getNumber(const QString& uri, Account* account, const QString& type)
{
   return getNumber(uri,nullptr,account,type);
}

///Add new information to existing numbers and try to merge
ContactMethod* PhoneDirectoryModelPrivate::fillDetails(NumberWrapper* wrap, const URI& strippedUri, Account* account, Person* contact, const QString& type)
{
   //TODO pick the best URI
   //TODO the account hostname change corner case
   //TODO search for account that has the same hostname as the URI
   if (wrap) {
      foreach(ContactMethod* number, wrap->numbers) {

         //BEGIN Check if contact can be set

         //Check if the contact is compatible
         const bool hasCompatiblePerson = contact && (
               (!number->contact())
            || (
                  (number->contact()->uid() == contact->uid())
               && number->contact() != contact
            )
         );

         //Check if the URI match
         const bool hasCompatibleURI =  hasCompatiblePerson && (number->uri().hasHostname()?(
               /* Has hostname */
                   strippedUri == number->uri()
                   /* Something with an hostname can be used with IP2IP */ //TODO support DHT here
               || (account && account->id() == DRing::Account::ProtocolNames::IP2IP)
            ) : ( /* Has no hostname */
                  number->account() && number->uri()+'@'+number->account()->hostname() == strippedUri
            ));

         //Check if the account is compatible
         const bool hasCompatibleAccount = hasCompatibleURI && ((!account)
            || (!number->account())
            /* Direct match, this is always valid */
            || (account == number->account())
            /* IP2IP is a special case */ //TODO support DHT here
            || (
                  account->id() == DRing::Account::ProtocolNames::IP2IP
                  && strippedUri.hasHostname()
               ));

         //TODO the Display name could be used to influence the choice
         //It would need to ignore all possible translated values of unknown
         //and only be available when another information match

         //If everything match, set the contact
         if (hasCompatibleAccount)
            number->setPerson(contact);
         //END Check if the contact can be set


         //BEGIN Check is account can be set
         // Try to match the account
         // Not perfect, but better than ignoring the high probabilities
         //TODO only do it is hostname match
         if ((!account) || account != number->account()) {

            if (account && (!contact) && !number->account())
               setAccount(number,account);

            //Set a type, this has low probabilities of being invalid
            if ((!number->hasType()) && (!type.isEmpty())) {
               number->setCategory(NumberCategoryModel::instance()->getCategory(type));
            }

            //We already have enough information to confirm the choice
            if (contact && number->contact() &&((contact->uid()) == number->contact()->uid()))
               return number;
         }
         //END Check is account can be set
      }
   }
   return nullptr;
}

///Return/create a number when no information is available
ContactMethod* PhoneDirectoryModel::getNumber(const QString& uri, const QString& type)
{
   const URI strippedUri(uri);
   NumberWrapper* wrap = d_ptr->m_hDirectory[strippedUri];
   if (wrap) {
      ContactMethod* nb = wrap->numbers[0];
      if ((!nb->hasType()) && (!type.isEmpty())) {
         nb->setCategory(NumberCategoryModel::instance()->getCategory(type));
      }
      return nb;
   }

   //Too bad, lets create one
   ContactMethod* number = new ContactMethod(strippedUri,NumberCategoryModel::instance()->getCategory(type));
   number->setIndex(d_ptr->m_lNumbers.size());
   d_ptr->m_lNumbers << number;
   connect(number,SIGNAL(callAdded(Call*)),d_ptr.data(),SLOT(slotCallAdded(Call*)));
   connect(number,SIGNAL(changed()),d_ptr.data(),SLOT(slotChanged()));

   const QString hn = number->uri().hostname();

   emit layoutChanged();
   if (!wrap) {
      wrap = new NumberWrapper();
      d_ptr->m_hDirectory[strippedUri] = wrap;
      d_ptr->m_hSortedNumbers[strippedUri] = wrap;
   }
   wrap->numbers << number;
   return number;
}

///Create a number when a more information is available duplicated ones
ContactMethod* PhoneDirectoryModel::getNumber(const QString& uri, Person* contact, Account* account, const QString& type)
{
   //Remove extra data such as "<sip:" from the main URI
   const URI strippedUri(uri);

   //See if the number is already loaded
   NumberWrapper* wrap  = d_ptr->m_hDirectory[strippedUri];
   NumberWrapper* wrap2 = nullptr;
   NumberWrapper* wrap3 = nullptr;

   //Check if the URI is complete or short
   const bool hasAtSign = strippedUri.hasHostname();

   //Try to see if there is a better candidate with a suffix (LAN only)
   if ( !hasAtSign && account ) {
      //Append the account hostname
      wrap2 = d_ptr->m_hDirectory[strippedUri+'@'+account->hostname()];
   }

   //Check
   ContactMethod* confirmedCandidate = d_ptr->fillDetails(wrap,strippedUri,account,contact,type);

   //URIs can be represented in multiple way, check if a more verbose version
   //already exist
   ContactMethod* confirmedCandidate2 = nullptr;

   //Try to use a ContactMethod with a contact when possible, work only after the
   //contact are loaded
   if (confirmedCandidate && confirmedCandidate->contact())
      confirmedCandidate2 = d_ptr->fillDetails(wrap2,strippedUri,account,contact,type);

   ContactMethod* confirmedCandidate3 = nullptr;

   //Else, try to see if the hostname correspond to the account and flush it
   //This have to be done after the parent if as the above give "better"
   //results. It cannot be merged with wrap2 as this check only work if the
   //candidate has an account.
   if (hasAtSign && account && strippedUri.hostname() == account->hostname()) {
     wrap3 = d_ptr->m_hDirectory[strippedUri.userinfo()];
     if (wrap3) {
         foreach(ContactMethod* number, wrap3->numbers) {
            if (number->account() == account) {
               if (contact && ((!number->contact()) || (contact->uid() == number->contact()->uid())))
                  number->setPerson(contact); //TODO Check all cases from fillDetails()
               //TODO add alternate URI
               confirmedCandidate3 = number;
               break;
            }
         }
     }
   }

   //If multiple ContactMethod are confirmed, then they are the same, merge them
   if (confirmedCandidate3 && (confirmedCandidate || confirmedCandidate2)) {
      confirmedCandidate3->merge(confirmedCandidate?confirmedCandidate:confirmedCandidate2);
   }
   else if (confirmedCandidate && confirmedCandidate2) {
      if (confirmedCandidate->contact() && !confirmedCandidate2->contact())
         confirmedCandidate2->merge(confirmedCandidate);
      else if (confirmedCandidate2->contact() && !confirmedCandidate->contact())
         confirmedCandidate->merge(confirmedCandidate2);
   }

   //Empirical testing resulted in this as the best return order
   //The merge may have failed either in the "if" above or in the merging code
   if (confirmedCandidate2)
      return confirmedCandidate2;
   if (confirmedCandidate)
      return confirmedCandidate;
   if (confirmedCandidate3)
      return confirmedCandidate3;

   //No better candidates were found than the original assumption, use it
   if (wrap) {
      foreach(ContactMethod* number, wrap->numbers) {
         if (((!account) || number->account() == account) && ((!contact) || ((*contact) == number->contact()) || (!number->contact()))) {
            //Assume this is valid until a smarter solution is implemented to merge both
            //For a short time, a placeholder contact and a contact can coexist, drop the placeholder
            if (contact && (!number->contact() || (contact->uid() == number->contact()->uid())))
               number->setPerson(contact);

            return number;
         }
      }
   }

   //Create the number
   ContactMethod* number = new ContactMethod(strippedUri,NumberCategoryModel::instance()->getCategory(type));
   number->setAccount(account);
   number->setIndex( d_ptr->m_lNumbers.size());
   if (contact)
      number->setPerson(contact);
   d_ptr->m_lNumbers << number;
   connect(number,SIGNAL(callAdded(Call*)),d_ptr.data(),SLOT(slotCallAdded(Call*)));
   connect(number,SIGNAL(changed()),d_ptr.data(),SLOT(slotChanged()));
   if (!wrap) {
      wrap = new NumberWrapper();
      d_ptr->m_hDirectory    [strippedUri] = wrap;
      d_ptr->m_hSortedNumbers[strippedUri] = wrap;

      //Also add its alternative URI, it should be safe to do
      if ( !hasAtSign && account && !account->hostname().isEmpty() ) {
         const QString extendedUri = strippedUri+'@'+account->hostname();
         //Also check if it hasn't been created by setAccount
         if ((!wrap2) && (!d_ptr->m_hDirectory[extendedUri])) {
            wrap2 = new NumberWrapper();
            d_ptr->m_hDirectory    [extendedUri] = wrap2;
            d_ptr->m_hSortedNumbers[extendedUri] = wrap2;
         }
         wrap2->numbers << number;
      }

   }
   wrap->numbers << number;
   emit layoutChanged();

   return number;
}

ContactMethod* PhoneDirectoryModel::fromTemporary(const TemporaryContactMethod* number)
{
   return getNumber(number->uri(),number->contact(),number->account());
}

ContactMethod* PhoneDirectoryModel::fromHash(const QString& hash)
{
   const QStringList fields = hash.split("///");
   if (fields.size() == 3) {
      const QString uri = fields[0];
      const QByteArray acc = fields[1].toLatin1();
      Account* account = acc.isEmpty() ? nullptr : AccountModel::instance()->getById(acc);
      Person* contact = PersonModel::instance()->getPersonByUid(fields[2].toUtf8());
      return getNumber(uri,contact,account);
   }
   else if (fields.size() == 1) {
      //FIXME Remove someday, handle version v1.0 to v1.2.3 bookmark format
      return getNumber(fields[0]);
   }
   qDebug() << "Invalid hash" << hash;
   return nullptr;
}

QVector<ContactMethod*> PhoneDirectoryModel::getNumbersByPopularity() const
{
   return d_ptr->m_lPopularityIndex;
}

void PhoneDirectoryModelPrivate::slotCallAdded(Call* call)
{
   Q_UNUSED(call)
   ContactMethod* number = qobject_cast<ContactMethod*>(sender());
   if (number) {
      int currentIndex = number->popularityIndex();

      //The number is already in the top 10 and just passed the "index-1" one
      if (currentIndex > 0 && m_lPopularityIndex[currentIndex-1]->callCount() < number->callCount()) {
         do {
            ContactMethod* tmp = m_lPopularityIndex[currentIndex-1];
            m_lPopularityIndex[currentIndex-1] = number;
            m_lPopularityIndex[currentIndex  ] = tmp   ;
            tmp->setPopularityIndex(tmp->popularityIndex()+1);
            currentIndex--;
         } while (currentIndex && m_lPopularityIndex[currentIndex-1]->callCount() < number->callCount());
         number->setPopularityIndex(currentIndex);
         emit q_ptr->layoutChanged();
         if (m_pPopularModel)
            m_pPopularModel->reload();
      }
      //The top 10 is not complete, a call count of "1" is enough to make it
      else if (m_lPopularityIndex.size() < 10 && currentIndex == -1) {
         m_lPopularityIndex << number;
         if (m_pPopularModel)
            m_pPopularModel->addRow();
         number->setPopularityIndex(m_lPopularityIndex.size()-1);
         emit q_ptr->layoutChanged();
      }
      //The top 10 is full, but this number just made it to the top 10
      else if (currentIndex == -1 && m_lPopularityIndex.size() >= 10 && m_lPopularityIndex[9] != number && m_lPopularityIndex[9]->callCount() < number->callCount()) {
         ContactMethod* tmp = m_lPopularityIndex[9];
         tmp->setPopularityIndex(-1);
         m_lPopularityIndex[9]     = number;
         number->setPopularityIndex(9);
         emit tmp->changed();
         emit number->changed();
         if (m_pPopularModel)
            m_pPopularModel->reload();
      }

      //Now check for new peer names
      if (!call->peerName().isEmpty()) {
         number->incrementAlternativeName(call->peerName());
      }
   }
}

void PhoneDirectoryModelPrivate::slotChanged()
{
   ContactMethod* number = qobject_cast<ContactMethod*>(sender());
   if (number) {
      const int idx = number->index();
#ifndef NDEBUG
      if (idx<0)
         qDebug() << "Invalid slotChanged() index!" << idx;
#endif
      emit q_ptr->dataChanged(q_ptr->index(idx,0),q_ptr->index(idx,static_cast<int>(Columns::UID)));
   }
}

void PhoneDirectoryModelPrivate::slotNewBuddySubscription(const QString& accountId, const QString& uri, bool status, const QString& message)
{
   qDebug() << "New presence buddy" << uri << status << message;
   ContactMethod* number = q_ptr->getNumber(uri,AccountModel::instance()->getById(accountId.toLatin1()));
   number->setPresent(status);
   number->setPresenceMessage(message);
   emit number->changed();
}

///Make sure the indexes are still valid for those names
void PhoneDirectoryModelPrivate::indexNumber(ContactMethod* number, const QStringList &names)
{
   foreach(const QString& name, names) {
      const QString lower = name.toLower();
      const QStringList split = lower.split(' ');
      if (split.size() > 1) {
         foreach(const QString& chunk, split) {
            NumberWrapper* wrap = m_hNumbersByNames[chunk];
            if (!wrap) {
               wrap = new NumberWrapper();
               m_hNumbersByNames[chunk] = wrap;
               m_lSortedNames[chunk]    = wrap;
            }
            const int numCount = wrap->numbers.size();
            if (!((numCount == 1 && wrap->numbers[0] == number) || (numCount > 1 && wrap->numbers.indexOf(number) != -1)))
               wrap->numbers << number;
         }
      }
      NumberWrapper* wrap = m_hNumbersByNames[lower];
      if (!wrap) {
         wrap = new NumberWrapper();
         m_hNumbersByNames[lower] = wrap;
         m_lSortedNames[lower]    = wrap;
      }
      const int numCount = wrap->numbers.size();
      if (!((numCount == 1 && wrap->numbers[0] == number) || (numCount > 1 && wrap->numbers.indexOf(number) != -1)))
         wrap->numbers << number;
   }
}

int PhoneDirectoryModel::count() const {
   return d_ptr->m_lNumbers.size();
}
bool PhoneDirectoryModel::callWithAccount() const {
   return d_ptr->m_CallWithAccount;
}

//Setters
void PhoneDirectoryModel::setCallWithAccount(bool value) {
   d_ptr->m_CallWithAccount = value;
}

///Popular number model related code

MostPopularNumberModel::MostPopularNumberModel() : QAbstractListModel(PhoneDirectoryModel::instance()) {
   setObjectName("MostPopularNumberModel");
}

QVariant MostPopularNumberModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   return PhoneDirectoryModel::instance()->d_ptr->m_lPopularityIndex[index.row()]->roleData(role);
}

int MostPopularNumberModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : PhoneDirectoryModel::instance()->d_ptr->m_lPopularityIndex.size();
}

Qt::ItemFlags MostPopularNumberModel::flags( const QModelIndex& index ) const
{
   return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}

bool MostPopularNumberModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

void MostPopularNumberModel::addRow()
{
   const int oldSize = PhoneDirectoryModel::instance()->d_ptr->m_lPopularityIndex.size()-1;
   beginInsertRows(QModelIndex(),oldSize,oldSize);
   endInsertRows();
}

void MostPopularNumberModel::reload()
{
   emit dataChanged(index(0,0),index(rowCount(),0));
}

QAbstractListModel* PhoneDirectoryModel::mostPopularNumberModel() const
{
   if (!d_ptr->m_pPopularModel)
      d_ptr->m_pPopularModel = new MostPopularNumberModel();

   return d_ptr->m_pPopularModel;
}

#include <phonedirectorymodel.moc>
