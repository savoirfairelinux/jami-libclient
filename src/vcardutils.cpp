/****************************************************************************
 *   Copyright (C) 2013-2014 by Savoir-Faire Linux                           *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com> *
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

#include "vcardutils.h"

//Qt
#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QUrl>

//Ring
#include "phonedirectorymodel.h"
#include "contactmethod.h"
#include "accountmodel.h"
#include "delegates/pixmapmanipulationdelegate.h"

/* https://www.ietf.org/rfc/rfc2045.txt
 * https://www.ietf.org/rfc/rfc2047.txt
 * https://www.ietf.org/rfc/rfc2426.txt
 */

//BEGIN:VCARD
//VERSION:3.0
//N:Gump;Forrest
//FN:Forrest Gump
//ORG:Bubba Gump Shrimp Co.
//TITLE:Shrimp Man
//PHOTO;VALUE=URL;TYPE=GIF:http://www.example.com/dir_photos/my_photo.gif
//TEL;TYPE=WORK,VOICE:(111) 555-1212
//TEL;TYPE=HOME,VOICE:(404) 555-1212
//ADR;TYPE=WORK:;;100 Waters Edge;Baytown;LA;30314;United States of America
//LABEL;TYPE=WORK:100 Waters Edge\nBaytown, LA 30314\nUnited States of America
//ADR;TYPE=HOME:;;42 Plantation St.;Baytown;LA;30314;United States of America
//LABEL;TYPE=HOME:42 Plantation St.\nBaytown, LA 30314\nUnited States of America
//EMAIL;TYPE=PREF,INTERNET:forrestgump@example.com
//REV:20080424T195243Z
//END:VCARD

struct VCardMapper;

typedef void (VCardMapper:: *mapToProperty)(Person*, const QString&, const QByteArray&);

struct VCardMapper {

   QHash<QByteArray, mapToProperty> m_hHash;

   VCardMapper() {
      m_hHash[VCardUtils::Property::UID] = &VCardMapper::setUid;
      m_hHash[VCardUtils::Property::NAME] = &VCardMapper::setNames;
      m_hHash[VCardUtils::Property::FORMATTED_NAME] = &VCardMapper::setFormattedName;
      m_hHash[VCardUtils::Property::EMAIL] = &VCardMapper::setEmail;
      m_hHash[VCardUtils::Property::ORGANIZATION] = &VCardMapper::setOrganization;
      m_hHash[VCardUtils::Property::TELEPHONE] = &VCardMapper::addContactMethod;
      m_hHash[VCardUtils::Property::ADDRESS] = &VCardMapper::addAddress;
      m_hHash[VCardUtils::Property::PHOTO] = &VCardMapper::setPhoto;
   }

   void setFormattedName(Person* c,  const QString&, const QByteArray& fn) {
      c->setFormattedName(QString::fromUtf8(fn));
   }

   void setNames(Person* c,  const QString&, const QByteArray& fn) {
      QList<QByteArray> splitted = fn.split(';');
      c->setFamilyName(splitted[0].trimmed());
      c->setFirstName(splitted[1].trimmed());
   }

   void setUid(Person* c,  const QString&, const QByteArray& fn) {
      c->setUid(fn);
   }

   void setEmail(Person* c,  const QString&, const QByteArray& fn) {
      c->setPreferredEmail(fn);
   }

   void setOrganization(Person* c, const QString&,  const QByteArray& fn) {
      c->setOrganization(QString::fromUtf8(fn));
   }

   void setPhoto(Person* c, const QString& key, const QByteArray& fn) {
      QByteArray type = "PNG";

      QRegExp rx("TYPE=([A-Za-z]*)");

      while ((rx.indexIn(key, 0)) != -1) {
         type = rx.cap(1).toLatin1();
         break;
      }

      QVariant photo = PixmapManipulationDelegate::instance()->profilePhoto(fn,type);
      c->setPhoto(photo);
   }

   void addContactMethod(Person* c, const QString& key, const QByteArray& fn) {
      Q_UNUSED(c)
      Q_UNUSED(key)
      QByteArray type;

      QRegExp rx("TYPE=([A-Za-z,]*)");

      while ((rx.indexIn(key, 0)) != -1) {
         type = rx.cap(1).toLatin1();
         break;
      }

      const QStringList categories = QString(type).split(',');

      ContactMethod* cm = PhoneDirectoryModel::instance()->getNumber(fn,c,nullptr,categories.size()?categories[0]:QString());
      Person::ContactMethods m = c->phoneNumbers();
      m << cm;
      c->setContactMethods(m);
   }

   void addAddress(Person* c, const QString& key, const QByteArray& fn) {
      Person::Address* addr = new Person::Address();
      QList<QByteArray> fields = fn.split(VCardUtils::Delimiter::SEPARATOR_TOKEN[0]);

      addr->setType        (key.split(VCardUtils::Delimiter::SEPARATOR_TOKEN)[1] );
      addr->setAddressLine (QString::fromUtf8(fields[2])                         );
      addr->setCity        (QString::fromUtf8(fields[3])                         );
      addr->setState       (QString::fromUtf8(fields[4])                         );
      addr->setZipCode     (QString::fromUtf8(fields[5])                         );
      addr->setCountry     (QString::fromUtf8(fields[6])                         );

      c->addAddress(addr);
   }

   bool metacall(Person* c, const QByteArray& key, const QByteArray& value) {
      const QStringList settings = QString(key).split(';');
      if (!m_hHash[settings[0].toLatin1()]) {
         if(key.contains(VCardUtils::Property::PHOTO)) {
            //key must contain additionnal attributes, we don't need them right now (ENCODING, TYPE...)
            setPhoto(c, key, value);
            return true;
         }

         if(key.contains(VCardUtils::Property::ADDRESS)) {
            addAddress(c, key, value);
            return true;
         }

         if(key.contains(VCardUtils::Property::TELEPHONE)) {
            addContactMethod(c, key, value);
            return true;
         }

         return false;
      }
      (this->*(m_hHash[settings[0].toLatin1()]))(c,key,value);
      return true;
   }
};
static VCardMapper* vc_mapper = new VCardMapper;

VCardUtils::VCardUtils()
{

}

void VCardUtils::startVCard(const QString& version)
{
   m_vCard << Delimiter::BEGIN_TOKEN;
   addProperty(Property::VERSION, version);
}

void VCardUtils::addProperty(const char* prop, const QString& value)
{
   if (value.isEmpty() || value == QString(';'))
      return;
   m_vCard << (QString::fromUtf8(prop) + ':' + value);
}

void VCardUtils::addProperty(const QString& prop, const QString& value)
{
   if (value.isEmpty() || value == QString(';'))
      return;
   m_vCard << (prop + ':' + value);
}

void VCardUtils::addEmail(const QString& type, const QString& email)
{
   addProperty(QString("%1%2%3%4").arg(Property::EMAIL).arg(Delimiter::SEPARATOR_TOKEN).arg("TYPE=").arg(type), email);
}

void VCardUtils::addAddress(const Person::Address* addr)
{
   QString prop = QString("%1%2%3").arg(Property::ADDRESS)
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(addr->type());

   //First two fiels are left empty for now, they are for Postal box and Extended Address
   QString value = QString("%1%2%3%4%5%6%7%8%9%10%11")
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(addr->addressLine())
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(addr->city())
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(addr->state())
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(addr->zipCode())
         .arg(Delimiter::SEPARATOR_TOKEN)
         .arg(addr->country());

   addProperty(prop, value);
}

void VCardUtils::addContactMethod(const QString& type, const QString& num)
{
   // This will need some formatting
   addProperty(Property::TELEPHONE, type + num);
//   char* prop = VCProperty::VC_TELEPHONE;
//   strcat(prop, VCDelimiter::VC_SEPARATOR_TOKEN);
//   strcat(prop, "TYPE=" + type);
//   strcat(prop, ",VOICE");
}

void VCardUtils::addPhoto(const QByteArray img)
{
   m_vCard << (QString::fromUtf8(Property::PHOTO) +
               QString::fromUtf8(Delimiter::SEPARATOR_TOKEN) +
               "ENCODING=BASE64" +
               QString::fromUtf8(Delimiter::SEPARATOR_TOKEN) +
               "TYPE=PNG:" +
               img.toBase64());
}

const QByteArray VCardUtils::endVCard()
{
   m_vCard << Delimiter::END_TOKEN;
   const QString result = m_vCard.join(QString::fromUtf8(Delimiter::END_LINE_TOKEN));
   return result.toUtf8();
}

QList< Person* > VCardUtils::loadDir (const QUrl& path, bool& ok, QHash<const Person*,QString>& paths)
{
   QList< Person* > ret;

   QDir dir(path.toString());
   if (!dir.exists())
      ok = false;
   else {
      ok = true;
      for (const QString& file : dir.entryList({"*.vcf"},QDir::Files)) {
         Person* p = new Person();
         mapToPerson(p,QUrl(dir.absoluteFilePath(file)));
         ret << p;
         paths[p] = dir.absoluteFilePath(file);
      }
   }

   return ret;
}

bool VCardUtils::mapToPerson(Person* p, const QUrl& path, Account** a)
{

   qDebug() << "file" << path.toString();

   QFile file(path.toString());
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "Error opening vcard: " << path;
      return false;
   }

   const QByteArray all = file.readAll();

//    bool propertyInserted = false;
   QByteArray previousKey,previousValue;
   for (const QByteArray& property : all.split('\n')) {

      //Ignore empty lines
      if (property.size()) {

         //Some properties are over multiple lines
         if (property[0] == ' ' && previousKey.size()) {
            previousValue += property.right(property.size()-1);
         }
         else {
            if (previousKey.size()) {
               /*propertyInserted = */vc_mapper->metacall(p,previousKey,previousValue.trimmed());

//                if(!propertyInserted)
//                   qDebug() << "Could not extract: " << previousKey;
            }

            const QList<QByteArray> splitted = property.split(':');
            if(splitted.size() < 2){
               qDebug() << "Malformed vCard property!" << splitted[0] << property[0] << (property[0] == ' ');
               continue;
            }


            //Link with accounts
            if(splitted[0] == VCardUtils::Property::X_RINGACCOUNT) {
               if (a) {
                  *a = AccountModel::instance()->getById(splitted[1].trimmed(),true);
                  if(!*a) {
                     qDebug() << "Could not find account: " << splitted[1].trimmed();
                     continue;
                  }
               }
            }

            previousKey   = splitted[0];
            previousValue = splitted[1];
         }

      }

   }
   return true;
}
