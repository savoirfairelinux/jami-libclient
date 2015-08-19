/****************************************************************************
 *   Copyright (C) 2009-2014 by Savoir-Faire Linux                          *
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
#ifndef PERSON_P_H
#define PERSON_P_H

#include <QtCore/QObject>

//std
#include <time.h>

//Ring
#include "person.h"
class ContactMethod;

class PersonPrivate : public QObject
{
   Q_OBJECT
   friend class ContactMethod;
public:
   PersonPrivate(Person* contact);
   ~PersonPrivate();
   QString                  m_FirstName           ;
   QString                  m_SecondName          ;
   QString                  m_NickName            ;
   QVariant                 m_vPhoto              ;
   QString                  m_FormattedName       ;
   QString                  m_PreferredEmail      ;
   QString                  m_Organization        ;
   QByteArray               m_Uid                 ;
   QString                  m_Group               ;
   QString                  m_Department          ;
   bool                     m_DisplayPhoto        ;
   Person::ContactMethods   m_Numbers             ;
   bool                     m_Active              ;
   bool                     m_isPlaceHolder       ;
   QList<Person::Address>   m_lAddresses          ;
   QHash<QString, QString>  m_lCustomAttributes   ;
   ::time_t                 m_LastUsed            ;
   bool                     m_LastUsedInit        ;
   QList<ContactMethod*>    m_HiddenContactMethods;

   //Cache
   QString m_CachedFilterString;

   QString filterString();

   //Helper code to help handle multiple parents
   QList<Person*> m_lParents;
   Person* q_ptr;

   //As a single D-Pointer can have multiple parent (when merged), all emit need
   //to use a proxy to make sure everybody is notified
   void presenceChanged          ( ContactMethod* );
   void statusChanged            ( bool           );
   void changed                  (                );
   void phoneNumbersChanged      (                );
   void phoneNumbersAboutToChange(                );

   //Helper
   void registerContactMethod(ContactMethod* m);

public Q_SLOTS:
   void slotLastUsedTimeChanged(::time_t t);
};

#endif
