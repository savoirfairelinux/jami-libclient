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
#include "contactmethod.h"

//Qt
#include <QtCore/QCryptographicHash>

//Ring daemon
#include "dbus/configurationmanager.h"

//Ring
#include "phonedirectorymodel.h"
#include "person.h"
#include "account.h"
#include "media/recordingmodel.h"
#include "private/account_p.h"
#include "private/person_p.h"
#include "private/contactmethod_p.h"
#include "call.h"
#include "dbus/presencemanager.h"
#include "numbercategorymodel.h"
#include "private/numbercategorymodel_p.h"
#include "numbercategory.h"

//Private
#include "private/phonedirectorymodel_p.h"

QHash<int,Call*> ContactMethod::m_shMostUsed = QHash<int,Call*>();

const ContactMethod* ContactMethod::m_spBlank = nullptr;

void ContactMethodPrivate::callAdded(Call* call)
{
   foreach (ContactMethod* n, m_lParents)
      emit n->callAdded(call);
}

void ContactMethodPrivate::changed()
{
   foreach (ContactMethod* n, m_lParents)
      emit n->changed();
}

void ContactMethodPrivate::presentChanged(bool s)
{
   foreach (ContactMethod* n, m_lParents)
      emit n->presentChanged(s);
}

void ContactMethodPrivate::presenceMessageChanged(const QString& status)
{
   foreach (ContactMethod* n, m_lParents)
      emit n->presenceMessageChanged(status);
}

void ContactMethodPrivate::trackedChanged(bool t)
{
   foreach (ContactMethod* n, m_lParents)
      emit n->trackedChanged(t);
}

void ContactMethodPrivate::primaryNameChanged(const QString& name)
{
   foreach (ContactMethod* n, m_lParents)
      emit n->primaryNameChanged(name);
}

void ContactMethodPrivate::rebased(ContactMethod* other)
{
   foreach (ContactMethod* n, m_lParents)
      emit n->rebased(other);
}


const ContactMethod* ContactMethod::BLANK()
{
   if (!m_spBlank) {
      m_spBlank = new ContactMethod(QString(),NumberCategoryModel::other());
      const_cast<ContactMethod*>(m_spBlank)->d_ptr->m_Type = ContactMethod::Type::BLANK;
   }
   return m_spBlank;
}

ContactMethodPrivate::ContactMethodPrivate(const URI& uri, NumberCategory* cat, ContactMethod::Type st) :
   m_Uri(uri),m_pCategory(cat),m_Tracked(false),m_Present(false),m_LastUsed(0),
   m_Type(st),m_PopularityIndex(-1),m_pPerson(nullptr),m_pAccount(nullptr),
   m_LastWeekCount(0),m_LastTrimCount(0),m_HaveCalled(false),m_IsBookmark(false),m_TotalSeconds(0),
   m_Index(-1),m_hasType(false),m_pTextRecording(nullptr)
{}

///Constructor
ContactMethod::ContactMethod(const URI& number, NumberCategory* cat, Type st) : ItemBase(PhoneDirectoryModel::instance()),
d_ptr(new ContactMethodPrivate(number,cat,st))
{
   setObjectName(d_ptr->m_Uri);
   d_ptr->m_hasType = cat != NumberCategoryModel::other();
   if (d_ptr->m_hasType) {
      NumberCategoryModel::instance()->d_ptr->registerNumber(this);
   }
   d_ptr->m_lParents << this;
}

ContactMethod::~ContactMethod()
{
   d_ptr->m_lParents.removeAll(this);
   if (!d_ptr->m_lParents.size())
      delete d_ptr;
}

///Return if this number presence is being tracked
bool ContactMethod::isTracked() const
{
   //If the number doesn't support it, ignore the flag
   return supportPresence() && d_ptr->m_Tracked;
}

///Is this number present
bool ContactMethod::isPresent() const
{
   return d_ptr->m_Tracked && d_ptr->m_Present;
}

///This number presence status string
QString ContactMethod::presenceMessage() const
{
   return d_ptr->m_PresentMessage;
}

///Return the number
URI ContactMethod::uri() const {
   return d_ptr->m_Uri ;
}

///This phone number has a type
bool ContactMethod::hasType() const
{
   return d_ptr->m_hasType;
}

///Protected getter to get the number index
int ContactMethod::index() const
{
   return d_ptr->m_Index;
}

///Return the phone number type
NumberCategory* ContactMethod::category() const {
   return d_ptr->m_pCategory ;
}

///Return this number associated account, if any
Account* ContactMethod::account() const
{
   return d_ptr->m_pAccount;
}

///Return this number associated contact, if any
Person* ContactMethod::contact() const
{
   return d_ptr->m_pPerson;
}

///Return when this number was last used
time_t ContactMethod::lastUsed() const
{
   return d_ptr->m_LastUsed;
}

///Set this number default account
void ContactMethod::setAccount(Account* account)
{

   //Add the statistics
   if (account && !d_ptr->m_pAccount) {
      account->d_ptr->m_HaveCalled    += d_ptr->m_HaveCalled    ;
      account->d_ptr->m_TotalCount    += callCount()            ;
      account->d_ptr->m_LastWeekCount += d_ptr->m_LastWeekCount ;
      account->d_ptr->m_LastTrimCount += d_ptr->m_LastTrimCount ;

      if (d_ptr->m_LastUsed > account->d_ptr->m_LastUsed)
         account->d_ptr->m_LastUsed = d_ptr->m_LastUsed;
   }

   d_ptr->m_pAccount = account;

   //The sha1 is no longer valid
   d_ptr->m_Sha1.clear();

   if (d_ptr->m_pAccount)
      connect (d_ptr->m_pAccount,SIGNAL(destroyed(QObject*)),this,SLOT(accountDestroyed(QObject*)));
   d_ptr->changed();
}

///Set this number contact
void ContactMethod::setPerson(Person* contact)
{
   if (d_ptr->m_pPerson == contact)
      return;

   d_ptr->m_pPerson = contact;

   //The sha1 is no longer valid
   d_ptr->m_Sha1.clear();

   contact->d_ptr->registerContactMethod(this);

   if (contact && d_ptr->m_Type != ContactMethod::Type::TEMPORARY) {
      PhoneDirectoryModel::instance()->d_ptr->indexNumber(this,d_ptr->m_hNames.keys()+QStringList(contact->formattedName()));
      d_ptr->m_PrimaryName_cache = contact->formattedName();
      d_ptr->primaryNameChanged(d_ptr->m_PrimaryName_cache);
      connect(contact,SIGNAL(rebased(Person*)),this,SLOT(contactRebased(Person*)));
   }
   d_ptr->changed();
}

///Protected setter to set if there is a type
void ContactMethod::setHasType(bool value)
{
   d_ptr->m_hasType = value;
}

///Protected setter to set the PhoneDirectoryModel index
void ContactMethod::setIndex(int value)
{
   d_ptr->m_Index = value;
}

///Protected setter to change the popularity index
void ContactMethod::setPopularityIndex(int value)
{
   d_ptr->m_PopularityIndex = value;
}

void ContactMethod::setCategory(NumberCategory* cat)
{
   if (cat == d_ptr->m_pCategory) return;
   if (d_ptr->m_hasType)
      NumberCategoryModel::instance()->d_ptr->unregisterNumber(this);
   d_ptr->m_hasType = cat != NumberCategoryModel::other();
   d_ptr->m_pCategory = cat;
   if (d_ptr->m_hasType)
      NumberCategoryModel::instance()->d_ptr->registerNumber(this);
   d_ptr->changed();
}

void ContactMethod::setBookmarked(bool bookmarked )
{
   d_ptr->m_IsBookmark = bookmarked;
}

///Force an Uid on this number (instead of hash)
void ContactMethod::setUid(const QString& uri)
{
   d_ptr->m_Uid = uri;
}

///Attempt to change the number type
bool ContactMethod::setType(ContactMethod::Type t)
{
   if (d_ptr->m_Type == ContactMethod::Type::BLANK)
      return false;
   if (account() && t == ContactMethod::Type::ACCOUNT) {
      if (account()->supportPresenceSubscribe()) {
         d_ptr->m_Tracked = true; //The daemon will init the tracker itself
         d_ptr->trackedChanged(true);
      }
      d_ptr->m_Type = t;
      return true;
   }
   return false;
}

///Set if this number is tracking presence information
void ContactMethod::setTracked(bool track)
{
   if (track != d_ptr->m_Tracked) { //Subscribe only once
      //You can't subscribe without account
      if (track && !d_ptr->m_pAccount) return;
      d_ptr->m_Tracked = track;
      DBus::PresenceManager::instance().subscribeBuddy(d_ptr->m_pAccount->id(),uri().fullUri(),track);
      d_ptr->changed();
      d_ptr->trackedChanged(track);
   }
}

///Allow phonedirectorymodel to change presence status
void ContactMethod::setPresent(bool present)
{
   if (d_ptr->m_Present != present) {
      d_ptr->m_Present = present;
      d_ptr->presentChanged(present);
   }
}

void ContactMethod::setPresenceMessage(const QString& message)
{
   if (d_ptr->m_PresentMessage != message) {
      d_ptr->m_PresentMessage = message;
      d_ptr->presenceMessageChanged(message);
   }
}

///Return the current type of the number
ContactMethod::Type ContactMethod::type() const
{
   return d_ptr->m_Type;
}

///Return the number of calls from this number
int ContactMethod::callCount() const
{
   return d_ptr->m_lCalls.size();
}

uint ContactMethod::weekCount() const
{
   return d_ptr->m_LastWeekCount;
}

uint ContactMethod::trimCount() const
{
   return d_ptr->m_LastTrimCount;
}

bool ContactMethod::haveCalled() const
{
   return d_ptr->m_HaveCalled;
}

///Best bet for this person real name
QString ContactMethod::primaryName() const
{
   //Compute the primary name
   if (d_ptr->m_PrimaryName_cache.isEmpty()) {
      QString ret;
      if (d_ptr->m_hNames.size() == 1)
         ret =  d_ptr->m_hNames.constBegin().key();
      else {
         QString toReturn = uri();
         int max = 0;
         for (QHash<QString,int>::const_iterator i = d_ptr->m_hNames.begin(); i != d_ptr->m_hNames.end(); ++i) {
            if (i.value() > max) {
               max      = i.value();
               toReturn = i.key  ();
            }
         }
         ret = toReturn;
      }
      const_cast<ContactMethod*>(this)->d_ptr->m_PrimaryName_cache = ret;
      const_cast<ContactMethod*>(this)->d_ptr->primaryNameChanged(d_ptr->m_PrimaryName_cache);
   }
   //Fallback: Use the URI
   if (d_ptr->m_PrimaryName_cache.isEmpty()) {
      return uri();
   }

   //Return the cached primaryname
   return d_ptr->m_PrimaryName_cache;
}

///Is this number bookmarked
bool ContactMethod::isBookmarked() const
{
   return d_ptr->m_IsBookmark;
}

///If this number could (theoretically) support presence status
bool ContactMethod::supportPresence() const
{
   //Without an account, presence is impossible
   if (!d_ptr->m_pAccount)
      return false;
   //The account also have to support it
   if (!d_ptr->m_pAccount->supportPresenceSubscribe())
       return false;

   //In the end, it all come down to this, is the number tracked
   return true;
}

///Proxy accessor to the category icon
QVariant ContactMethod::icon() const
{
   return category()->icon(isTracked(),isPresent());
}

QVariant TemporaryContactMethod::icon() const
{
   return QVariant(); //TODO use the pixmap delegate to get a better icon
}

///The number of seconds spent with the URI (from history)
int ContactMethod::totalSpentTime() const
{
   return d_ptr->m_TotalSeconds;
}

///Return this number unique identifier (hash)
QString ContactMethod::uid() const
{
   return d_ptr->m_Uid.isEmpty()?toHash():d_ptr->m_Uid;
}

///Return the URI protocol hint
URI::ProtocolHint ContactMethod::protocolHint() const
{
   return d_ptr->m_Uri.protocolHint();
}

///Create a SHA1 hash identifying this contact method
QByteArray ContactMethod::sha1() const
{
   if (d_ptr->m_Sha1.isEmpty()) {
      QCryptographicHash hash(QCryptographicHash::Sha1);
      hash.addData(toHash().toLatin1());

      //Create a reproducible key for this file
      d_ptr->m_Sha1 = hash.result().toHex();
   }
   return d_ptr->m_Sha1;
}

///Return all calls from this number
QList<Call*> ContactMethod::calls() const
{
   return d_ptr->m_lCalls;
}

///Return the phonenumber position in the popularity index
int ContactMethod::popularityIndex() const
{
   return d_ptr->m_PopularityIndex;
}

QHash<QString,int> ContactMethod::alternativeNames() const
{
   return d_ptr->m_hNames;
}

QVariant ContactMethod::roleData(int role) const
{
   QVariant cat;
   switch (role) {
      case static_cast<int>(Call::Role::Name):
         cat = contact()?contact()->formattedName():primaryName();
         break;
      case Qt::ToolTipRole:
         cat = presenceMessage();
         break;
      case Qt::DisplayRole:
      case Qt::EditRole:
      case static_cast<int>(Role::Uri):
      case static_cast<int>(Call::Role::Number):
         cat = uri();//call->getPeerContactMethod();
         break;
      case static_cast<int>(Call::Role::Direction):
         cat = tr("N/A");//call->getHistoryState();
         break;
      case static_cast<int>(Call::Role::Date):
         cat = tr("N/A");//call->getStartTimeStamp();
         break;
      case static_cast<int>(Call::Role::Length):
         cat = tr("N/A");//call->getLength();
         break;
      case static_cast<int>(Call::Role::FormattedDate):
         cat = tr("N/A");//QDateTime::fromTime_t(call->getStartTimeStamp().toUInt()).toString();
         break;
      case static_cast<int>(Call::Role::HasAVRecording):
         cat = false;//call->hasRecording();
         break;
      case static_cast<int>(Call::Role::FuzzyDate):
         cat = "N/A";//timeToHistoryCategory(QDateTime::fromTime_t(call->getStartTimeStamp().toUInt()).date());
         break;
      case static_cast<int>(Call::Role::ContactMethod):
      case static_cast<int>(Role::Object):
         cat = QVariant::fromValue(const_cast<ContactMethod*>(this));
         break;
      case static_cast<int>(Call::Role::IsBookmark):
         cat = false;
         break;
      case static_cast<int>(Call::Role::Filter):
         cat = uri()+primaryName();
         break;
      case static_cast<int>(Call::Role::IsPresent):
         cat = isPresent();
         break;
      case static_cast<int>(Call::Role::Photo):
         if (contact())
            cat = contact()->photo();
         break;
      case static_cast<int>(Role::CategoryIcon):
         if (category())
            cat = d_ptr->m_pCategory->icon(isTracked(), isPresent());
         break;
      case static_cast<int>(Call::Role::LifeCycleState):
         return QVariant::fromValue(Call::LifeCycleState::FINISHED);
   }
   return cat;
}

///Add a call to the call list, notify listener
void ContactMethod::addCall(Call* call)
{
   if (!call) return;

   //Update the contact method statistics
   d_ptr->m_Type = ContactMethod::Type::USED;
   d_ptr->m_lCalls << call;
   d_ptr->m_TotalSeconds += call->stopTimeStamp() - call->startTimeStamp();
   time_t now;
   ::time ( &now );

   if (now - 3600*24*7 < call->stopTimeStamp()) {
      d_ptr->m_LastWeekCount++;
      if (d_ptr->m_pAccount)
         d_ptr->m_pAccount->d_ptr->m_LastWeekCount++;
   }

   if (now - 3600*24*7*15 < call->stopTimeStamp()) {
      d_ptr->m_LastTrimCount++;
      if (d_ptr->m_pAccount)
         d_ptr->m_pAccount->d_ptr->m_LastTrimCount++;
   }

   if (call->direction() == Call::Direction::OUTGOING) {
      d_ptr->m_HaveCalled = true;
      if (d_ptr->m_pAccount)
         d_ptr->m_pAccount->d_ptr->m_HaveCalled = true;
   }

   d_ptr->callAdded(call);

   if (call->startTimeStamp() > d_ptr->m_LastUsed) {
      d_ptr->m_LastUsed = call->startTimeStamp();

      if (d_ptr->m_LastUsed)
         emit lastUsedChanged(d_ptr->m_LastUsed);

      //Notify the account directly. This avoid having to track all contact
      //methods from there
      if (d_ptr->m_pAccount && d_ptr->m_pAccount->d_ptr->m_LastUsed < d_ptr->m_LastUsed)
         d_ptr->m_pAccount->d_ptr->m_LastUsed = d_ptr->m_LastUsed;
   }

   d_ptr->changed();

   if (d_ptr->m_pAccount)
      d_ptr->m_pAccount->d_ptr->m_TotalCount++;
}

///Generate an unique representation of this number
QString ContactMethod::toHash() const
{
   QString uristr;

   switch(uri().protocolHint()) {
      case URI::ProtocolHint::RING     :
         //There is no point in keeping the full URI, a Ring hash is unique
         uristr = uri().userinfo();
         break;
      case URI::ProtocolHint::SIP_OTHER:
      case URI::ProtocolHint::IAX      :
      case URI::ProtocolHint::IP       :
      case URI::ProtocolHint::SIP_HOST :
         //Some URI have port number in them. They have to be stripped prior to the hash creation
         uristr = uri().format(
            URI::Section::CHEVRONS  |
            URI::Section::SCHEME    |
            URI::Section::USER_INFO |
            URI::Section::HOSTNAME
         );
         break;
   }

   return QString("%1///%2///%3")
      .arg(
         uristr
      )
      .arg(
         account()?account()->id():QString()
      )
      .arg(
         contact()?contact()->uid():QString()
      );
}

///Increment name counter and update indexes
void ContactMethod::incrementAlternativeName(const QString& name)
{
   const bool needReIndexing = !d_ptr->m_hNames[name];
   d_ptr->m_hNames[name]++;
   if (needReIndexing && d_ptr->m_Type != ContactMethod::Type::TEMPORARY) {
      PhoneDirectoryModel::instance()->d_ptr->indexNumber(this,d_ptr->m_hNames.keys()+(d_ptr->m_pPerson?(QStringList(d_ptr->m_pPerson->formattedName())):QStringList()));
      //Invalid m_PrimaryName_cache
      if (!d_ptr->m_pPerson)
         d_ptr->m_PrimaryName_cache.clear();
   }
}

void ContactMethod::accountDestroyed(QObject* o)
{
   if (o == d_ptr->m_pAccount)
      d_ptr->m_pAccount = nullptr;
}

/**
 * When the ContactMethod contact is merged with another one, the phone number
 * data might be replaced, like the preferred name.
 */
void ContactMethod::contactRebased(Person* other)
{
   d_ptr->m_PrimaryName_cache = other->formattedName();
   d_ptr->primaryNameChanged(d_ptr->m_PrimaryName_cache);
   d_ptr->changed();

   //It is a "partial" rebase, so the ContactMethod data stay the same
   d_ptr->rebased(this);
}

/**
 * Merge two phone number to share the same data. This avoid having to change
 * pointers all over the place. The ContactMethod objects remain intact, the
 * PhoneDirectoryModel will replace the old references, but existing ones will
 * keep working.
 */
bool ContactMethod::merge(ContactMethod* other)
{

   if ((!other) || other == this || other->d_ptr == d_ptr)
      return false;

   //This is invalid, those are different numbers
   if (account() && other->account() && account() != other->account())
      return false;

   //TODO Check if the merge is valid

   //TODO Merge the alternative names

   //TODO Handle presence

   const QString oldName = primaryName();

   ContactMethodPrivate* currentD = d_ptr;

   //Replace the D-Pointer
   this->d_ptr= other->d_ptr;
   d_ptr->m_lParents << this;

   //In case the URI is different, take the longest and most precise
   //TODO keep a log of all URI used
   if (currentD->m_Uri.size() > other->d_ptr->m_Uri.size()) {
      other->d_ptr->m_lOtherURIs << other->d_ptr->m_Uri;
      other->d_ptr->m_Uri = currentD->m_Uri;
   }
   else
      other->d_ptr->m_lOtherURIs << currentD->m_Uri;

   emit changed();
   emit rebased(other);

   if (oldName != primaryName())
      d_ptr->primaryNameChanged(primaryName());

   currentD->m_lParents.removeAll(this);
   if (!currentD->m_lParents.size())
      delete currentD;
   return true;
}

bool ContactMethod::operator==(ContactMethod* other)
{
   return other && this->d_ptr== other->d_ptr;
}

bool ContactMethod::operator==(const ContactMethod* other) const
{
   return other && this->d_ptr== other->d_ptr;
}

bool ContactMethod::operator==(ContactMethod& other)
{
   return this->d_ptr== other.d_ptr;
}

bool ContactMethod::operator==(const ContactMethod& other) const
{
   return this->d_ptr== other.d_ptr;
}

Media::TextRecording* ContactMethod::textRecording() const
{
   if ((!d_ptr->m_hasTriedTextRec) && (!d_ptr->m_pTextRecording)) {
      d_ptr->m_pTextRecording = Media::RecordingModel::instance()->createTextRecording(this);
      d_ptr->m_hasTriedTextRec = true;
   }

   return d_ptr->m_pTextRecording;
}

void ContactMethodPrivate::setTextRecording(Media::TextRecording* r)
{
   m_pTextRecording = r;
}

bool ContactMethod::sendOfflineTextMessage(const QString& text)
{
   if (!account())
      return false;

   DBus::ConfigurationManager::instance().sendTextMessage(account()->id(),uri(),text);
   return true;
}


/************************************************************************************
 *                                                                                  *
 *                             Temporary phone number                               *
 *                                                                                  *
 ***********************************************************************************/

void TemporaryContactMethod::setUri(const URI& uri)
{
   ContactMethod::d_ptr->m_Uri = uri;

   //The sha1 is no longer valid
   ContactMethod::d_ptr->m_Sha1.clear();

   ContactMethod::d_ptr->changed();
}

///Constructor
TemporaryContactMethod::TemporaryContactMethod(const ContactMethod* number) :
   ContactMethod(QString(),NumberCategoryModel::other(),ContactMethod::Type::TEMPORARY),d_ptr(nullptr)
{
   if (number) {
      setPerson(number->contact());
      setAccount(number->account());
   }
}

Q_DECLARE_METATYPE(QList<Call*>)
