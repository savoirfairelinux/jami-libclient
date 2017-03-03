/****************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                               *
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

// Std
#include <memory>

//Qt
#include <QtCore/QObject>
#include <QtCore/QHash>

//Ring
#include "account.h"
#include "private/matrixutils.h"
#include "credentialmodel.h"
#include "codecmodel.h"
#include "keyexchangemodel.h"
#include "ciphermodel.h"
#include "accountstatusmodel.h"
#include "securityevaluationmodel.h"
#include "tlsmethodmodel.h"
#include "protocolmodel.h"
#include "bootstrapmodel.h"
#include "ringdevicemodel.h"
#include "networkinterfacemodel.h"
#include "pendingtrustrequestmodel.h"

class AccountPrivate;
class ContactMethod;
class CipherModel;
class AccountStatusModel;
class TlsMethodModel;
class ProtocolModel;
class NetworkInterfaceModel;
class BootstrapModel;
class DaemonCertificateCollection;
class PendingTrustRequestModel;
class Profile;

typedef void (AccountPrivate::*account_function)();

class AccountPrivate final : public QObject
{
public:
   Q_OBJECT
   Q_DECLARE_PUBLIC(Account)

   class RegistrationEnabled {
      public:
         constexpr static const char* YES  = "true";
         constexpr static const char* NO   = "false";
   };

   friend class AccountPlaceHolder;
   friend class AccountModel;
   friend class AccountModelPrivate;
   friend class CipherModelPrivate;
   friend class CipherModel;
   friend class AccountStatusModelPrivate;
   friend class AccountStatusModel;
   friend class TlsMethodModelPrivate;
   friend class TlsMethodModel;
   friend class BootstrapModelPrivate;
   friend class KeyExchangeModelPrivate;
   friend class ContactMethod;
   friend class Certificate;
   friend class NetworkInterfaceModelPrivate;
   friend class CredentialModelPrivate;

   //Constructor
   explicit AccountPrivate(Account* acc);

   //Attributes
   QByteArray                 m_AccountId                ;
   QHash<QString,QString>     m_hAccountDetails          ;
   ContactMethod*             m_pAccountNumber           ;
   Account*                   q_ptr                      ;
   bool                       m_isLoaded                 ;
   int                        m_LastTransportCode        ;
   QString                    m_LastTransportMessage     ;
   Account::RegistrationState m_RegistrationState        ;
   QString                    m_LastSipRegistrationStatus;
   unsigned short             m_UseDefaultPort           ;
   bool                       m_RemoteEnabledState       ;
   uint                       m_InternalId               ;

   //Statistic
   bool   m_HaveCalled    ;
   uint   m_TotalCount    ;
   uint   m_LastWeekCount ;
   uint   m_LastTrimCount ;
   time_t m_LastUsed      ;

   //Setters
   void setAccountProperties(const QHash<QString,QString>& m          );
   bool setAccountProperty  (const QString& param, const QString& val );

   //Getters
   const QString accountDetail(const QString& param) const;
   uint internalId() const;

   //Mutator
   bool merge(Account* account);
   //Constructors
   static Account* buildExistingAccountFromId(const QByteArray& _accountId);
   static Account* buildNewAccountFromAlias  (Account::Protocol proto, const QString& alias);

   //Helpers
   inline void changeState(Account::EditState state);
   bool updateState();
   void regenSecurityValidation();

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
   void reloadMod() {reload();modify();}

   std::unique_ptr<CredentialModel> m_pCredentials;
   std::unique_ptr<CodecModel> m_pCodecModel;
   std::unique_ptr<KeyExchangeModel> m_pKeyExchangeModel;
   std::unique_ptr<CipherModel> m_pCipherModel;
   std::unique_ptr<AccountStatusModel> m_pStatusModel;
   std::unique_ptr<SecurityEvaluationModel> m_pSecurityEvaluationModel;
   std::unique_ptr<TlsMethodModel> m_pTlsMethodModel;
   std::unique_ptr<ProtocolModel> m_pProtocolModel;
   std::unique_ptr<BootstrapModel> m_pBootstrapModel;
   std::unique_ptr<RingDeviceModel> m_pRingDeviceModel;
   QAbstractItemModel*          m_pKnownCertificates;
   QAbstractItemModel*          m_pBannedCertificates;
   QAbstractItemModel*          m_pAllowedCertificates;
   std::unique_ptr<NetworkInterfaceModel> m_pNetworkInterfaceModel;
   DaemonCertificateCollection* m_pAllowedCerts           ;
   DaemonCertificateCollection* m_pBannedCerts            ;
   Account::EditState           m_CurrentState            ;
   QMetaObject::Connection      m_cTlsCert                ;
   QMetaObject::Connection      m_cTlsCaCert              ;
   Profile*                     m_pProfile {nullptr}      ;
   std::unique_ptr<PendingTrustRequestModel> m_pPendingTrustRequestModel;

   QHash<int, Account::RoleStatus> m_hRoleStatus;

   // State machines
   static const Matrix2D<Account::EditState, Account::EditAction, account_function> stateMachineActionsOnState;

   //Cached account details (as they are called too often for the hash)
   mutable QString      m_HostName;
   mutable QString      m_LastErrorMessage;
   mutable int          m_LastErrorCode;
   mutable int          m_VoiceMailCount;
   mutable Certificate* m_pCaCert;
   mutable Certificate* m_pTlsCert;

public Q_SLOTS:
      void slotPresentChanged        (bool  present  );
      void slotPresenceMessageChanged(const QString& );
      void slotUpdateCertificate     (               );
};
