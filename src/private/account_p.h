/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#ifndef ACCOUNTPRIVATE_H
#define ACCOUNTPRIVATE_H

#include <QtCore/QObject>
#include <QtCore/QHash>
#include <account.h>

class AccountPrivate;
class PhoneNumber;

typedef void (AccountPrivate::*account_function)();

class AccountPrivate : public QObject
{
public:
   Q_OBJECT
   Q_DECLARE_PUBLIC(Account)

   friend class AccountPlaceHolder;
   friend class AccountModel;
   friend class AccountModelPrivate;

   //Constructor
   AccountPrivate(Account* acc);

   //Attributes
   QByteArray              m_AccountId      ;
   QHash<QString,QString>  m_hAccountDetails;
   PhoneNumber*            m_pAccountNumber ;
   Account*                q_ptr            ;
   bool                    m_isLoaded       ;

   //Setters
   void setAccountProperties(const QHash<QString,QString>& m          );
   bool setAccountProperty  (const QString& param, const QString& val );

   //Getters
   const QString accountDetail(const QString& param) const;

   //Mutator
   bool merge(Account* account);
   //Constructors
   static Account* buildExistingAccountFromId(const QByteArray& _accountId);
   static Account* buildNewAccountFromAlias  (const QString& alias        );

   //Helpers
   inline void changeState(Account::EditState state);
   bool updateState();

   //State actions
   void performAction(Account::EditAction action);
   void nothing();
   void edit   ();
   void modify ();
   void remove ();
   void cancel ();
   void outdate();
   void reload ();
   void save   ();
   void reloadMod() {reload();modify();};

   CredentialModel*          m_pCredentials     ;
   Audio::CodecModel*        m_pAudioCodecs     ;
   Video::CodecModel2*       m_pVideoCodecs     ;
   RingToneModel*            m_pRingToneModel   ;
   KeyExchangeModel*         m_pKeyExchangeModel;
   SecurityValidationModel*  m_pSecurityValidationModel;
   Account::EditState m_CurrentState;

   // State machines
   static const account_function stateMachineActionsOnState[6][7];

   //Cached account details (as they are called too often for the hash)
   mutable QString      m_HostName;
   mutable QString      m_LastErrorMessage;
   mutable int          m_LastErrorCode;
   mutable int          m_VoiceMailCount;
   mutable Certificate* m_pCaCert;
   mutable Certificate* m_pTlsCert;
   mutable Certificate* m_pPrivateKey;

public Q_SLOTS:
      void slotPresentChanged        (bool  present  );
      void slotPresenceMessageChanged(const QString& );
      void slotUpdateCertificate     (               );
};

#endif
