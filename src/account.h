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
#include "keyexchangemodel.h"
#include "tlsmethodmodel.h"
#include "uri.h"
#include "typedefs.h"
class CredentialModel        ;
class RingToneModel          ;
class ContactMethod          ;
class SecurityEvaluationModel;
class Certificate            ;
class CipherModel            ;
class AccountStatusModel     ;
class ProtocolModel          ;
class CodecModel             ;
class BootstrapModel         ;
class NetworkInterfaceModel  ;

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
class LIB_EXPORT Account : public QObject {
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
   Q_PROPERTY(QString        lastErrorMessage             READ lastErrorMessage              WRITE setLastErrorMessage            )
   Q_PROPERTY(KeyExchangeModel::Type keyExchange          READ keyExchange                   WRITE setKeyExchange                 )
   Q_PROPERTY(int            lastErrorCode                READ lastErrorCode                 WRITE setLastErrorCode               )
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
   Q_PROPERTY(RegistrationState registrationState         READ registrationState                                                  )
   Q_PROPERTY(bool           usedForOutgogingCall         READ isUsedForOutgogingCall                                             )
   Q_PROPERTY(uint           totalCallCount               READ totalCallCount                                                     )
   Q_PROPERTY(uint           weekCallCount                READ weekCallCount                                                      )
   Q_PROPERTY(uint           trimesterCallCount           READ trimesterCallCount                                                 )
   Q_PROPERTY(time_t         lastUsed                     READ lastUsed                                                           )

   public:
      ///@enum EditState: Manage how and when an account can be reloaded or change state
      enum class EditState {
         READY    = 0,
         EDITING  = 1,
         OUTDATED = 2,
         NEW      = 3,
         MODIFIED = 4,
         REMOVED  = 5
      };

      ///@enum EditAction Actions that can be performed on the Account state
      enum class EditAction {
         NOTHING = 0,
         EDIT    = 1,
         RELOAD  = 2,
         SAVE    = 3,
         REMOVE  = 4,
         MODIFY  = 5,
         CANCEL  = 6
      };
      Q_ENUMS(EditAction)

      ///@enum RegistrationState The account state from a client point of view
      enum class RegistrationState {
         READY        = 0,
         UNREGISTERED = 1,
         TRYING       = 2,
         ERROR        = 3,
         COUNT__,
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
         KeyExchange                 = 190,
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
      };

      enum class Protocol {
         SIP  = 0,
         IAX  = 1,
         RING = 2,
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

      Q_INVOKABLE CredentialModel*         credentialModel            () const;
      Q_INVOKABLE CodecModel*              codecModel                 () const;
      Q_INVOKABLE RingToneModel*           ringToneModel              () const;
      Q_INVOKABLE KeyExchangeModel*        keyExchangeModel           () const;
      Q_INVOKABLE CipherModel*             cipherModel                () const;
      Q_INVOKABLE AccountStatusModel*      statusModel                () const;
      Q_INVOKABLE SecurityEvaluationModel* securityEvaluationModel    () const;
      Q_INVOKABLE TlsMethodModel*          tlsMethodModel             () const;
      Q_INVOKABLE ProtocolModel*           protocolModel              () const;
      Q_INVOKABLE BootstrapModel*          bootstrapModel             () const;
      Q_INVOKABLE NetworkInterfaceModel*   networkInterfaceModel      () const;
      Q_INVOKABLE QAbstractItemModel*      knownCertificateModel      () const;
      Q_INVOKABLE QAbstractItemModel*      backlistedCertificatesModel() const;
      Q_INVOKABLE QAbstractItemModel*      trustedCertificatesModel   () const;

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
      RegistrationState  registrationState () const;
      Protocol               protocol      () const;
      KeyExchangeModel::Type keyExchange   () const;

      bool   isUsedForOutgogingCall () const;
      uint   totalCallCount         () const;
      uint   weekCallCount          () const;
      uint   trimesterCallCount     () const;
      time_t lastUsed               () const;

      Q_INVOKABLE QVariant roleData ( int role             ) const;
      Q_INVOKABLE bool supportScheme( URI::SchemeType type )      ;

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
      void setLastErrorMessage              (const QString& message );
      void setTurnServer                    (const QString& value   );
      void setKeyExchange                   (KeyExchangeModel::Type detail);
      void setLastErrorCode                 (int  code  );
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
      void setDTMFType                      (DtmfType type);
      void setUserAgent                     (const QString& agent);
      void setUpnpEnabled                   (bool enable);
      void setHasCustomUserAgent            (bool enable);
      void setUseDefaultPort                (bool value );
      void setTurnEnabled                   (bool value );

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
