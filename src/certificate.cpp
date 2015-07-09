/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
 *                                                                          *
 *   This library is free software you can redistribute it and/or           *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation either            *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY without even the implied warranty of          *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "certificate.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QDateTime>

//STD
#include <stdint.h>

//Ring daemon
#include <security_const.h>

//Ring
#include "dbus/configurationmanager.h"
#include <certificatemodel.h>
#include "private/matrixutils.h"
#include "private/account_p.h"
#include "private/certificatemodel_p.h"
#include <account.h>

class DetailsCache {
public:
   DetailsCache(const MapStringString& details);

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
   QString    m_OutgoingServer            ;

};

class ChecksCache {
public:
   ChecksCache(const MapStringString& checks);

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
   Certificate::CheckValues m_Exist                               ;
   Certificate::CheckValues m_IsValid                             ;
   Certificate::CheckValues m_ValidAuthority                      ;
   Certificate::CheckValues m_HasKnownAuthority                   ;
   Certificate::CheckValues m_IsNotRevoked                        ;
   Certificate::CheckValues m_AuthorityMismatch                   ;
   Certificate::CheckValues m_UnexpectedOwner                     ;
   Certificate::CheckValues m_NotActivated                        ;
};

/**
 * Certificates can be loaded either from disk or directly from the
 * network sockets. In that case, they don't (always) need to be saved
 */
enum class LoadingType {
   FROM_PATH,
   FROM_ID
};

class CertificatePrivate
{
public:
   //Types
   typedef Certificate::CheckValues (Certificate::*accessor)();

   //Constructor
   CertificatePrivate(LoadingType _ltype);
   ~CertificatePrivate();

   //Attributes
   QUrl              m_Path                    ;
   Certificate::Type m_Type                    ;
   QByteArray        m_Content                 ;
   LoadingType       m_LoadingType             ;
   QByteArray        m_Id                      ;
   quint64           m_Statuses             [3];
   QUrl              m_PrivateKey              ;
   bool              m_RequirePrivateKey       ;
   bool              m_RequireStrictPermissions;

   mutable DetailsCache* m_pDetailsCache;
   mutable ChecksCache*  m_pCheckCache  ;

   //Helpers
   void loadDetails();
   void loadChecks (bool loadChecks = false);

   static Matrix1D<Certificate::Checks ,QString> m_slChecksName;
   static Matrix1D<Certificate::Checks ,QString> m_slChecksDescription;
   static Matrix1D<Certificate::Details,QString> m_slDetailssName;
   static Matrix1D<Certificate::Details,QString> m_slDetailssDescription;

   static Certificate::CheckValues toBool(const QString& string);
};

Matrix1D<Certificate::Checks ,QString> CertificatePrivate::m_slChecksName = {{
   /* HAS_PRIVATE_KEY                   */ QObject::tr("Has a private key"                               ),
   /* EXPIRED                           */ QObject::tr("Is not expired"                                  ),
   /* STRONG_SIGNING                    */ QObject::tr("Has strong signing"                              ),
   /* NOT_SELF_SIGNED                   */ QObject::tr("Is not self signed"                              ),
   /* KEY_MATCH                         */ QObject::tr("Have a matching key pair"                        ),
   /* PRIVATE_KEY_STORAGE_PERMISSION    */ QObject::tr("Has the right private key file permissions"      ),
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ QObject::tr("Has the right public key file permissions"       ),
   /* PRIVATE_KEY_DIRECTORY_PERMISSIONS */ QObject::tr("Has the right private key directory permissions" ),
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ QObject::tr("Has the right public key directory permissions"  ),
   /* PRIVATE_KEY_STORAGE_LOCATION      */ QObject::tr("Has the right private key directory location"    ),
   /* PUBLIC_KEY_STORAGE_LOCATION       */ QObject::tr("Has the right public key directory location"     ),
   /* PRIVATE_KEY_SELINUX_ATTRIBUTES    */ QObject::tr("Has the right private key SELinux attributes"    ),
   /* PUBLIC_KEY_SELINUX_ATTRIBUTES     */ QObject::tr("Has the right public key SELinux attributes"     ),
   /* EXIST                             */ QObject::tr("The certificate file exist and is readable"      ),
   /* VALID                             */ QObject::tr("The file is a valid certificate"                 ),
   /* VALID_AUTHORITY                   */ QObject::tr("The certificate has a valid authority"           ),
   /* KNOWN_AUTHORITY                   */ QObject::tr("The certificate has a known authority"           ),
   /* NOT_REVOKED                       */ QObject::tr("The certificate is not revoked"                  ),
   /* AUTHORITY_MATCH                   */ QObject::tr("The certificate authority match"                 ),
   /* EXPECTED_OWNER                    */ QObject::tr("The certificate has the expected owner"          ),
   /* ACTIVATED                         */ QObject::tr("The certificate is within its active period"     ),
}};

Matrix1D<Certificate::Checks ,QString> CertificatePrivate::m_slChecksDescription = {{
   /* HAS_PRIVATE_KEY                   */ "TODO",
   /* EXPIRED                           */ "TODO",
   /* STRONG_SIGNING                    */ "TODO",
   /* NOT_SELF_SIGNED                   */ "TODO",
   /* KEY_MATCH                         */ "TODO",
   /* PRIVATE_KEY_STORAGE_PERMISSION    */ "TODO",
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ "TODO",
   /* PRIVATE_KEY_DIRECTORY_PERMISSIONS */ "TODO",
   /* PUBLIC_KEY_DIRECTORY_PERMISSIONS  */ "TODO",
   /* PRIVATE_KEY_STORAGE_LOCATION      */ "TODO",
   /* PUBLIC_KEY_STORAGE_LOCATION       */ "TODO",
   /* PRIVATE_KEY_SELINUX_ATTRIBUTES    */ "TODO",
   /* PUBLIC_KEY_SELINUX_ATTRIBUTES     */ "TODO",
   /* EXIST                             */ "TODO",
   /* VALID                             */ "TODO",
   /* VALID_AUTHORITY                   */ "TODO",
   /* KNOWN_AUTHORITY                   */ "TODO",
   /* NOT_REVOKED                       */ "TODO",
   /* AUTHORITY_MATCH                   */ "TODO",
   /* EXPECTED_OWNER                    */ "TODO",
   /* ACTIVATED                         */ "TODO",
}};

Matrix1D<Certificate::Details,QString> CertificatePrivate::m_slDetailssName = {{
   /* EXPIRATION_DATE                */ QObject::tr("Expiration date"               ),
   /* ACTIVATION_DATE                */ QObject::tr("Activation date"               ),
   /* REQUIRE_PRIVATE_KEY_PASSWORD   */ QObject::tr("Require a private key password"),
   /* PUBLIC_SIGNATURE               */ QObject::tr("Public signature"              ),
   /* VERSION_NUMBER                 */ QObject::tr("Version"                       ),
   /* SERIAL_NUMBER                  */ QObject::tr("Serial number"                 ),
   /* ISSUER                         */ QObject::tr("Issuer"                        ),
   /* SUBJECT_KEY_ALGORITHM          */ QObject::tr("Subject key algorithm"         ),
   /* CN                             */ QObject::tr("Common name (CN)"              ),
   /* N                              */ QObject::tr("Name (N)"                      ),
   /* O                              */ QObject::tr("Organization (O)"              ),
   /* SIGNATURE_ALGORITHM            */ QObject::tr("Signature algorithm"           ),
   /* MD5_FINGERPRINT                */ QObject::tr("Md5 fingerprint"               ),
   /* SHA1_FINGERPRINT               */ QObject::tr("Sha1 fingerprint"              ),
   /* PUBLIC_KEY_ID                  */ QObject::tr("Public key id"                 ),
   /* ISSUER_DN                      */ QObject::tr("Issuer domain name"            ),
   /* NEXT_EXPECTED_UPDATE_DATE      */ QObject::tr("Next expected update"          ),
   /* OUTGOING_SERVER                */ QObject::tr("Outgoing server"               ),

}};

Matrix1D<Certificate::Details,QString> CertificatePrivate::m_slDetailssDescription = {{
   /* EXPIRATION_DATE                 */ "TODO",
   /* ACTIVATION_DATE                 */ "TODO",
   /* REQUIRE_PRIVATE_KEY_PASSWORD    */ "TODO",
   /* PUBLIC_SIGNATURE                */ "TODO",
   /* VERSION_NUMBER                  */ "TODO",
   /* SERIAL_NUMBER                   */ "TODO",
   /* ISSUER                          */ "TODO",
   /* SUBJECT_KEY_ALGORITHM           */ "TODO",
   /* CN                              */ "TODO",
   /* N                               */ "TODO",
   /* O                               */ "TODO",
   /* SIGNATURE_ALGORITHM             */ "TODO",
   /* MD5_FINGERPRINT                 */ "TODO",
   /* SHA1_FINGERPRINT                */ "TODO",
   /* PUBLIC_KEY_ID                   */ "TODO",
   /* ISSUER_DN                       */ "TODO",
   /* NEXT_EXPECTED_UPDATE_DATE       */ "TODO",
   /* OUTGOING_SERVER                 */ "TODO",

}};


CertificatePrivate::CertificatePrivate(LoadingType _type) :
m_pCheckCache(nullptr), m_pDetailsCache(nullptr), m_LoadingType(_type),
m_Statuses{0,0,0},m_RequirePrivateKey(false),m_RequireStrictPermissions(true)
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

DetailsCache::DetailsCache(const MapStringString& details)
{
   m_ExpirationDate             = QDateTime::fromString( details[DRing::Certificate::DetailsNames::EXPIRATION_DATE ],"yyyy-mm-dd");
   m_ActivationDate             = QDateTime::fromString( details[DRing::Certificate::DetailsNames::ACTIVATION_DATE ],"yyyy-mm-dd");
   m_RequirePrivateKeyPassword  = false;//TODO//details[DRing::Certificate::DetailsNames::REQUIRE_PRIVATE_KEY_PASSWORD].toBool();
   m_PublicSignature            = details[DRing::Certificate::DetailsNames::PUBLIC_SIGNATURE            ].toLatin1();
   m_VersionNumber              = details[DRing::Certificate::DetailsNames::VERSION_NUMBER              ].toInt();
   m_SerialNumber               = details[DRing::Certificate::DetailsNames::SERIAL_NUMBER               ].toLatin1();
   m_Issuer                     = details[DRing::Certificate::DetailsNames::ISSUER                      ];
   m_SubjectKeyAlgorithm        = details[DRing::Certificate::DetailsNames::SUBJECT_KEY_ALGORITHM       ].toLatin1();
   m_Cn                         = details[DRing::Certificate::DetailsNames::CN                          ];
   m_N                          = details[DRing::Certificate::DetailsNames::N                           ];
   m_O                          = details[DRing::Certificate::DetailsNames::O                           ];
   m_SignatureAlgorithm         = details[DRing::Certificate::DetailsNames::SIGNATURE_ALGORITHM         ].toLatin1();
   m_Md5Fingerprint             = details[DRing::Certificate::DetailsNames::MD5_FINGERPRINT             ].toLatin1();
   m_Sha1Fingerprint            = details[DRing::Certificate::DetailsNames::SHA1_FINGERPRINT            ].toLatin1();
   m_PublicKeyId                = details[DRing::Certificate::DetailsNames::PUBLIC_KEY_ID               ].toLatin1();
   m_IssuerDn                   = details[DRing::Certificate::DetailsNames::ISSUER_DN                   ].toLatin1();
   m_NextExpectedUpdateDate     = QDateTime::fromString(details[DRing::Certificate::DetailsNames::NEXT_EXPECTED_UPDATE_DATE ],"yyyy-mm-dd");
   m_OutgoingServer             = details[DRing::Certificate::DetailsNames::OUTGOING_SERVER             ] ;
}

ChecksCache::ChecksCache(const MapStringString& checks)
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
      MapStringString d;
      switch(m_LoadingType) {
         case LoadingType::FROM_PATH:
            d = DBus::ConfigurationManager::instance().getCertificateDetailsPath(m_Path.toString());
            break;
         case LoadingType::FROM_ID:
            d = DBus::ConfigurationManager::instance().getCertificateDetails(m_Id);
            break;
      }
      m_pDetailsCache = new DetailsCache(d);
   }
}

void CertificatePrivate::loadChecks(bool reload)
{
   if ((!m_pCheckCache) || reload) {
      MapStringString checks;
      switch(m_LoadingType) {
         case LoadingType::FROM_PATH:
            checks = DBus::ConfigurationManager::instance().validateCertificatePath(QString(),m_Path.toString(),m_PrivateKey.toString(), {});
            break;
         case LoadingType::FROM_ID:
            checks = DBus::ConfigurationManager::instance().validateCertificate(QString(),m_Id);
            break;
      }
      if (reload && m_pCheckCache)
         delete m_pCheckCache;
      m_pCheckCache = new ChecksCache(checks);
   }
}

Certificate::Certificate(const QUrl& path, Type type, const QUrl& privateKey) : ItemBase<QObject>(nullptr),d_ptr(new CertificatePrivate(LoadingType::FROM_PATH))
{
   Q_UNUSED(privateKey)
   moveToThread(CertificateModel::instance()->thread());
   setParent(CertificateModel::instance());
   d_ptr->m_Path = path.path();
   d_ptr->m_Type = type;
}

Certificate::Certificate(const QString& id) : ItemBase<QObject>(nullptr),d_ptr(new CertificatePrivate(LoadingType::FROM_ID))
{
   moveToThread(CertificateModel::instance()->thread());
   setParent(CertificateModel::instance());
   d_ptr->m_Id = id.toLatin1();
}

Certificate::~Certificate()
{
   delete d_ptr;
}

bool Certificate::hasRemote() const
{
   return ! d_ptr->m_Id.isEmpty();
}

QByteArray Certificate::remoteId() const
{
   return d_ptr->m_Id;
}

QString Certificate::getName(Certificate::Checks check)
{
   return CertificatePrivate::m_slChecksName[check];
}

QString Certificate::getName(Certificate::Details detail)
{
   return CertificatePrivate::m_slDetailssName[detail];
}

QString Certificate::getDescription(Certificate::Checks check)
{
   return CertificatePrivate::m_slChecksDescription[check];
}

QString Certificate::getDescription(Certificate::Details detail)
{
   return CertificatePrivate::m_slDetailssDescription[detail];
}

Certificate::CheckValues Certificate::hasPrivateKey() const
{
   d_ptr->loadChecks();

   if (!d_ptr->m_RequirePrivateKey)
      return Certificate::CheckValues::UNSUPPORTED;

   if (!d_ptr->m_PrivateKey.isEmpty())
      return Certificate::CheckValues::PASSED;

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
   if ((!d_ptr->m_RequirePrivateKey) || !d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeyStoragePermissionOk;
}

Certificate::CheckValues Certificate::arePublicKeyStoragePermissionOk() const
{
   if (!d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeyStoragePermissionOk;
}

Certificate::CheckValues Certificate::arePrivateKeyDirectoryPermissionsOk() const
{
   if ((!d_ptr->m_RequirePrivateKey) || !d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeyDirectoryPermissionsOk;
}

Certificate::CheckValues Certificate::arePublicKeyDirectoryPermissionsOk() const
{
   if (!d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

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
   if (!d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeyStorageLocationOk;
}

Certificate::CheckValues Certificate::arePrivateKeySelinuxAttributesOk() const
{
   if ((!d_ptr->m_RequirePrivateKey) || !d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePrivateKeySelinuxAttributesOk;
}

Certificate::CheckValues Certificate::arePublicKeySelinuxAttributesOk() const
{
   if (!d_ptr->m_RequireStrictPermissions)
      return Certificate::CheckValues::UNSUPPORTED;

   d_ptr->loadChecks();
   return d_ptr->m_pCheckCache->m_ArePublicKeySelinuxAttributesOk;
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
   if (!d_ptr->m_RequirePrivateKey)
      return false;

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

void Certificate::setPrivateKeyPath(const QUrl& path)
{
   d_ptr->m_PrivateKey = path;
   d_ptr->m_RequirePrivateKey = true;

   //Reload the checks if necessary
   if (d_ptr->m_pCheckCache)
      d_ptr->loadChecks(true);
}

QUrl Certificate::privateKeyPath() const
{
   return d_ptr->m_PrivateKey;
}

Certificate::Type Certificate::type() const
{
   return d_ptr->m_Type;
}

QString Certificate::outgoingServer() const
{
   d_ptr->loadChecks();
   return d_ptr->m_pDetailsCache->m_OutgoingServer;
}

///This attribute can be set if the private key will be required at some point
void Certificate::setRequirePrivateKey(bool value)
{
   d_ptr->m_RequirePrivateKey = value;
}

///This attribute can be set if the private key will be required at some point
bool Certificate::requirePrivateKey() const
{
   return d_ptr->m_RequirePrivateKey;
}

void Certificate::setRequireStrictPermission(bool value)
{
   d_ptr->m_RequireStrictPermissions = value;
}

bool Certificate::requireStrictPermission() const
{
   return d_ptr->m_RequireStrictPermissions;
}

Certificate::CheckValues Certificate::checkResult(Certificate::Checks check) const
{
   switch (check) {
      case Checks::HAS_PRIVATE_KEY                   : return hasPrivateKey                       ();
      case Checks::EXPIRED                           : return isNotExpired                        ();
      case Checks::STRONG_SIGNING                    : return hasStrongSigning                    ();
      case Checks::NOT_SELF_SIGNED                   : return isNotSelfSigned                     ();
      case Checks::KEY_MATCH                         : return privateKeyMatch                     ();
      case Checks::PRIVATE_KEY_STORAGE_PERMISSION    : return arePrivateKeyStoragePermissionOk    ();
      case Checks::PUBLIC_KEY_STORAGE_PERMISSION     : return arePublicKeyStoragePermissionOk     ();
      case Checks::PRIVATE_KEY_DIRECTORY_PERMISSIONS : return arePrivateKeyDirectoryPermissionsOk ();
      case Checks::PUBLIC_KEY_DIRECTORY_PERMISSIONS  : return arePublicKeyDirectoryPermissionsOk  ();
      case Checks::PRIVATE_KEY_STORAGE_LOCATION      : return arePrivateKeyStorageLocationOk      ();
      case Checks::PUBLIC_KEY_STORAGE_LOCATION       : return arePublicKeyStorageLocationOk       ();
      case Checks::PRIVATE_KEY_SELINUX_ATTRIBUTES    : return arePrivateKeySelinuxAttributesOk    ();
      case Checks::PUBLIC_KEY_SELINUX_ATTRIBUTES     : return arePublicKeySelinuxAttributesOk     ();
      case Checks::EXIST                             : return exist                               ();
      case Checks::VALID                             : return isValid                             ();
      case Checks::VALID_AUTHORITY                   : return hasValidAuthority                   ();
      case Checks::KNOWN_AUTHORITY                   : return hasKnownAuthority                   ();
      case Checks::NOT_REVOKED                       : return isNotRevoked                        ();
      case Checks::AUTHORITY_MATCH                   : return authorityMatch                      ();
      case Checks::EXPECTED_OWNER                    : return hasExpectedOwner                    ();
      case Checks::ACTIVATED                         : return static_cast<Certificate::CheckValues>(isActivated());
      case Checks::COUNT__:
         Q_ASSERT(false);
   };
   return Certificate::CheckValues::UNSUPPORTED;
}

QVariant Certificate::detailResult(Certificate::Details detail) const
{
   switch(detail) {
      case Details::EXPIRATION_DATE                : return expirationDate                ();
      case Details::ACTIVATION_DATE                : return activationDate                ();
      case Details::REQUIRE_PRIVATE_KEY_PASSWORD   : return requirePrivateKeyPassword     ();
      case Details::PUBLIC_SIGNATURE               : return publicSignature               ();
      case Details::VERSION_NUMBER                 : return versionNumber                 ();
      case Details::SERIAL_NUMBER                  : return serialNumber                  ();
      case Details::ISSUER                         : return issuer                        ();
      case Details::SUBJECT_KEY_ALGORITHM          : return subjectKeyAlgorithm           ();
      case Details::CN                             : return cn                            ();
      case Details::N                              : return n                             ();
      case Details::O                              : return o                             ();
      case Details::SIGNATURE_ALGORITHM            : return signatureAlgorithm            ();
      case Details::MD5_FINGERPRINT                : return md5Fingerprint                ();
      case Details::SHA1_FINGERPRINT               : return sha1Fingerprint               ();
      case Details::PUBLIC_KEY_ID                  : return publicKeyId                   ();
      case Details::ISSUER_DN                      : return issuerDn                      ();
      case Details::NEXT_EXPECTED_UPDATE_DATE      : return nextExpectedUpdateDate        ();
      case Details::OUTGOING_SERVER                : return outgoingServer                ();

      case Details::COUNT__:
         Q_ASSERT(false);
   };
   return QVariant();
}

/**
 * Get the details of this certificate as a QAbstractItemModel
 *
 * Please note that the object ownership will be transferred. To avoid memory
 * leaks, the users of this object must delete it once they are done with it.
 */
QAbstractItemModel* Certificate::model() const
{
   return CertificateModel::instance()->d_ptr->model(this);
}

/**
 * This model further reduce the data to only return the relevant certificate
 * checks.
 */
QAbstractItemModel* Certificate::checksModel() const
{
   return CertificateModel::instance()->d_ptr->checksModel(this);
}

///DEPRECATED
bool Certificate::setStatus(const Account* a, Status s)
{
   if (!a)
      return false;

   //This status is stored as a 3bit bitmask in 3 64bit integers

   const int maskId = a->d_ptr->internalId();
   const int position = (maskId*3)/64;
   const int offset   = (maskId*3)%64;

   //Only 63 accounts are currently supported
   if (position > 3)
      return false;

   d_ptr->m_Statuses[position] = static_cast<int>(s) << offset;

   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   //Notify the daemon
   if (hasRemote()) {
      configurationManager.setCertificateStatus(a->id(),d_ptr->m_Id,CertificateModelPrivate::m_StatusMap[s]);
   }
   else {
      //d_ptr->m_Id = configurationManager.pinCertificatePath(path());
      //TODO register the certificate in the daemon
   }

   return true;
}

///DEPRECATED
Certificate::Status Certificate::status(const Account* a) const
{
   const int maskId = a->d_ptr->internalId();
   const int position = (maskId*3)/64;
   const int offset   = (maskId*3)%64;

   if (position >= 3)
      return Certificate::Status::UNDEFINED;

   const int raw = (d_ptr->m_Statuses[position] >> offset) & 0b111;

   Q_ASSERT(raw < enum_class_size<Certificate::Status>());

   return static_cast<Certificate::Status>(raw);
}

bool Certificate::fixPermissions() const
{
#ifndef Q_OS_WIN
   if (d_ptr->m_LoadingType != LoadingType::FROM_PATH)
      return false;

   bool ret = true;

   QFile publicKey(d_ptr->m_Path.path());

   if (!publicKey.exists()) {
      qWarning() << "The public key" << d_ptr->m_Path.path() << "doesn't exist";
      ret &= false;
   }

   const bool publicperm = publicKey.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
   ret &= publicperm;

   if (!publicperm)
      qWarning() << "Setting the public key" << d_ptr->m_Path.path() << "permissions failed";

   if (!d_ptr->m_PrivateKey.isEmpty()) {
      QFile privateKey(d_ptr->m_PrivateKey.path());

      if (!privateKey.exists()) {
         qWarning() << "The private key" << d_ptr->m_PrivateKey.path() << "doesn't exist";
         ret &= false;
      }

      const bool privperm = privateKey.setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner);
      ret &= privperm;

      if (!privperm)
         qWarning() << "Setting the private key" << d_ptr->m_PrivateKey.path() << "permissions failed";
   }

   return ret;

#else
   return false;
#endif
}

bool Certificate::moveToDotCert() const
{
//TODO implement for OSX and Windows
#ifdef Q_OS_LINUX
   if (d_ptr->m_LoadingType != LoadingType::FROM_PATH)
      return false;

   bool ret = true;

   QFile publicKey(d_ptr->m_Path.path());

   if (!publicKey.exists()) {
      qWarning() << "The public key" << d_ptr->m_Path.path() << "doesn't exist";
      ret &= false;
   }

   QDir certDir(QDir::homePath()+".cert");

   if (!certDir.exists()) {
      const bool mk = QDir(QDir::homePath()).mkdir(".cert");
      if (!mk)
         qWarning() << "Creating" << (QDir::homePath()+"/.cert") << "failed";

      ret &= mk;
   }

   ret &= publicKey.rename(QString("/home/%1/.cert/%2/")
      .arg( QDir::homePath     () )
      .arg( publicKey.fileName () )
   );

   if (!d_ptr->m_PrivateKey.isEmpty()) {
      QFile privateKey(d_ptr->m_PrivateKey.path());

      if (!privateKey.exists()) {
         qWarning() << "The private key" << d_ptr->m_Path.path() << "doesn't exist";
         ret &= false;
      }

      ret &= privateKey.rename(QString("/home/%1/.cert/%2/")
         .arg( QDir::homePath      () )
         .arg( privateKey.fileName () )
      );
   }

   return ret;

#else
   return false;
#endif
}

#include <certificate.moc>
