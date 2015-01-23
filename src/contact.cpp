/****************************************************************************
 *   Copyright (C) 2009-2015 by Savoir-Faire Linux                          *
 *   Author : Jérémy Quentin <jeremy.quentin@savoirfairelinux.com>          *
 *            Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

//Parent
#include "contact.h"

//Ring library
#include "phonenumber.h"
#include "abstractitembackend.h"
#include "transitionalcontactbackend.h"
#include "account.h"
#include "vcardutils.h"
#include "numbercategorymodel.h"
#include "numbercategory.h"
#include "visitors/pixmapmanipulationvisitor.h"


class AddressPrivate
{
public:
   QString addressLine;
   QString city;
   QString zipCode;
   QString state;
   QString country;
   QString type;
};

Contact::Address::Address() : d_ptr(new AddressPrivate())
{

}

QString Contact::Address::addressLine() const
{
   return d_ptr->addressLine;
}

QString Contact::Address::city() const
{
   return d_ptr->city;
}

QString Contact::Address::zipCode() const
{
   return d_ptr->zipCode;
}

QString Contact::Address::state() const
{
   return d_ptr->state;
}

QString Contact::Address::country() const
{
   return d_ptr->country;
}

QString Contact::Address::type() const
{
   return d_ptr->type;
}

void Contact::Address::setAddressLine(const QString& value)
{
   d_ptr->addressLine = value;
}

void Contact::Address::setCity(const QString& value)
{
   d_ptr->city = value;
}

void Contact::Address::setZipCode(const QString& value)
{
   d_ptr->zipCode = value;
}

void Contact::Address::setState(const QString& value)
{
   d_ptr->state = value;
}

void Contact::Address::setCountry(const QString& value)
{
   d_ptr->country = value;
}

void Contact::Address::setType(const QString& value)
{
   d_ptr->type = value;
}

class ContactPrivate {
public:
   ContactPrivate(Contact* contact, AbstractContactBackend* parent);
   ~ContactPrivate();
   QString                  m_FirstName        ;
   QString                  m_SecondName       ;
   QString                  m_NickName         ;
   QVariant                 m_vPhoto           ;
   QString                  m_FormattedName    ;
   QString                  m_PreferredEmail   ;
   QString                  m_Organization     ;
   QByteArray               m_Uid              ;
   QString                  m_Group            ;
   QString                  m_Department       ;
   bool                     m_DisplayPhoto     ;
   Contact::PhoneNumbers    m_Numbers          ;
   bool                     m_Active           ;
   AbstractContactBackend*  m_pBackend         ;
   bool                     m_isPlaceHolder    ;
   QList<Contact::Address*> m_lAddresses       ;
   QHash<QString, QString>  m_lCustomAttributes;

   //Cache
   QString m_CachedFilterString;

   QString filterString();

   //Helper code to help handle multiple parents
   QList<Contact*> m_lParents;

   //As a single D-Pointer can have multiple parent (when merged), all emit need
   //to use a proxy to make sure everybody is notified
   void presenceChanged( PhoneNumber* );
   void statusChanged  ( bool         );
   void changed        (              );
   void phoneNumberCountChanged(int,int);
   void phoneNumberCountAboutToChange(int,int);
};

QString ContactPrivate::filterString()
{
   if (m_CachedFilterString.size())
      return m_CachedFilterString;

   //Also filter by phone numbers, accents are negligible
   foreach(const PhoneNumber* n , m_Numbers) {
      m_CachedFilterString += n->uri();
   }

   //Strip non essential characters like accents from the filter string
   foreach(const QChar& char2,QString(m_FormattedName+'\n'+m_Organization+'\n'+m_Group+'\n'+
      m_Department+'\n'+m_PreferredEmail).toLower().normalized(QString::NormalizationForm_KD) ) {
      if (!char2.combiningClass())
         m_CachedFilterString += char2;
   }

   return m_CachedFilterString;
}

void ContactPrivate::changed()
{
   m_CachedFilterString.clear();
   foreach (Contact* c,m_lParents) {
      emit c->changed();
   }
}

void ContactPrivate::presenceChanged( PhoneNumber* n )
{
   foreach (Contact* c,m_lParents) {
      emit c->presenceChanged(n);
   }
}

void ContactPrivate::statusChanged  ( bool s )
{
   foreach (Contact* c,m_lParents) {
      emit c->statusChanged(s);
   }
}

void ContactPrivate::phoneNumberCountChanged(int n,int o)
{
   foreach (Contact* c,m_lParents) {
      emit c->phoneNumberCountChanged(n,o);
   }
}

void ContactPrivate::phoneNumberCountAboutToChange(int n,int o)
{
   foreach (Contact* c,m_lParents) {
      emit c->phoneNumberCountAboutToChange(n,o);
   }
}

ContactPrivate::ContactPrivate(Contact* contact, AbstractContactBackend* parent) :
   m_Numbers(contact),m_DisplayPhoto(false),m_Active(true),
   m_pBackend(parent?parent:TransitionalContactBackend::instance())
{}

ContactPrivate::~ContactPrivate()
{
}

Contact::PhoneNumbers::PhoneNumbers(Contact* parent) : QVector<PhoneNumber*>(),CategorizedCompositeNode(CategorizedCompositeNode::Type::NUMBER),
    m_pParent2(parent)
{
}

Contact::PhoneNumbers::PhoneNumbers(Contact* parent, const QVector<PhoneNumber*>& list)
: QVector<PhoneNumber*>(list),CategorizedCompositeNode(CategorizedCompositeNode::Type::NUMBER),m_pParent2(parent)
{
}

Contact* Contact::PhoneNumbers::contact() const
{
   return m_pParent2;
}

///Constructor
Contact::Contact(AbstractContactBackend* parent):QObject(parent?parent:TransitionalContactBackend::instance()),
   d_ptr(new ContactPrivate(this,parent))
{
   d_ptr->m_isPlaceHolder = false;
   d_ptr->m_lParents << this;
}

///Destructor
Contact::~Contact()
{
   //Unregister itself from the D-Pointer list
   d_ptr->m_lParents.removeAll(this);

   if (!d_ptr->m_lParents.size()) {
      delete d_ptr;
   }
}

///Get the phone number list
const Contact::PhoneNumbers& Contact::phoneNumbers() const
{
   return d_ptr->m_Numbers;
}

///Get the nickname
const QString& Contact::nickName() const
{
   return d_ptr->m_NickName;
}

///Get the firstname
const QString& Contact::firstName() const
{
   return d_ptr->m_FirstName;
}

///Get the second/family name
const QString& Contact::secondName() const
{
   return d_ptr->m_SecondName;
}

///Get the photo
const QVariant Contact::photo() const
{
   return d_ptr->m_vPhoto;
}

///Get the formatted name
const QString& Contact::formattedName() const
{
   return d_ptr->m_FormattedName;
}

///Get the organisation
const QString& Contact::organization()  const
{
   return d_ptr->m_Organization;
}

///Get the preferred email
const QString& Contact::preferredEmail()  const
{
   return d_ptr->m_PreferredEmail;
}

///Get the unique identifier (used for drag and drop) 
const QByteArray& Contact::uid() const
{
   return d_ptr->m_Uid;
}

///Get the group
const QString& Contact::group() const
{
   return d_ptr->m_Group;
}

const QString& Contact::department() const
{
   return d_ptr->m_Department;
}

///Set the phone number (type and number)
void Contact::setPhoneNumbers(PhoneNumbers numbers)
{
   const int oldCount(d_ptr->m_Numbers.size()),newCount(numbers.size());
   foreach(PhoneNumber* n, d_ptr->m_Numbers)
      disconnect(n,SIGNAL(presentChanged(bool)),this,SLOT(slotPresenceChanged()));
   d_ptr->m_Numbers = numbers;
   if (newCount < oldCount) //Rows need to be removed from models first
      d_ptr->phoneNumberCountAboutToChange(newCount,oldCount);
   foreach(PhoneNumber* n, d_ptr->m_Numbers)
      connect(n,SIGNAL(presentChanged(bool)),this,SLOT(slotPresenceChanged()));
   if (newCount > oldCount) //Need to be updated after the data to prevent invalid memory access
      d_ptr->phoneNumberCountChanged(newCount,oldCount);
   d_ptr->changed();
}

///Set the nickname
void Contact::setNickName(const QString& name)
{
   d_ptr->m_NickName = name;
   d_ptr->changed();
}

///Set the first name
void Contact::setFirstName(const QString& name)
{
   d_ptr->m_FirstName = name;
   setObjectName(formattedName());
   d_ptr->changed();
}

///Set the family name
void Contact::setFamilyName(const QString& name)
{
   d_ptr->m_SecondName = name;
   setObjectName(formattedName());
   d_ptr->changed();
}

///Set the Photo/Avatar
void Contact::setPhoto(const QVariant& photo)
{
   d_ptr->m_vPhoto = photo;
   d_ptr->changed();
}

///Set the formatted name (display name)
void Contact::setFormattedName(const QString& name)
{
   d_ptr->m_FormattedName = name;
   d_ptr->changed();
}

///Set the organisation / business
void Contact::setOrganization(const QString& name)
{
   d_ptr->m_Organization = name;
   d_ptr->changed();
}

///Set the default email
void Contact::setPreferredEmail(const QString& name)
{
   d_ptr->m_PreferredEmail = name;
   d_ptr->changed();
}

///Set UID
void Contact::setUid(const QByteArray& id)
{
   d_ptr->m_Uid = id;
   d_ptr->changed();
}

///Set Group
void Contact::setGroup(const QString& name)
{
   d_ptr->m_Group = name;
   d_ptr->changed();
}

///Set department
void Contact::setDepartment(const QString& name)
{
   d_ptr->m_Department = name;
   d_ptr->changed();
}

///If the contact have been deleted or not yet fully created
void Contact::setActive( bool active)
{
   d_ptr->m_Active = active;
   d_ptr->statusChanged(d_ptr->m_Active);
   d_ptr->changed();
}

///Return if one of the PhoneNumber is present
bool Contact::isPresent() const
{
   foreach(const PhoneNumber* n,d_ptr->m_Numbers) {
      if (n->isPresent())
         return true;
   }
   return false;
}

///Return if one of the PhoneNumber is tracked
bool Contact::isTracked() const
{
   foreach(const PhoneNumber* n,d_ptr->m_Numbers) {
      if (n->isTracked())
         return true;
   }
   return false;
}

///Have this contact been deleted or doesn't exist yet
bool Contact::isActive() const
{
   return d_ptr->m_Active;
}

///Return if one of the PhoneNumber support presence
bool Contact::supportPresence() const
{
   foreach(const PhoneNumber* n,d_ptr->m_Numbers) {
      if (n->supportPresence())
         return true;
   }
   return false;
}


QObject* Contact::PhoneNumbers::getSelf() const {
   return m_pParent2;
}

time_t Contact::PhoneNumbers::lastUsedTimeStamp() const
{
   time_t t = 0;
   for (int i=0;i<size();i++) {
      if (at(i)->lastUsed() > t)
         t = at(i)->lastUsed();
   }
   return t;
}

///Recomputing the filter string is heavy, cache it
QString Contact::filterString() const
{
   return d_ptr->filterString();
}

///Callback when one of the phone number presence change
void Contact::slotPresenceChanged()
{
   d_ptr->changed();
}

///Save the contact
bool Contact::save() const
{
   return d_ptr->m_pBackend->save(this);
}

///Show an implementation dependant dialog to edit the contact
bool Contact::edit()
{
   return d_ptr->m_pBackend->edit(this);
}

///Remove the contact from the backend
bool Contact::remove()
{
   return d_ptr->m_pBackend->remove(this);
}

///Add a new phone number to the backend
///@note The backend is expected to notify the Contact (asynchronously) when done
bool Contact::addPhoneNumber(PhoneNumber* n)
{
   return d_ptr->m_pBackend->addPhoneNumber(this,n);
}

///Create a placeholder contact, it will eventually be replaced when the real one is loaded
ContactPlaceHolder::ContactPlaceHolder(const QByteArray& uid)
{
   setUid(uid);
   d_ptr->m_isPlaceHolder = true;
}

/**
 * Sometime, items will use contacts before they are loaded.
 *
 * Once loaded, those pointers need to be upgraded to the real contact.
 */
bool ContactPlaceHolder::merge(Contact* contact)
{
   if ((!contact) || ((*contact) == this))
      return false;

   ContactPrivate* currentD = d_ptr;
   replaceDPointer(contact);
   currentD->m_lParents.removeAll(this);
   if (!currentD->m_lParents.size())
      delete currentD;
   return true;
}

void Contact::replaceDPointer(Contact* c)
{
   this->d_ptr = c->d_ptr;
   d_ptr->m_lParents << this;
   emit changed();
   emit rebased(c);
}

bool Contact::operator==(const Contact* other) const
{
   return other && this->d_ptr == other->d_ptr;
}

bool Contact::operator==(const Contact& other) const
{
   return &other && this->d_ptr == other.d_ptr;
}

///Add a new address to this contact
void Contact::addAddress(Contact::Address* addr)
{
   d_ptr->m_lAddresses << addr;
}

///Add custom fields for contact profiles
void Contact::addCustomField(const QString& key, const QString& value)
{
   d_ptr->m_lCustomAttributes.insert(key, value);
}

const QByteArray Contact::toVCard(QList<Account*> accounts) const
{
   //serializing here
   VCardUtils* maker = new VCardUtils();
   maker->startVCard("2.1");
   maker->addProperty(VCardUtils::Property::UID, uid());
   maker->addProperty(VCardUtils::Property::NAME, QString(secondName()
                                                   + VCardUtils::Delimiter::SEPARATOR_TOKEN
                                                   + firstName()));
   maker->addProperty(VCardUtils::Property::FORMATTED_NAME, formattedName());
   maker->addProperty(VCardUtils::Property::ORGANIZATION, organization());

   maker->addEmail("PREF", preferredEmail());

   foreach (PhoneNumber* phone , phoneNumbers()) {
      maker->addPhoneNumber(phone->category()->name(), phone->uri());
   }

   foreach (Address* addr , d_ptr->m_lAddresses) {
      maker->addAddress(addr);
   }

   foreach (const QString& key , d_ptr->m_lCustomAttributes.keys()) {
      maker->addProperty(key, d_ptr->m_lCustomAttributes.value(key));
   }

   foreach (Account* acc , accounts) {
      maker->addProperty(VCardUtils::Property::X_RINGACCOUNT, acc->id());
   }

   maker->addPhoto(PixmapManipulationVisitor::instance()->toByteArray(photo()).simplified());
   return maker->endVCard();
}
