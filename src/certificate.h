/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#ifndef CERTIFICATE_H
#define CERTIFICATE_H

#include "typedefs.h"

//Qt
#include <QUrl>

class CertificatePrivate;

/**
 * This class represent a conceptual certificate.
 */
class LIB_EXPORT Certificate : public QObject {
   Q_OBJECT

   friend class CertificateModel;
   friend class CertificateModelPrivate;
public:

   //Properties
   Q_PROPERTY(CheckValues hasPrivateKey                       READ hasPrivateKey                       )
   Q_PROPERTY(CheckValues isNotExpired                        READ isNotExpired                        )
   Q_PROPERTY(CheckValues hasStrongSigning                    READ hasStrongSigning                    )
   Q_PROPERTY(CheckValues isNotSelfSigned                     READ isNotSelfSigned                     )
   Q_PROPERTY(CheckValues privateKeyMatch                     READ privateKeyMatch                     )
   Q_PROPERTY(CheckValues arePrivateKeyStoragePermissionOk    READ arePrivateKeyStoragePermissionOk    )
   Q_PROPERTY(CheckValues arePublicKeyStoragePermissionOk     READ arePublicKeyStoragePermissionOk     )
   Q_PROPERTY(CheckValues arePrivateKeyDirectoryPermissionsOk READ arePrivateKeyDirectoryPermissionsOk )
   Q_PROPERTY(CheckValues arePublicKeyDirectoryPermissionsOk  READ arePublicKeyDirectoryPermissionsOk  )
   Q_PROPERTY(CheckValues arePrivateKeyStorageLocationOk      READ arePrivateKeyStorageLocationOk      )
   Q_PROPERTY(CheckValues arePublicKeyStorageLocationOk       READ arePublicKeyStorageLocationOk       )
   Q_PROPERTY(CheckValues arePrivateKeySelinuxAttributesOk    READ arePrivateKeySelinuxAttributesOk    )
   Q_PROPERTY(CheckValues arePublicKeySelinuxAttributesOk     READ arePublicKeySelinuxAttributesOk     )
   Q_PROPERTY(CheckValues exist                               READ exist                               )
   Q_PROPERTY(CheckValues isValid                             READ isValid                             )
   Q_PROPERTY(CheckValues hasValidAuthority                   READ hasValidAuthority                   )
   Q_PROPERTY(CheckValues hasKnownAuthority                   READ hasKnownAuthority                   )
   Q_PROPERTY(CheckValues isNotRevoked                        READ isNotRevoked                        )
   Q_PROPERTY(CheckValues authorityMatch                      READ authorityMatch                      )
   Q_PROPERTY(CheckValues hasExpectedOwner                    READ hasExpectedOwner                    )
   Q_PROPERTY(bool        isActivated                         READ isActivated                         )

   Q_PROPERTY(QDateTime  expirationDate                       READ expirationDate           )
   Q_PROPERTY(QDateTime  activationDate                       READ activationDate           )
   Q_PROPERTY(bool       requirePrivateKeyPassword            READ requirePrivateKeyPassword)
   Q_PROPERTY(QByteArray publicSignature                      READ publicSignature          )
   Q_PROPERTY(int        versionNumber                        READ versionNumber            )
   Q_PROPERTY(QByteArray serialNumber                         READ serialNumber             )
   Q_PROPERTY(QString    issuer                               READ issuer                   )
   Q_PROPERTY(QByteArray subjectKeyAlgorithm                  READ subjectKeyAlgorithm      )
   Q_PROPERTY(QString    cn                                   READ cn                       )
   Q_PROPERTY(QString    n                                    READ n                        )
   Q_PROPERTY(QString    o                                    READ o                        )
   Q_PROPERTY(QByteArray signatureAlgorithm                   READ signatureAlgorithm       )
   Q_PROPERTY(QByteArray md5Fingerprint                       READ md5Fingerprint           )
   Q_PROPERTY(QByteArray sha1Fingerprint                      READ sha1Fingerprint          )
   Q_PROPERTY(QByteArray publicKeyId                          READ publicKeyId              )
   Q_PROPERTY(QByteArray issuerDn                             READ issuerDn                 )
   Q_PROPERTY(QDateTime  nextExpectedUpdateDate               READ nextExpectedUpdateDate   )
   Q_PROPERTY(QString    outgoingServer                       READ outgoingServer           )


   //Structures
   enum class Type {
      AUTHORITY  ,
      USER       ,
      PRIVATE_KEY,
      CALL       ,
      NONE       ,
   };

   /**
   * @enum Checks All validation fields
   *
   */
   enum class Checks {
      HAS_PRIVATE_KEY                   , /** This certificate has a build in private key                          */
      EXPIRED                           , /** This certificate is past its expiration date                         */
      STRONG_SIGNING                    , /** This certificate has been signed with a brute-force-able method      */
      NOT_SELF_SIGNED                   , /** This certificate has been self signed                                */
      KEY_MATCH                         , /** The public and private keys provided don't match                     */
      PRIVATE_KEY_STORAGE_PERMISSION    , /** The file hosting the private key isn't correctly secured             */
      PUBLIC_KEY_STORAGE_PERMISSION     , /** The file hosting the public key isn't correctly secured              */
      PRIVATE_KEY_DIRECTORY_PERMISSIONS , /** The folder storing the private key isn't correctly secured           */
      PUBLIC_KEY_DIRECTORY_PERMISSIONS  , /** The folder storing the public key isn't correctly secured            */
      PRIVATE_KEY_STORAGE_LOCATION      , /** Some operating systems have extra policies for certificate storage   */
      PUBLIC_KEY_STORAGE_LOCATION       , /** Some operating systems have extra policies for certificate storage   */
      PRIVATE_KEY_SELINUX_ATTRIBUTES    , /** Some operating systems require keys to have extra attributes         */
      PUBLIC_KEY_SELINUX_ATTRIBUTES     , /** Some operating systems require keys to have extra attributes         */
      EXIST                             , /** The certificate file doesn't exist or is not accessible              */
      VALID                             , /** The file is not a certificate                                        */
      VALID_AUTHORITY                   , /** The claimed authority did not sign the certificate                   */
      KNOWN_AUTHORITY                   , /** Some operating systems provide a list of trusted authorities, use it */
      NOT_REVOKED                       , /** The certificate has been revoked by the authority                    */
      AUTHORITY_MATCH                   , /** The certificate and authority mismatch                               */
      EXPECTED_OWNER                    , /** The certificate has an expected owner                                */
      ACTIVATED                         , /** The certificate has not been activated yet                           */
      COUNT__,
   };

   /**
   * @enum Details Informative fields about a certificate
   */
   enum class Details {
      EXPIRATION_DATE                , /** The certificate expiration date                                      */
      ACTIVATION_DATE                , /** The certificate activation date                                      */
      REQUIRE_PRIVATE_KEY_PASSWORD   , /** Does the private key require a password                              */
      PUBLIC_SIGNATURE               ,
      VERSION_NUMBER                 ,
      SERIAL_NUMBER                  ,
      ISSUER                         ,
      SUBJECT_KEY_ALGORITHM          ,
      CN                             ,
      N                              ,
      O                              ,
      SIGNATURE_ALGORITHM            ,
      MD5_FINGERPRINT                ,
      SHA1_FINGERPRINT               ,
      PUBLIC_KEY_ID                  ,
      ISSUER_DN                      ,
      NEXT_EXPECTED_UPDATE_DATE      ,
      OUTGOING_SERVER                , /** The hostname/outgoing server used for this certificate               */

      COUNT__
   };

   /**
   * @enum CheckValuesType Categories of possible values for each Checks
   */
   enum class CheckValuesType {
      BOOLEAN,
      ISO_DATE,
      CUSTOM,
      NUMBER,
      COUNT__,
   };

   /**
   * @enum CheckValue possible values for check
   *
   * All boolean check use PASSED when the test result is positive and
   * FAILED when it is negative. All new check need to keep this convention
   * or ::isValid() result will become unrepresentative of the real state.
   *
   * CUSTOM should be avoided when possible. This enum can be extended when
   * new validated types are required.
   */
   enum class CheckValues {
      FAILED     , /** Equivalent of a boolean "false"                                   */
      PASSED     , /** Equivalent of a boolean "true"                                    */
      UNSUPPORTED, /** The operating system doesn't support or require the check         */
      COUNT__,
   };

   //Getter
   QUrl path() const;
   Certificate::Type type() const;
   Certificate::CheckValues checkResult (Certificate::Checks check  ) const;
   QVariant detailResult(Certificate::Details detail) const;

   QString getName        (Certificate::Checks   check  );
   QString getName        (Certificate::Details details );
   QString getDescription (Certificate::Checks   check  );
   QString getDescription (Certificate::Details details );

   //Checks
   CheckValues hasPrivateKey                       () const;
   CheckValues isNotExpired                        () const;
   CheckValues hasStrongSigning                    () const;
   CheckValues isNotSelfSigned                     () const;
   CheckValues privateKeyMatch                     () const;
   CheckValues arePrivateKeyStoragePermissionOk    () const;
   CheckValues arePublicKeyStoragePermissionOk     () const;
   CheckValues arePrivateKeyDirectoryPermissionsOk () const;
   CheckValues arePublicKeyDirectoryPermissionsOk  () const;
   CheckValues arePrivateKeyStorageLocationOk      () const;
   CheckValues arePublicKeyStorageLocationOk       () const;
   CheckValues arePrivateKeySelinuxAttributesOk    () const;
   CheckValues arePublicKeySelinuxAttributesOk     () const;
   CheckValues exist                               () const;
   CheckValues isValid                             () const;
   CheckValues hasValidAuthority                   () const;
   CheckValues hasKnownAuthority                   () const;
   CheckValues isNotRevoked                        () const;
   CheckValues authorityMatch                      () const;
   CheckValues hasExpectedOwner                    () const;
   bool        isActivated                         () const;

   //Details
   QDateTime  expirationDate                       () const;
   QDateTime  activationDate                       () const;
   bool       requirePrivateKeyPassword            () const;
   QByteArray publicSignature                      () const;
   int        versionNumber                        () const;
   QByteArray serialNumber                         () const;
   QString    issuer                               () const;
   QByteArray subjectKeyAlgorithm                  () const;
   QString    cn                                   () const;
   QString    n                                    () const;
   QString    o                                    () const;
   QByteArray signatureAlgorithm                   () const;
   QByteArray md5Fingerprint                       () const;
   QByteArray sha1Fingerprint                      () const;
   QByteArray publicKeyId                          () const;
   QByteArray issuerDn                             () const;
   QDateTime  nextExpectedUpdateDate               () const;
   QString    outgoingServer                       () const;


   //Setter
   void setPath(const QUrl& path);

private:
   explicit Certificate(const QUrl& path, Type type = Type::NONE, const QUrl& privateKey = QUrl());
   Certificate(const QByteArray& content, Type type = Type::CALL);
   virtual ~Certificate();
   CertificatePrivate* d_ptr;

Q_SIGNALS:
   ///This certificate changed, all users need to reload it
   void changed();
};
Q_DECLARE_METATYPE(Certificate*)

#endif
