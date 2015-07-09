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
#ifndef CERTIFICATEPRIVATE_H
#define CERTIFICATEPRIVATE_H

//Qt
#include <QtCore/QDateTime>

//Ring
#include <securityevaluationmodel.h>

/**
 * Certificates can be loaded either from disk or directly from the
 * network sockets. In that case, they don't (always) need to be saved
 */
enum class LoadingType {
   FROM_PATH,
   FROM_CONTENT,
   FROM_ID
};

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

class CertificatePrivate
{
public:
   //Types
   typedef Certificate::CheckValues (Certificate::*accessor)();

   //Constructor
   CertificatePrivate(Certificate* parent, LoadingType _ltype);
   ~CertificatePrivate();

   //Attributes
   QUrl                m_Path                    ;
   Certificate::Type   m_Type                    ;
   QByteArray          m_Content                 ;
   LoadingType         m_LoadingType             ;
   QByteArray          m_Id                      ;
   quint64             m_Statuses             [3];
   QUrl                m_PrivateKey              ;
   bool                m_RequirePrivateKey       ;
   bool                m_RequireStrictPermissions;
   Certificate*        m_pSignedBy               ;
   QAbstractItemModel* m_pSeverityProxy          ;
   ChainOfTrustModel*  m_pChainOfTrust           ;
   FlagPack<Certificate::OriginHint> m_fHints    ;

   //Caching
   /* The certificate doesn't handle the security evaluation. This is
    * SecurityEvaluationModel job. However, it sill have to be kept
    * somewhere or it will be recomputed every time ::data() is called on
    * a model.
    */
   SecurityEvaluationModel::SecurityLevel m_SecurityLevelWithPriv    {SecurityEvaluationModel::SecurityLevel::NONE};
   SecurityEvaluationModel::SecurityLevel m_SecurityLevelWithoutPriv {SecurityEvaluationModel::SecurityLevel::NONE};
   bool m_hasLoadedSecurityLevel {false};

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

private:
   Certificate* q_ptr;
};

#endif
