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

#ifndef ACCOUNT_H
#define ACCOUNT_H

#include <QtCore/QList>
#include <QtCore/QSharedPointer>

//Qt
class QString;

//Ring
#include "itembase.h"
#include "keyexchangemodel.h"
#include "tlsmethodmodel.h"
#include "uri.h"
#include "typedefs.h"
class CredentialModel         ;
class ContactMethod           ;
class SecurityEvaluationModel ;
class Certificate             ;
class CipherModel             ;
class AccountStatusModel      ;
class ProtocolModel           ;
class CodecModel              ;
class BootstrapModel          ;
class NetworkInterfaceModel   ;
class KeyExchangeModelPrivate ;
class PendingTrustRequestModel;

//Private
class AccountPrivate;
class AccountPlaceHolderPrivate;


///@enum DtmfType Different method to send the DTMF (key sound) to the peer
enum DtmfType {
   OverRtp,
   OverSip
};
Q_ENUMS(DtmfType)

/**
 * A communication account.
 *
 * This class represent an account based around a protocol and a bunch of properties.
 *
 * Using the setters on this object wont cause the changes to take effect immediately.
 *
 * To save the changes, use the "<<" operator on the account with Account::EditAction::SAVE.
 * Similarly, the Account::EditAction::RELOAD action will reset the changes to match the
 * current properties used by daemon.
 */
class LIB_EXPORT Account : public ItemBase {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop

   //The account class delegate all properties part of "sets" to children models
   friend class AccountModel;
   friend class AccountModelPrivate;
   friend class AccountPlaceHolder;
   friend class CipherModelPrivate;
   friend class CipherModel;
   friend class AccountStatusModelPrivate;
   friend class AccountStatusModel;
   friend class TlsMethodModelPrivate;
   friend class TlsMethodModel;
   friend class BootstrapModelPrivate;
   friend class KeyExchangeModel;
   friend class KeyExchangeModelPrivate;
   friend class ContactMethod;
   friend class Certificate;
   friend class NetworkInterfaceModelPrivate;

   //Properties
   Q_PROPERTY(QByteArray     id                           READ id                                                                 )
   Q_PROPERTY(QString        alias                        READ alias                         WRITE setAlias                       )
   Q_PROPERTY(Account::Protocol protocol                  READ protocol                      WRITE setProtocol                    )
   Q_PROPERTY(QString        hostname                     READ hostname                      WRITE setHostname                    )
   Q_PROPERTY(QString        username                     READ username                      WRITE setUsername                    )
   Q_PROPERTY(QString        mailbox                      READ mailbox                       WRITE setMailbox                     )
   Q_PROPERTY(QString        proxy                        READ proxy                         WRITE setProxy                       )
   Q_PROPERTY(QString        tlsPassword                  READ tlsPassword                   WRITE setTlsPassword                 )
   Q_PROPERTY(Certificate*   tlsCaListCertificate         READ tlsCaListCertificate          WRITE setTlsCaListCertificate        )
   Q_PROPERTY(Certificate*   tlsCertificate               READ tlsCertificate                WRITE setTlsCertificate              )
   Q_PROPERTY(Certificate*   tlsPrivateKeyCertificate     READ tlsPrivateKeyCertificate      WRITE setTlsPrivateKeyCertificate    )
   Q_PROPERTY(QString        tlsServerName                READ tlsServerName                 WRITE setTlsServerName               )
   Q_PROPERTY(QString        sipStunServer                READ sipStunServer                 WRITE setSipStunServer               )
   Q_PROPERTY(QString        publishedAddress             READ publishedAddress              WRITE setPublishedAddress            )
   Q_PROPERTY(QString        ringtonePath                 READ ringtonePath                  WRITE setRingtonePath                )
   Q_PROPERTY(int            registrationExpire           READ registrationExpire            WRITE setRegistrationExpire          )
   Q_PROPERTY(int            tlsNegotiationTimeoutSec     READ tlsNegotiationTimeoutSec      WRITE setTlsNegotiationTimeoutSec    )
   Q_PROPERTY(int            localPort                    READ localPort                     WRITE setLocalPort                   )
   Q_PROPERTY(int            bootstrapPort                READ bootstrapPort                 WRITE setBootstrapPort               )
   Q_PROPERTY(int            publishedPort                READ publishedPort                 WRITE setPublishedPort               )
   Q_PROPERTY(bool           enabled                      READ isEnabled                     WRITE setEnabled                     )
   Q_PROPERTY(bool           autoAnswer                   READ isAutoAnswer                  WRITE setAutoAnswer                  )
   Q_PROPERTY(bool           tlsVerifyServer              READ isTlsVerifyServer             WRITE setTlsVerifyServer             )
   Q_PROPERTY(bool           tlsVerifyClient              READ isTlsVerifyClient             WRITE setTlsVerifyClient             )
   Q_PROPERTY(bool           tlsRequireClientCertificate  READ isTlsRequireClientCertificate WRITE setTlsRequireClientCertificate )
   Q_PROPERTY(bool           tlsEnabled                   READ isTlsEnabled                  WRITE setTlsEnabled                  )
   Q_PROPERTY(bool           displaySasOnce               READ isDisplaySasOnce              WRITE setDisplaySasOnce              )
   Q_PROPERTY(bool           srtpRtpFallback              READ isSrtpRtpFallback             WRITE setSrtpRtpFallback             )
   Q_PROPERTY(bool           zrtpDisplaySas               READ isZrtpDisplaySas              WRITE setZrtpDisplaySas              )
   Q_PROPERTY(bool           zrtpNotSuppWarning           READ isZrtpNotSuppWarning          WRITE setZrtpNotSuppWarning          )
   Q_PROPERTY(bool           zrtpHelloHash                READ isZrtpHelloHash               WRITE setZrtpHelloHash               )
   Q_PROPERTY(bool           sipStunEnabled               READ isSipStunEnabled              WRITE setSipStunEnabled              )
   Q_PROPERTY(bool           publishedSameAsLocal         READ isPublishedSameAsLocal        WRITE setPublishedSameAsLocal        )
   Q_PROPERTY(bool           ringtoneEnabled              READ isRingtoneEnabled             WRITE setRingtoneEnabled             )
   Q_PROPERTY(DtmfType       dTMFType                     READ DTMFType                      WRITE setDTMFType                    )
   Q_PROPERTY(int            voiceMailCount               READ voiceMailCount                WRITE setVoiceMailCount              )
//    Q_PROPERTY(QString        typeName                     READ type                          WRITE setType                        )
   Q_PROPERTY(QString        lastErrorMessage             READ lastErrorMessage                                                   )
   Q_PROPERTY(int            lastErrorCode                READ lastErrorCode                                                      )
   Q_PROPERTY(bool           presenceStatus               READ presenceStatus                                                     )
   Q_PROPERTY(QString        presenceMessage              READ presenceMessage                                                    )
   Q_PROPERTY(bool           supportPresencePublish       READ supportPresencePublish                                             )
   Q_PROPERTY(bool           supportPresenceSubscribe     READ supportPresenceSubscribe                                           )
   Q_PROPERTY(bool           presenceEnabled              READ presenceEnabled               WRITE setPresenceEnabled NOTIFY presenceEnabledChanged)
   Q_PROPERTY(bool           videoEnabled                 READ isVideoEnabled                WRITE setVideoEnabled                )
   Q_PROPERTY(int            videoPortMax                 READ videoPortMax                  WRITE setVideoPortMax                )
   Q_PROPERTY(int            videoPortMin                 READ videoPortMin                  WRITE setVideoPortMin                )
   Q_PROPERTY(int            audioPortMax                 READ audioPortMax                  WRITE setAudioPortMax                )
   Q_PROPERTY(int            audioPortMin                 READ audioPortMin                  WRITE setAudioPortMin                )
   Q_PROPERTY(bool           upnpEnabled                  READ isUpnpEnabled                 WRITE setUpnpEnabled                 )
   Q_PROPERTY(bool           hasCustomUserAgent           READ hasCustomUserAgent            WRITE setHasCustomUserAgent          )

   Q_PROPERTY(QString        userAgent                    READ userAgent                     WRITE setUserAgent                   )
   Q_PROPERTY(bool           useDefaultPort               READ useDefaultPort                WRITE setUseDefaultPort              )
   Q_PROPERTY(QString        displayName                  READ displayName                   WRITE setDisplayName                 )
   Q_PROPERTY(RegistrationState registrationState         READ registrationState                                                  )
   Q_PROPERTY(bool           usedForOutgogingCall         READ isUsedForOutgogingCall                                             )
   Q_PROPERTY(uint           totalCallCount               READ totalCallCount                                                     )
   Q_PROPERTY(uint           weekCallCount                READ weekCallCount                                                      )
   Q_PROPERTY(uint           trimesterCallCount           READ trimesterCallCount                                                 )
   Q_PROPERTY(time_t         lastUsed                     READ lastUsed                                                           )

   Q_PROPERTY(bool           allowIncomingFromHistory     READ allowIncomingFromHistory      WRITE setAllowIncomingFromHistory    )
   Q_PROPERTY(bool           allowIncomingFromContact     READ allowIncomingFromContact      WRITE setAllowIncomingFromContact    )
   Q_PROPERTY(bool           allowIncomingFromUnknown     READ allowIncomingFromUnknown      WRITE setAllowIncomingFromUnknown    )

   Q_PROPERTY(CredentialModel*         credentialModel             READ credentialModel                                           )
   Q_PROPERTY(CodecModel*              codecModel                  READ codecModel                                                )
   Q_PROPERTY(KeyExchangeModel*        keyExchangeModel            READ keyExchangeModel                                          )
   Q_PROPERTY(CipherModel*             cipherModel                 READ cipherModel                                               )
   Q_PROPERTY(AccountStatusModel*      statusModel                 READ statusModel                                               )
   Q_PROPERTY(SecurityEvaluationModel* securityEvaluationModel     READ securityEvaluationModel                                   )
   Q_PROPERTY(TlsMethodModel*          tlsMethodModel              READ tlsMethodModel                                            )
   Q_PROPERTY(ProtocolModel*           protocolModel               READ protocolModel                                             )
   Q_PROPERTY(BootstrapModel*          bootstrapModel              READ bootstrapModel                                            )
   Q_PROPERTY(NetworkInterfaceModel*   networkInterfaceModel       READ networkInterfaceModel                                     )
   Q_PROPERTY(QAbstractItemModel*      knownCertificateModel       READ knownCertificateModel                                     )
   Q_PROPERTY(QAbstractItemModel*      bannedCertificatesModel     READ bannedCertificatesModel                                   )
   Q_PROPERTY(QAbstractItemModel*      allowedCertificatesModel    READ allowedCertificatesModel                                  )

   public:

      ///@enum EditState: Manage how and when an account can be reloaded or change state
      enum class EditState {
         READY               = 0, /*!< The account is synchronized                                       */
         EDITING             = 1, /*!< The account is "locked" by the client for edition                 */
         OUTDATED            = 2, /*!< The remote and local details are out of sync                      */
         NEW                 = 3, /*!< The account is new, there is no remote                            */
         MODIFIED_INCOMPLETE = 4, /*!< The account has modified, but required fields are missing/invalid */
         MODIFIED_COMPLETE   = 5, /*!< The account has modified and can be saved                         */
         REMOVED             = 6, /*!< The account remote has been deleted                               */
         COUNT__
      };

      ///@enum EditAction Actions that can be performed on the Account state
      enum class EditAction {
         NOTHING = 0, /*!< Do nothing                                        */
         EDIT    = 1, /*!< Lock the current details for edition              */
         RELOAD  = 2, /*!< Override the local details with the remote ones   */
         SAVE    = 3, /*!< Override the remote details with the local ones   */
         REMOVE  = 4, /*!< Remove this account                               */
         MODIFY  = 5, /*!< Change the account details                        */
         CANCEL  = 6, /*!< Cancel the modification, override with the remote */
         COUNT__
      };
      Q_ENUMS(EditAction)

      ///@enum RegistrationState The account state from a client point of view
      enum class RegistrationState {
         READY        = 0, /*!< The account can be used to pass a call   */
         UNREGISTERED = 1, /*!< The account isn't functional voluntarily */
         TRYING       = 2, /*!< The account is trying to become ready    */
         ERROR        = 3, /*!< The account failed to become ready       */
         COUNT__
      };
      Q_ENUMS(RegistrationState)

      enum class Role {
         Alias                       = 100,
         Proto                       = 101,
         Hostname                    = 102,
         Username                    = 103,
         Mailbox                     = 104,
         Proxy                       = 105,
         TlsPassword                 = 107,
         TlsCaListCertificate        = 108,
         TlsCertificate              = 109,
         TlsPrivateKeyCertificate    = 110,
         TlsServerName               = 112,
         SipStunServer               = 113,
         PublishedAddress            = 114,
         RingtonePath                = 116,
         RegistrationExpire          = 118,
         TlsNegotiationTimeoutSec    = 119,
         TlsNegotiationTimeoutMsec   = 120,
         LocalPort                   = 121,
         BootstrapPort               = 122,
         PublishedPort               = 123,
         Enabled                     = 124,
         AutoAnswer                  = 125,
         TlsVerifyServer             = 126,
         TlsVerifyClient             = 127,
         TlsRequireClientCertificate = 128,
         TlsEnabled                  = 129,
         DisplaySasOnce              = 130,
         SrtpRtpFallback             = 131,
         ZrtpDisplaySas              = 132,
         ZrtpNotSuppWarning          = 133,
         ZrtpHelloHash               = 134,
         SipStunEnabled              = 135,
         PublishedSameAsLocal        = 136,
         RingtoneEnabled             = 137,
         dTMFType                    = 138,
         Id                          = 139,
         Object                      = 140,
         TypeName                    = 141,
         PresenceStatus              = 142,
         PresenceMessage             = 143,
         RegistrationState           = 144,
         UseDefaultPort              = 145,
         UsedForOutgogingCall        = 146,
         TotalCallCount              = 147,
         WeekCallCount               = 148,
         TrimesterCallCount          = 149,
         LastUsed                    = 150,
         SipTurnServer               = 151,
         SipTurnEnabled              = 152,
         UserAgent                   = 153,
         Password                    = 154,
         SupportPresencePublish      = 155,
         SupportPresenceSubscribe    = 156,
         PresenceEnabled             = 165,
         IsVideoEnabled              = 169,
         VideoPortMax                = 157,
         VideoPortMin                = 158,
         AudioPortMin                = 159,
         AudioPortMax                = 160,
         IsUpnpEnabled               = 161,
         HasCustomUserAgent          = 162,
         LastTransportErrorCode      = 163,
         LastTransportErrorMessage   = 164,
         TurnServer                  = 168,
         HasProxy                    = 170,
         DisplayName                 = 171,
         SrtpEnabled                 = 172,
         HasCustomBootstrap          = 173,
         CredentialModel             = 174,
         CodecModel                  = 175,
         KeyExchangeModel            = 176,
         CipherModel                 = 177,
         StatusModel                 = 178,
         SecurityEvaluationModel     = 179,
         TlsMethodModel              = 180,
         ProtocolModel               = 181,
         BootstrapModel              = 182,
         NetworkInterfaceModel       = 183,
         KnownCertificateModel       = 184,
         BannedCertificatesModel     = 185,
         AllowedCertificatesModel    = 186,
         AllowIncomingFromHistory    = 187,
         AllowIncomingFromContact    = 188,
         AllowIncomingFromUnknown    = 189,
         ActiveCallLimit             = 190,
         HasActiveCallLimit          = 191,
         SecurityLevel               = 192,
         SecurityLevelIcon           = 193,
      };

      ///@enum RoleState Whether a role can be used in a certain context
      enum class RoleState {
         READ_WRITE ,
         READ_ONLY  ,
         UNAVAILABLE,
      };

      ///@enum RoleStatus The current status of the role based on its value
      enum class RoleStatus {
         OK            , /*!< The field is ok (the default)                      */
         UNTESTED      , /*!< A test is in progress, no results yet (ex: network)*/
         INVALID       , /*!< The content is invalid                             */
         REQUIRED_EMPTY, /*!< The field is empty but is required                 */
         OUT_OF_RANGE  , /*!< The value is out of range                          */
      };

      enum class Protocol {
         SIP  = 0, /*!< Used for both SIP and IP2IP calls */
         IAX  = 1, /*!< Inter Asterisk exchange protocol  */
         RING = 2, /*!< Used for RING-DHT calls           */
         COUNT__,
      };
      Q_ENUMS(Protocol)

      /**
       *Perform an action
       * @return If the state changed
       */
      bool performAction(Account::EditAction action);
      Account::EditState editState() const;

      //Getters
      bool             isNew           () const;
      const QByteArray id              () const;
      const QString    toHumanStateName() const;
      const QString    alias           () const;
      QModelIndex      index           () const;
      QString          stateColorName  () const;
      QVariant         stateColor      () const;
      virtual bool     isLoaded        () const;

      CredentialModel*          credentialModel            () const;
      CodecModel*               codecModel                 () const;
      KeyExchangeModel*         keyExchangeModel           () const;
      CipherModel*              cipherModel                () const;
      AccountStatusModel*       statusModel                () const;
      SecurityEvaluationModel*  securityEvaluationModel    () const;
      TlsMethodModel*           tlsMethodModel             () const;
      ProtocolModel*            protocolModel              () const;
      BootstrapModel*           bootstrapModel             () const;
      NetworkInterfaceModel*    networkInterfaceModel      () const;
      QAbstractItemModel*       knownCertificateModel      () const;
      QAbstractItemModel*       bannedCertificatesModel    () const;
      QAbstractItemModel*       allowedCertificatesModel   () const;
      PendingTrustRequestModel* pendingTrustRequestModel   () const;

      Q_INVOKABLE RoleState  roleState (Account::Role role) const;
      Q_INVOKABLE RoleStatus roleStatus(Account::Role role) const;

      //Getters
      QString hostname                     () const;
      bool    isEnabled                    () const;
      bool    isAutoAnswer                 () const;
      QString username                     () const;
      QString mailbox                      () const;
      QString proxy                        () const;
      QString password                     () const;
      bool    isDisplaySasOnce             () const;
      bool    isSrtpRtpFallback            () const;
      bool    isSrtpEnabled                () const;
      bool    isZrtpDisplaySas             () const;
      bool    isZrtpNotSuppWarning         () const;
      bool    isZrtpHelloHash              () const;
      bool    isSipStunEnabled             () const;
      QString sipStunServer                () const;
      int     registrationExpire           () const;
      bool    isPublishedSameAsLocal       () const;
      QString publishedAddress             () const;
      int     publishedPort                () const;
      QString tlsPassword                  () const;
      int     bootstrapPort                () const;
      Certificate* tlsCaListCertificate    () const;
      Certificate* tlsCertificate          () const;
      Certificate* tlsPrivateKeyCertificate() const;
      QString tlsServerName                () const;
      int     tlsNegotiationTimeoutSec     () const;
      bool    isTlsVerifyServer            () const;
      bool    isTlsVerifyClient            () const;
      bool    isTlsRequireClientCertificate() const;
      bool    isTlsEnabled                 () const;
      bool    isRingtoneEnabled            () const;
      QString ringtonePath                 () const;
      QString lastErrorMessage             () const;
      int     lastErrorCode                () const;
      int     localPort                    () const;
      int     voiceMailCount               () const;
      DtmfType DTMFType                    () const;
      bool    presenceStatus               () const;
      QString presenceMessage              () const;
      bool    supportPresencePublish       () const;
      bool    supportPresenceSubscribe     () const;
      bool    presenceEnabled              () const;
      bool    isVideoEnabled               () const;
      int     videoPortMax                 () const;
      int     videoPortMin                 () const;
      int     audioPortMin                 () const;
      int     audioPortMax                 () const;
      bool    isUpnpEnabled                () const;
      bool    hasCustomUserAgent           () const;
      int     lastTransportErrorCode       () const;
      QString lastTransportErrorMessage    () const;
      QString userAgent                    () const;
      bool    useDefaultPort               () const;
      bool    isTurnEnabled                () const;
      QString turnServer                   () const;
      bool    hasProxy                     () const;
      QString displayName                  () const;
      RegistrationState  registrationState () const;
      Protocol           protocol          () const;
      ContactMethod*     contactMethod     () const;
      bool    allowIncomingFromUnknown     () const;
      bool    allowIncomingFromHistory     () const;
      bool    allowIncomingFromContact     () const;
      int     activeCallLimit              () const;
      bool    hasActiveCallLimit           () const;

      bool   isUsedForOutgogingCall () const;
      uint   totalCallCount         () const;
      uint   weekCallCount          () const;
      uint   trimesterCallCount     () const;
      time_t lastUsed               () const;

      Q_INVOKABLE QVariant roleData    ( int role             ) const;
      Q_INVOKABLE bool supportScheme   ( URI::SchemeType type )      ;
      Q_INVOKABLE bool allowCertificate( Certificate* c       )      ;
      Q_INVOKABLE bool banCertificate  ( Certificate* c       )      ;
      Q_INVOKABLE bool requestTrust    ( Certificate* c       )      ;

      //Setters
      void setId                            (const QByteArray& id   );
      void setAlias                         (const QString& detail  );
      void setProtocol                      (Account::Protocol proto);
      void setHostname                      (const QString& detail  );
      void setUsername                      (const QString& detail  );
      void setMailbox                       (const QString& detail  );
      void setProxy                         (const QString& detail  );
      void setPassword                      (const QString& detail  );
      void setTlsPassword                   (const QString& detail  );
      void setTlsCaListCertificate          (Certificate* cert      );
      void setTlsCertificate                (Certificate* cert      );
      void setTlsPrivateKeyCertificate      (Certificate* cert      );
      void setTlsCaListCertificate          (const QString& detail  );
      void setTlsCertificate                (const QString& detail  );
      void setTlsPrivateKeyCertificate      (const QString& detail  );
      void setTlsServerName                 (const QString& detail  );
      void setSipStunServer                 (const QString& detail  );
      void setPublishedAddress              (const QString& detail  );
      void setRingtonePath                  (const QString& detail  );
      void setTurnServer                    (const QString& value   );
      void setDisplayName                   (const QString& value   );
      void setVoiceMailCount                (int  count );
      void setRegistrationExpire            (int  detail);
      void setTlsNegotiationTimeoutSec      (int  detail);
      void setLocalPort                     (unsigned short detail);
      void setBootstrapPort                 (unsigned short detail);
      void setPublishedPort                 (unsigned short detail);
      void setAutoAnswer                    (bool detail);
      void setTlsVerifyServer               (bool detail);
      void setTlsVerifyClient               (bool detail);
      void setTlsRequireClientCertificate   (bool detail);
      void setTlsEnabled                    (bool detail);
      void setDisplaySasOnce                (bool detail);
      void setSrtpRtpFallback               (bool detail);
      void setSrtpEnabled                   (bool detail);
      void setZrtpDisplaySas                (bool detail);
      void setZrtpNotSuppWarning            (bool detail);
      void setZrtpHelloHash                 (bool detail);
      void setSipStunEnabled                (bool detail);
      void setPublishedSameAsLocal          (bool detail);
      void setRingtoneEnabled               (bool detail);
      void setPresenceEnabled               (bool enable);
      void setVideoEnabled                  (bool enable);
      void setAudioPortMax                  (int port   );
      void setAudioPortMin                  (int port   );
      void setVideoPortMax                  (int port   );
      void setVideoPortMin                  (int port   );
      void setActiveCallLimit               (int value  );
      void setDTMFType                      (DtmfType type);
      void setUserAgent                     (const QString& agent);
      void setUpnpEnabled                   (bool enable);
      void setHasCustomUserAgent            (bool enable);
      void setUseDefaultPort                (bool value );
      void setTurnEnabled                   (bool value );
      void setAllowIncomingFromHistory      (bool value );
      void setAllowIncomingFromContact      (bool value );
      void setAllowIncomingFromUnknown      (bool value );
      void setHasActiveCallLimit            (bool value );

      void setRoleData(int role, const QVariant& value);

      //Operators
      bool operator==(const Account&)const;
      Account* operator<<(Account::EditAction& action);

   public Q_SLOTS:
      void setEnabled(bool checked);

   private:
      //Constructors
      explicit Account();
      ~Account();

      QSharedPointer<AccountPrivate> d_ptr;
      Q_DECLARE_PRIVATE(Account)

   Q_SIGNALS:
      ///The account state (Invalid,Trying,Registered) changed
      void stateChanged(Account::RegistrationState state);
      ///One of the account property changed
      //TODO Qt5 drop the account parameter
      void propertyChanged(Account* a, const QString& name, const QString& newVal, const QString& oldVal);
      ///Something(s) in the account changed
      void changed(Account* a);
      ///The alias changed, take effect instantaneously
      void aliasChanged(const QString&);
      ///The presence support changed
      void presenceEnabledChanged(bool);
      ///The account has been enabled/disabled
      void enabled(bool);
      ///The account edit state changed
      void editStateChanged(const EditState state, const EditState previous);
};
// Q_DISABLE_COPY(Account)
Q_DECLARE_METATYPE(Account*)
Q_DECLARE_METATYPE(Account::RegistrationState)
Q_DECLARE_METATYPE(Account::EditAction)
Q_DECLARE_METATYPE(Account::Protocol)
Q_DECLARE_METATYPE(DtmfType)

Account* operator<<(Account* a, Account::EditAction action);

/**
 * Some accounts can be loaded at later time. This object will be upgraded
 * to an account when it arrive
 */
class LIB_EXPORT AccountPlaceHolder : public Account {
   Q_OBJECT
   friend class AccountModel;
private:
   explicit AccountPlaceHolder(const QByteArray& uid);

   AccountPlaceHolderPrivate* d_ptr;
   Q_DECLARE_PRIVATE(AccountPlaceHolder)
};


#endif
