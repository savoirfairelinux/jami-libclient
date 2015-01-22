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
#include <QBuffer>

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
   if(value.isEmpty() || value == ";")
      return;
   m_vCard << QString(QString::fromUtf8(prop) + ":" + value);
}

void VCardUtils::addProperty(const QString& prop, const QString& value)
{
   if(value.isEmpty() || value == ";")
      return;
   m_vCard << QString(prop + ":" + value);
}

void VCardUtils::addEmail(const QString& type, const QString& email)
{
   addProperty(QString("%1%2%3%4").arg(Property::EMAIL).arg(Delimiter::SEPARATOR_TOKEN).arg("TYPE=").arg(type), email);
}

void VCardUtils::addAddress(const Contact::Address* addr)
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

void VCardUtils::addPhoneNumber(const QString& type, const QString& num)
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
   m_vCard << QString(QString::fromUtf8(Property::PHOTO) +
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
