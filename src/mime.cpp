/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include "mime.h"

//Qt
#include <QtCore/QMimeData>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>

//LRC
#include <call.h>
#include <person.h>
#include <contactmethod.h>
#include <numbercategory.h>

/**
 * This method enable inter-operability with various third party applications
 * by converting Ring objects into various standard MIME representations.
 *
 * This is designed to be an unified algorithm for drag and drop and copy and
 * paste. It replace no less than 17 chunks of code (LRC + KDE client) that
 * either all did the same thing or were missing parts.
 *
 * This methods need to be accessed directly in same specific scenarios,
 * otherwise, helper methods "mimePayload() const" are available for
 * Call, Person and ContactMethod.
 *
 * @note If new MIME are ever to be supported, this algorithm has to be
 * updated instead of hardcoding the payload in a random file.
 */
QMimeData* RingMimes::payload(const Call* c, const ContactMethod* cm, const Person* p)
{
   if (!c && !cm && !p)
      return nullptr;

   // Deduce other objects where applicable
   if (c && !cm)
      cm = c->peerContactMethod();

   if (!cm && p && p->phoneNumbers().size() == 1)
      cm = p->phoneNumbers().first();

   if (!p && cm->contact())
      p  = cm->contact();

   QMimeData* d = new QMimeData();

   // Person
   if (p) {

      // Ring interface contact identifier
      d->setData(RingMimes::CONTACT, p->uid());

      // Serialise as vCard for PIM applications
      const QByteArray vcf = p->toVCard();

      // A quick search and tests show those are common vard MIMEs
      d->setData(RingMimes::VCF    , vcf );
      d->setData(RingMimes::XVCF   , vcf );
      d->setData(RingMimes::MAC_VCF, vcf );

   }

   // URI list for other soft phones are SIP aware applications
   if (cm) {

      // This is the format used by URI list I found
      d->setData(RingMimes::URI_LIST, cm->uri().full().toUtf8());

   }
   else if (p) {
      QString     uriList;
      QTextStream stream(&uriList);

      foreach (const ContactMethod* number, p->phoneNumbers()) {
         stream << number->uri().format(
            URI::Section::SCHEME    |
            URI::Section::USER_INFO |
            URI::Section::HOSTNAME  |
            URI::Section::PORT
         ) << QChar('\n');
      }

      d->setData(RingMimes::URI_LIST, uriList.toUtf8());
   }

   // Allow conference and transfer by drag and drop
   if (c) {

      if (c->type() == Call::Type::CALL && c->hasRemote())
         d->setData(RingMimes::CALLID   , c->dringId().toLatin1() );
      else if (c->type() == Call::Type::HISTORY)
         d->setData(RingMimes::HISTORYID, c->dringId().toLatin1() );

      if (cm)
         d->setData(RingMimes::PHONENUMBER, cm->toHash().toUtf8());

   }

   // The "default" plain text and HTML representation
   if (c) {

      // The photo could be embdeed in the HTML as BASE64 JPG
      d->setData(RingMimes::HTML_TEXT , QString(
            "<p>\n"
            "    <b>%1</b><br />\n" /* Name */
            "    %2<br />\n"        /* URI  */
            "    <i>%3</i><br />\n" /* Time */
            "</p>"
         )
         .arg( c->formattedName()                                    )
         .arg( cm->uri()                                             )
         .arg( QDateTime::fromTime_t(c->startTimeStamp()).toString() )
         .toUtf8()
      );

      d->setData(RingMimes::PLAIN_TEXT, QString("%1\n%2\n%3\n")
      .arg( c->formattedName()                                    )
      .arg( c->peerContactMethod()->uri()                         )
      .arg( QDateTime::fromTime_t(c->startTimeStamp()).toString() )
      .toUtf8() );

   }
   else if (cm) {
      d->setData(RingMimes::PLAIN_TEXT, cm->uri().toUtf8() );
   }
   else if (p) {
      QString     html             , text;
      QTextStream htmlStream(&html), textStream(&text);

      // Contact name
      htmlStream << QString("<p>\n"
      "    <b>%1</b><br />\n").arg(p->formattedName());

      // Phone numbers (as saved in the contacts)
      foreach (const ContactMethod* number, p->phoneNumbers()) {

         htmlStream << number->uri()
            << QStringLiteral(" (")
            << number->category()->name()
            << QStringLiteral(")  <br />\n");

         textStream << number->uri() << QChar('\n');
      }

      htmlStream << QStringLiteral("</p>");

      d->setData(RingMimes::PLAIN_TEXT, text.toUtf8() );
      d->setData(RingMimes::HTML_TEXT , html.toUtf8() );

   }

   return d;
}
