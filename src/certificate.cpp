/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software;you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation;either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY;without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "certificate.h"

#include <QtCore/QFile>
#include "dbus/configurationmanager.h"
#include "/home/lepagee/dev/sflphone_review/daemon/src/dring/security.h"

class CertificateDetailsCache {
public:
   CertificateDetailsCache(const MapStringString& details);

   QDateTime  m_ExpirationDate            ;
   QDateTime  m_ActivationDate            ;
   bool       m_RequirePrivateKeyPassword ;
   QByteArray m_PublicSignature           ;
   int        m_VersionNumber             ;
   QByteArray m_SerialNumber              ;
   QString    m_Issuer                    ;
   QByteArray m_SubjectKeyAlgorithm       ;
   QString    m_Cn                        ;
   QString    m_N                         ;
   QString    m_O                         ;
   QByteArray m_SignatureAlgorithm        ;
   QByteArray m_Md5Fingerprint            ;
   QByteArray m_Sha1Fingerprint           ;
   QByteArray m_PublicKeyId               ;
   QByteArray m_IssuerDn                  ;
   QDateTime  m_NextExpectedUpdateDate    ;
};

class CertificateChecksCache {
public:
   CertificateChecksCache(const MapStringString& checks);

   Certificate::CheckValues m_HasPrivateKey                       ;
   Certificate::CheckValues m_IsExpired                           ;
   Certificate::CheckValues m_HasStrongSigning                    ;
   Certificate::CheckValues m_IsSelfSigned                        ;
   Certificate::CheckValues m_PrivateKeyMatch                     ;
   Certificate::CheckValues m_ArePrivateKeyStoragePermissionOk    ;
   Certificate::CheckValues m_ArePublicKeyStoragePermissionOk     ;
   Certificate::CheckValues m_ArePrivateKeyDirectoryPermissionsOk ;
   Certificate::CheckValues m_ArePublicKeyDirectoryPermissionsOk  ;
   Certificate::CheckValues m_ArePrivateKeyStorageLocationOk      ;
   Certificate::CheckValues m_ArePublicKeyStorageLocationOk       ;
   Certificate::CheckValues m_ArePrivateKeySelinuxAttributesOk    ;
   Certificate::CheckValues m_ArePublicKeySelinuxAttributesOk     ;
   QString                  m_OutgoingServer                      ;
   Certificate::CheckValues m_Exist                               ;
   Certificate::CheckValues m_IsValid                             ;
   Certificate::CheckValues m_ValidAuthority                      ;
   Certificate::CheckValues m_HasKnownAuthority                   ;
   Certificate::CheckValues m_IsNotRevoked                        ;
   Certificate::CheckValues m_AuthorityMismatch                   ;
   Certificate::CheckValues m_UnexpectedOwner                     ;
   Certificate::CheckValues m_NotActivated                        ;
};

class CertificatePrivate
{
public:
   //Attributes
   CertificatePrivate();
   ~CertificatePrivate();
   QUrl m_Path;
   Certificate::Type m_Type;

   mutable CertificateDetailsCache* m_pDetailsCache;
   mutable CertificateChecksCache*  m_pCheckCache  ;

   //Helpers
   void loadDetails();
   void loadChecks ();

   static Certificate::CheckValues toBool(const QString& string);
};

CertificatePrivate::CertificatePrivate() :
m_pCheckCache(nullptr), m_pDetailsCache(nullptr)
{
}

CertificatePrivate::~CertificatePrivate()
{
   if (m_pDetailsCache) delete m_pDetailsCache;
   if (m_pCheckCache  ) delete m_pCheckCache  ;
}

Certificate::CheckValues CertificatePrivate::toBool(const QString& string)
{
   if (string == DRing::Certificate::CheckValuesNames::PASSED)
      return Certificate::CheckValues::PASSED;
   else if (string == DRing::Certificate::CheckValuesNames::FAILED)
      return Certificate::CheckValues::FAILED;
   else
      return Certificate::CheckValues::UNSUPPORTED;
}

CertificateDetailsCache::CertificateDetailsCache(const MapStringString& details)
{
   m_ExpirationDate             = QDateTime::fromTime_t( details[DRing::Certificate::DetailsNames::EXPIRATION_DATE ].toInt());
   m_ActivationDate             = QDateTime::fromTime_t( details[DRing::Certificate::DetailsNames::ACTIVATION_DATE ].toInt());
   m_RequirePrivateKeyPassword  = false;//TODO//details[DRing::Certificate::DetailsNames::REQUIRE_PRIVATE_KEY_PASSWORD].toBool();
   m_PublicSignature            = details[DRing::Certificate::DetailsNames::PUBLIC_SIGNATURE            ].toAscii();
   m_VersionNumber              = details[DRing::Certificate::DetailsNames::VERSION_NUMBER              ].toInt();
   m_SerialNumber               = details[DRing::Certificate::DetailsNames::SERIAL_NUMBER               ].toAscii();
   m_Issuer                     = details[DRing::Certificate::DetailsNames::ISSUER                      ];
   m_SubjectKeyAlgorithm        = details[DRing::Certificate::DetailsNames::SUBJECT_KEY_ALGORITHM       ].toAscii();
   m_Cn                         = details[DRing::Certificate::DetailsNames::CN                          ];
   m_N                          = details[DRing::Certificate::DetailsNames::N                           ];
   m_O                          = details[DRing::Certificate::DetailsNames::O                           ];
   m_SignatureAlgorithm         = details[DRing::Certificate::DetailsNames::SIGNATURE_ALGORITHM         ].toAscii();
   m_Md5Fingerprint             = details[DRing::Certificate::DetailsNames::MD5_FINGERPRINT             ].toAscii();
   m_Sha1Fingerprint            = details[DRing::Certificate::DetailsNames::SHA1_FINGERPRINT            ].toAscii();
   m_PublicKeyId                = details[DRing::Certificate::DetailsNames::PUBLIC_KEY_ID               ].toAscii();
   m_IssuerDn                   = details[DRing::Certificate::DetailsNames::ISSUER_DN                   ].toAscii();
   m_NextExpectedUpdateDate     = QDateTime::fromTime_t(details[DRing::Certificate::DetailsNames::NEXT_EXPECTED_UPDATE_DATE ].toInt());
}

CertificateChecksCache::CertificateChecksCache(const MapStringString& checks)
{
   m_HasPrivateKey                       = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::HAS_PRIVATE_KEY                  ]);
   m_IsExpired                           = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::EXPIRED                          ]);
   m_HasStrongSigning                    = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::STRONG_SIGNING                   ]);
   m_IsSelfSigned                        = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::NOT_SELF_SIGNED                  ]);
   m_PrivateKeyMatch                     = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::KEY_MATCH                        ]);
   m_ArePrivateKeyStoragePermissionOk    = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PRIVATE_KEY_STORAGE_PERMISSION   ]);
   m_ArePublicKeyStoragePermissionOk     = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PUBLIC_KEY_STORAGE_PERMISSION    ]);
   m_ArePrivateKeyDirectoryPermissionsOk = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PRIVATE_KEY_DIRECTORY_PERMISSIONS]);
   m_ArePublicKeyDirectoryPermissionsOk  = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PUBLIC_KEY_DIRECTORY_PERMISSIONS ]);
   m_ArePrivateKeyStorageLocationOk      = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PRIVATE_KEY_STORAGE_LOCATION     ]);
   m_ArePublicKeyStorageLocationOk       = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PUBLIC_KEY_STORAGE_LOCATION      ]);
   m_ArePrivateKeySelinuxAttributesOk    = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PRIVATE_KEY_SELINUX_ATTRIBUTES   ]);
   m_ArePublicKeySelinuxAttributesOk     = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::PUBLIC_KEY_SELINUX_ATTRIBUTES    ]);
   m_OutgoingServer                      = checks[DRing::Certificate::ChecksNames::OUTGOING_SERVER                                             ] ;
   m_Exist                               = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::EXIST                            ]);
   m_IsValid                             = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::VALID                            ]);
   m_ValidAuthority                      = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::VALID_AUTHORITY                  ]);
   m_HasKnownAuthority                   = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::KNOWN_AUTHORITY                  ]);
   m_IsNotRevoked                        = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::NOT_REVOKED                      ]);
   m_AuthorityMismatch                   = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::AUTHORITY_MISMATCH               ]);
   m_UnexpectedOwner                     = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::UNEXPECTED_OWNER                 ]);
   m_NotActivated                        = CertificatePrivate::toBool(checks[DRing::Certificate::ChecksNames::NOT_ACTIVATED                    ]);
}

void CertificatePrivate::loadDetails()
{
   if (!m_pDetailsCache) {
      const MapStringString details = DBus::ConfigurationManager::instance().getCertificateDetails(m_Path.toString());
      m_pDetailsCache = new CertificateDetailsCache(details);
   }
}

void CertificatePrivate::loadChecks()
{
   if (!m_pCheckCache) {
      const MapStringString checks = DBus::ConfigurationManager::instance().validateCertificate(QString(),m_Path.toString(),QString());
      m_pCheckCache = new CertificateChecksCache(checks);
   }
}

Certificate::Certificate(Certificate::Type type, const QObject* parent) : QObject(const_cast<QObject*>(parent)),d_ptr(new CertificatePrivate())
{
   d_ptr->m_Type = type;
}

Certificate::CheckValues Certificate::hasPrivateKey() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_HasPrivateKey;
}

Certificate::CheckValues Certificate::isNotExpired() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_IsExpired;
}

Certificate::CheckValues Certificate::hasStrongSigning() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_HasStrongSigning;
}

Certificate::CheckValues Certificate::isNotSelfSigned() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_IsSelfSigned;
}

Certificate::CheckValues Certificate::privateKeyMatch() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_PrivateKeyMatch;
}

Certificate::CheckValues Certificate::arePrivateKeyStoragePermissionOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeyStoragePermissionOk;
}

Certificate::CheckValues Certificate::arePublicKeyStoragePermissionOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeyStoragePermissionOk;
}

Certificate::CheckValues Certificate::arePrivateKeyDirectoryPermissionsOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeyDirectoryPermissionsOk;
}

Certificate::CheckValues Certificate::arePublicKeyDirectoryPermissionsOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeyDirectoryPermissionsOk;
}

Certificate::CheckValues Certificate::arePrivateKeyStorageLocationOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeyStorageLocationOk;
}

Certificate::CheckValues Certificate::arePublicKeyStorageLocationOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeyStorageLocationOk;
}

Certificate::CheckValues Certificate::arePrivateKeySelinuxAttributesOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeySelinuxAttributesOk;
}

Certificate::CheckValues Certificate::arePublicKeySelinuxAttributesOk() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeySelinuxAttributesOk;
}

QString Certificate::outgoingServer() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_OutgoingServer;
}

Certificate::CheckValues Certificate::exist() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_Exist;
}

Certificate::CheckValues Certificate::isValid() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_IsValid;
}

Certificate::CheckValues Certificate::hasValidAuthority() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ValidAuthority;
}

Certificate::CheckValues Certificate::hasKnownAuthority() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_HasKnownAuthority;
}

Certificate::CheckValues Certificate::isNotRevoked() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_IsNotRevoked;
}

Certificate::CheckValues Certificate::authorityMatch() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_AuthorityMismatch;
}

Certificate::CheckValues Certificate::hasExpectedOwner() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_UnexpectedOwner;
}

QDateTime Certificate::expirationDate() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_ExpirationDate;
}

QDateTime Certificate::activationDate() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_ActivationDate;
}

bool Certificate::requirePrivateKeyPassword() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_RequirePrivateKeyPassword;
}

QByteArray Certificate::publicSignature() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_PublicSignature;
}

int Certificate::versionNumber() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_VersionNumber;
}

QByteArray Certificate::serialNumber() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_SerialNumber;
}

QString Certificate::issuer() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_Issuer;
}

QByteArray Certificate::subjectKeyAlgorithm() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_SubjectKeyAlgorithm;
}

QString Certificate::cn() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_Cn;
}

QString Certificate::n() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_N;
}

QString Certificate::o() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_O;
}

QByteArray Certificate::signatureAlgorithm() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_SignatureAlgorithm;
}

QByteArray Certificate::md5Fingerprint() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_Md5Fingerprint;
}

QByteArray Certificate::sha1Fingerprint() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_Sha1Fingerprint;
}

QByteArray Certificate::publicKeyId() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_PublicKeyId;
}

QByteArray Certificate::issuerDn() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_IssuerDn;
}

QDateTime Certificate::nextExpectedUpdateDate() const
{
   d_ptr->loadDetails();
   return d_ptr->m_pDetailsCache->m_NextExpectedUpdateDate;
}

bool Certificate::isActivated() const {
   d_ptr->loadDetails();
   return d_ptr->m_pCheckCache->m_NotActivated == Certificate::CheckValues::PASSED;
}


void Certificate::setPath(const QUrl& path)
{
   d_ptr->m_Path = path;
}

QUrl Certificate::path() const
{
   return d_ptr->m_Path;
}

Certificate::Type Certificate::type() const
{
   return d_ptr->m_Type;
}


QVariant Certificate::checkResults(Certificate::CertificateCheck check) const
{
   switch (check) {
      case CertificateCheck::HAS_PRIVATE_KEY                   : return static_cast<int>(hasPrivateKey                       ());
      case CertificateCheck::EXPIRED                           : return static_cast<int>(isNotExpired                        ());
      case CertificateCheck::STRONG_SIGNING                    : return static_cast<int>(hasStrongSigning                    ());
      case CertificateCheck::NOT_SELF_SIGNED                   : return static_cast<int>(isNotSelfSigned                     ());
      case CertificateCheck::KEY_MATCH                         : return static_cast<int>(privateKeyMatch                     ());
      case CertificateCheck::PRIVATE_KEY_STORAGE_PERMISSION    : return static_cast<int>(arePrivateKeyStoragePermissionOk    ());
      case CertificateCheck::PUBLIC_KEY_STORAGE_PERMISSION     : return static_cast<int>(arePublicKeyStoragePermissionOk     ());
      case CertificateCheck::PRIVATE_KEY_DIRECTORY_PERMISSIONS : return static_cast<int>(arePrivateKeyDirectoryPermissionsOk ());
      case CertificateCheck::PUBLIC_KEY_DIRECTORY_PERMISSIONS  : return static_cast<int>(arePublicKeyDirectoryPermissionsOk  ());
      case CertificateCheck::PRIVATE_KEY_STORAGE_LOCATION      : return static_cast<int>(arePrivateKeyStorageLocationOk      ());
      case CertificateCheck::PUBLIC_KEY_STORAGE_LOCATION       : return static_cast<int>(arePublicKeyStorageLocationOk       ());
      case CertificateCheck::PRIVATE_KEY_SELINUX_ATTRIBUTES    : return static_cast<int>(arePrivateKeySelinuxAttributesOk    ());
      case CertificateCheck::PUBLIC_KEY_SELINUX_ATTRIBUTES     : return static_cast<int>(arePublicKeySelinuxAttributesOk     ());
      case CertificateCheck::OUTGOING_SERVER                   : return outgoingServer();
      case CertificateCheck::EXIST                             : return static_cast<int>(exist                               ());
      case CertificateCheck::VALID                             : return static_cast<int>(isValid                             ());
      case CertificateCheck::VALID_AUTHORITY                   : return static_cast<int>(hasValidAuthority                   ());
      case CertificateCheck::KNOWN_AUTHORITY                   : return static_cast<int>(hasKnownAuthority                   ());
      case CertificateCheck::NOT_REVOKED                       : return static_cast<int>(isNotRevoked                        ());
      case CertificateCheck::AUTHORITY_MATCH                   : return static_cast<int>(authorityMatch                      ());
      case CertificateCheck::EXPECTED_OWNER                    : return static_cast<int>(hasExpectedOwner                    ());
      case CertificateCheck::ACTIVATED                         : return static_cast<int>(isActivated                         ());
      case CertificateCheck::COUNT__:
         Q_ASSERT(false);
   };
   return QVariant();
}

QVariant Certificate::detailResult(Certificate::CertificateDetails detail) const
{
   switch(detail) {
      case CertificateDetails::EXPIRATION_DATE                : expirationDate                       ();
      case CertificateDetails::ACTIVATION_DATE                : activationDate                       ();
      case CertificateDetails::REQUIRE_PRIVATE_KEY_PASSWORD   : requirePrivateKeyPassword            ();
      case CertificateDetails::PUBLIC_SIGNATURE               : publicSignature                      ();
      case CertificateDetails::VERSION_NUMBER                 : versionNumber                        ();
      case CertificateDetails::SERIAL_NUMBER                  : serialNumber                         ();
      case CertificateDetails::ISSUER                         : issuer                               ();
      case CertificateDetails::SUBJECT_KEY_ALGORITHM          : subjectKeyAlgorithm                  ();
      case CertificateDetails::CN                             : cn                                   ();
      case CertificateDetails::N                              : n                                    ();
      case CertificateDetails::O                              : o                                    ();
      case CertificateDetails::SIGNATURE_ALGORITHM            : signatureAlgorithm                   ();
      case CertificateDetails::MD5_FINGERPRINT                : md5Fingerprint                       ();
      case CertificateDetails::SHA1_FINGERPRINT               : sha1Fingerprint                      ();
      case CertificateDetails::PUBLIC_KEY_ID                  : publicKeyId                          ();
      case CertificateDetails::ISSUER_DN                      : issuerDn                             ();
      case CertificateDetails::NEXT_EXPECTED_UPDATE_DATE      : nextExpectedUpdateDate               ();
      case CertificateDetails::COUNT__:
         Q_ASSERT(false);
   };
   return QVariant();
}
