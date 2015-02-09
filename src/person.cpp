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
#include "person.h"

//Ring library
#include "contactmethod.h"
#include "collectioninterface.h"
#include "transitionalpersonbackend.h"
#include "account.h"
#include "vcardutils.h"
#include "numbercategorymodel.h"
#include "numbercategory.h"
#include "delegates/pixmapmanipulationdelegate.h"


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

Person::Address::Address() : d_ptr(new AddressPrivate())
{

}

QString Person::Address::addressLine() const
{
   return d_ptr->addressLine;
}

QString Person::Address::city() const
{
   return d_ptr->city;
}

QString Person::Address::zipCode() const
{
   return d_ptr->zipCode;
}

QString Person::Address::state() const
{
   return d_ptr->state;
}

QString Person::Address::country() const
{
   return d_ptr->country;
}

QString Person::Address::type() const
{
   return d_ptr->type;
}

void Person::Address::setAddressLine(const QString& value)
{
   d_ptr->addressLine = value;
}

void Person::Address::setCity(const QString& value)
{
   d_ptr->city = value;
}

void Person::Address::setZipCode(const QString& value)
{
   d_ptr->zipCode = value;
}

void Person::Address::setState(const QString& value)
{
   d_ptr->state = value;
}

void Person::Address::setCountry(const QString& value)
{
   d_ptr->country = value;
}

void Person::Address::setType(const QString& value)
{
   d_ptr->type = value;
}

class PersonPrivate {
public:
   PersonPrivate(Person* contact);
   ~PersonPrivate();
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
   Person::ContactMethods    m_Numbers          ;
   bool                     m_Active           ;
   bool                     m_isPlaceHolder    ;
   QList<Person::Address*> m_lAddresses       ;
   QHash<QString, QString>  m_lCustomAttributes;

   //Cache
   QString m_CachedFilterString;

   QString filterString();

   //Helper code to help handle multiple parents
   QList<Person*> m_lParents;

   //As a single D-Pointer can have multiple parent (when merged), all emit need
   //to use a proxy to make sure everybody is notified
   void presenceChanged( ContactMethod* );
   void statusChanged  ( bool         );
   void changed        (              );
   void phoneNumberCountChanged(int,int);
   void phoneNumberCountAboutToChange(int,int);
};

QString PersonPrivate::filterString()
{
   if (m_CachedFilterString.size())
      return m_CachedFilterString;

   //Also filter by phone numbers, accents are negligible
   foreach(const ContactMethod* n , m_Numbers) {
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

void PersonPrivate::changed()
{
   m_CachedFilterString.clear();
   foreach (Person* c,m_lParents) {
      emit c->changed();
   }
}

void PersonPrivate::presenceChanged( ContactMethod* n )
{
   foreach (Person* c,m_lParents) {
      emit c->presenceChanged(n);
   }
}

void PersonPrivate::statusChanged  ( bool s )
{
   foreach (Person* c,m_lParents) {
      emit c->statusChanged(s);
   }
}

void PersonPrivate::phoneNumberCountChanged(int n,int o)
{
   foreach (Person* c,m_lParents) {
      emit c->phoneNumberCountChanged(n,o);
   }
}

void PersonPrivate::phoneNumberCountAboutToChange(int n,int o)
{
   foreach (Person* c,m_lParents) {
      emit c->phoneNumberCountAboutToChange(n,o);
   }
}

PersonPrivate::PersonPrivate(Person* contact) :
   m_Numbers(contact),m_DisplayPhoto(false),m_Active(true)
{
}

PersonPrivate::~PersonPrivate()
{
}

Person::ContactMethods::ContactMethods(Person* parent) : QVector<ContactMethod*>(),CategorizedCompositeNode(CategorizedCompositeNode::Type::NUMBER),
    m_pParent2(parent)
{
}

Person::ContactMethods::ContactMethods(Person* parent, const QVector<ContactMethod*>& list)
: QVector<ContactMethod*>(list),CategorizedCompositeNode(CategorizedCompositeNode::Type::NUMBER),m_pParent2(parent)
{
}

Person* Person::ContactMethods::contact() const
{
   return m_pParent2;
}

///Constructor
Person::Person(CollectionInterface* parent): ItemBase<QObject>(parent?parent->model():TransitionalPersonBackend::instance()->model()),
   d_ptr(new PersonPrivate(this))
{
   setBackend(parent?parent:TransitionalPersonBackend::instance());
   d_ptr->m_isPlaceHolder = false;
   d_ptr->m_lParents << this;
}

///Destructor
Person::~Person()
{
   //Unregister itself from the D-Pointer list
   d_ptr->m_lParents.removeAll(this);

   if (!d_ptr->m_lParents.size()) {
      delete d_ptr;
   }
}

///Get the phone number list
const Person::ContactMethods& Person::phoneNumbers() const
{
   return d_ptr->m_Numbers;
}

///Get the nickname
const QString& Person::nickName() const
{
   return d_ptr->m_NickName;
}

///Get the firstname
const QString& Person::firstName() const
{
   return d_ptr->m_FirstName;
}

///Get the second/family name
const QString& Person::secondName() const
{
   return d_ptr->m_SecondName;
}

///Get the photo
const QVariant Person::photo() const
{
   return d_ptr->m_vPhoto;
}

///Get the formatted name
const QString& Person::formattedName() const
{
   return d_ptr->m_FormattedName;
}

///Get the organisation
const QString& Person::organization()  const
{
   return d_ptr->m_Organization;
}

///Get the preferred email
const QString& Person::preferredEmail()  const
{
   return d_ptr->m_PreferredEmail;
}

///Get the unique identifier (used for drag and drop) 
const QByteArray& Person::uid() const
{
   return d_ptr->m_Uid;
}

///Get the group
const QString& Person::group() const
{
   return d_ptr->m_Group;
}

const QString& Person::department() const
{
   return d_ptr->m_Department;
}

///Set the phone number (type and number)
void Person::setContactMethods(ContactMethods numbers)
{
   const int oldCount(d_ptr->m_Numbers.size()),newCount(numbers.size());
   foreach(ContactMethod* n, d_ptr->m_Numbers)
      disconnect(n,SIGNAL(presentChanged(bool)),this,SLOT(slotPresenceChanged()));
   d_ptr->m_Numbers = numbers;
   if (newCount < oldCount) //Rows need to be removed from models first
      d_ptr->phoneNumberCountAboutToChange(newCount,oldCount);
   foreach(ContactMethod* n, d_ptr->m_Numbers)
      connect(n,SIGNAL(presentChanged(bool)),this,SLOT(slotPresenceChanged()));
   if (newCount > oldCount) //Need to be updated after the data to prevent invalid memory access
      d_ptr->phoneNumberCountChanged(newCount,oldCount);
   d_ptr->changed();
}

///Set the nickname
void Person::setNickName(const QString& name)
{
   d_ptr->m_NickName = name;
   d_ptr->changed();
}

///Set the first name
void Person::setFirstName(const QString& name)
{
   d_ptr->m_FirstName = name;
   setObjectName(formattedName());
   d_ptr->changed();
}

///Set the family name
void Person::setFamilyName(const QString& name)
{
   d_ptr->m_SecondName = name;
   setObjectName(formattedName());
   d_ptr->changed();
}

///Set the Photo/Avatar
void Person::setPhoto(const QVariant& photo)
{
   d_ptr->m_vPhoto = photo;
   d_ptr->changed();
}

///Set the formatted name (display name)
void Person::setFormattedName(const QString& name)
{
   d_ptr->m_FormattedName = name;
   d_ptr->changed();
}

///Set the organisation / business
void Person::setOrganization(const QString& name)
{
   d_ptr->m_Organization = name;
   d_ptr->changed();
}

///Set the default email
void Person::setPreferredEmail(const QString& name)
{
   d_ptr->m_PreferredEmail = name;
   d_ptr->changed();
}

///Set UID
void Person::setUid(const QByteArray& id)
{
   d_ptr->m_Uid = id;
   d_ptr->changed();
}

///Set Group
void Person::setGroup(const QString& name)
{
   d_ptr->m_Group = name;
   d_ptr->changed();
}

///Set department
void Person::setDepartment(const QString& name)
{
   d_ptr->m_Department = name;
   d_ptr->changed();
}

///If the contact have been deleted or not yet fully created
void Person::setActive( bool active)
{
   d_ptr->m_Active = active;
   d_ptr->statusChanged(d_ptr->m_Active);
   d_ptr->changed();
}

///Return if one of the ContactMethod is present
bool Person::isPresent() const
{
   foreach(const ContactMethod* n,d_ptr->m_Numbers) {
      if (n->isPresent())
         return true;
   }
   return false;
}

///Return if one of the ContactMethod is tracked
bool Person::isTracked() const
{
   foreach(const ContactMethod* n,d_ptr->m_Numbers) {
      if (n->isTracked())
         return true;
   }
   return false;
}

///Have this contact been deleted or doesn't exist yet
bool Person::isActive() const
{
   return d_ptr->m_Active;
}

///Return if one of the ContactMethod support presence
bool Person::supportPresence() const
{
   foreach(const ContactMethod* n,d_ptr->m_Numbers) {
      if (n->supportPresence())
         return true;
   }
   return false;
}


QObject* Person::ContactMethods::getSelf() const {
   return m_pParent2;
}

time_t Person::ContactMethods::lastUsedTimeStamp() const
{
   time_t t = 0;
   for (int i=0;i<size();i++) {
      if (at(i)->lastUsed() > t)
         t = at(i)->lastUsed();
   }
   return t;
}

///Recomputing the filter string is heavy, cache it
QString Person::filterString() const
{
   return d_ptr->filterString();
}

///Callback when one of the phone number presence change
void Person::slotPresenceChanged()
{
   d_ptr->changed();
}

///Create a placeholder contact, it will eventually be replaced when the real one is loaded
PersonPlaceHolder::PersonPlaceHolder(const QByteArray& uid)
{
   setUid(uid);
   d_ptr->m_isPlaceHolder = true;
}

/**
 * Sometime, items will use contacts before they are loaded.
 *
 * Once loaded, those pointers need to be upgraded to the real contact.
 */
bool PersonPlaceHolder::merge(Person* contact)
{
   if ((!contact) || ((*contact) == this))
      return false;

   PersonPrivate* currentD = d_ptr;
   replaceDPointer(contact);
   currentD->m_lParents.removeAll(this);
   if (!currentD->m_lParents.size())
      delete currentD;
   return true;
}

void Person::replaceDPointer(Person* c)
{
   this->d_ptr = c->d_ptr;
   d_ptr->m_lParents << this;
   emit changed();
   emit rebased(c);
}

bool Person::operator==(const Person* other) const
{
   return other && this->d_ptr == other->d_ptr;
}

bool Person::operator==(const Person& other) const
{
   return &other && this->d_ptr == other.d_ptr;
}

///Add a new address to this contact
void Person::addAddress(Person::Address* addr)
{
   d_ptr->m_lAddresses << addr;
}

///Add custom fields for contact profiles
void Person::addCustomField(const QString& key, const QString& value)
{
   d_ptr->m_lCustomAttributes.insert(key, value);
}

const QByteArray Person::toVCard(QList<Account*> accounts) const
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

   foreach (ContactMethod* phone , phoneNumbers()) {
      maker->addContactMethod(phone->category()->name(), phone->uri());
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

   maker->addPhoto(PixmapManipulationDelegate::instance()->toByteArray(photo()).simplified());
   return maker->endVCard();
}
