/****************************************************************************
 *   Copyright (C) 2013-2016 by Savoir-faire Linux                          *
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
#pragma once

//Internal
#include "usage_statistics.h"

class ContactMethodPrivate {
public:
   ContactMethodPrivate(const URI& number, NumberCategory* cat, ContactMethod::Type st,
                        ContactMethod* q);
   NumberCategory*    m_pCategory        ;
   bool               m_Present          ;
   QString            m_PresentMessage   ;
   bool               m_Tracked          ;
   Person*            m_pPerson          ;
   Account*           m_pAccount         ;
   QList<Call*>       m_lCalls           ;
   int                m_PopularityIndex  ;
   QString            m_MostCommonName   ;
   QHash<QString,QPair<int,time_t>> m_hNames;
   bool               m_hasType          ;
   int                m_Index            ;
   bool               m_IsBookmark       ;
   QString            m_Uid              ;
   QString            m_PrimaryName_cache;
   URI                m_Uri              ;
   QByteArray         m_Sha1             ;
   ContactMethod::Type  m_Type           ;
   QList<URI>         m_lOtherURIs       ;
   Media::TextRecording* m_pTextRecording;
   Certificate*       m_pCertificate;
   QString            m_RegisteredName   ;
   UsageStatistics    m_UsageStats       ;
   bool               m_Trusted          ;

   //Parents
   QList<ContactMethod*> m_lParents;

   //Emit proxies
   void callAdded(Call* call);
   void changed  (          );
   void presentChanged(bool);
   void presenceMessageChanged(const QString&);
   void trackedChanged(bool);
   void primaryNameChanged(const QString& name);
   void rebased(ContactMethod* other);
   void registeredNameSet(const QString& registeredName);

   //Helpers
   void setTextRecording(Media::TextRecording* r);
   void setCertificate (Certificate*);
   void setRegisteredName(const QString& registeredName);

 private:
   ContactMethod* q_ptr;
};
