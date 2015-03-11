/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                           *
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
#include "securityvalidationmodel.h"
#include "account.h"
#include "delegates/pixmapmanipulationdelegate.h"
#include "private/securityvalidationmodel_p.h"
#include "securityflaw.h"
#include "private/securityflaw_p.h"

#include <QtAlgorithms>

const QString SecurityValidationModelPrivate::messages[enum_class_size<SecurityValidationModel::AccountSecurityFlaw>()] = {
   QObject::tr("Your communication negotiation is secured, but not the media stream, please enable ZRTP or SDES"),
   QObject::tr("TLS is disabled, the negotiation wont be encrypted. Your communication will be vulnerable to "
   "snooping"),
   QObject::tr("Your certificate is expired, please contact your system administrator."),
   QObject::tr("Your certificate is self signed. This break the chain of trust."),
   QObject::tr("CA_CERTIFICATE_MISSING         "),
   QObject::tr("END_CERTIFICATE_MISSING        "),
   QObject::tr("None of your certificate provide a private key, this is required. Please select a private key"
   " or use a certificate with one built-in"),
   QObject::tr("CERTIFICATE_MISMATCH           "),
   QObject::tr("CERTIFICATE_STORAGE_PERMISSION "),
   QObject::tr("CERTIFICATE_STORAGE_FOLDER     "),
   QObject::tr("CERTIFICATE_STORAGE_LOCATION   "),
   QObject::tr("OUTGOING_SERVER_MISMATCH       "),
   QObject::tr("VERIFY_INCOMING_DISABLED       "),
   QObject::tr("VERIFY_ANSWER_DISABLED         "),
   QObject::tr("REQUIRE_CERTIFICATE_DISABLED   "),
};

const TypedStateMachine< SecurityValidationModel::SecurityLevel , SecurityValidationModel::AccountSecurityFlaw >
SecurityValidationModelPrivate::maximumSecurityLevel = {{
   /* SRTP_DISABLED                  */ SecurityValidationModel::SecurityLevel::WEAK       ,
   /* TLS_DISABLED                   */ SecurityValidationModel::SecurityLevel::WEAK       ,
   /* CERTIFICATE_EXPIRED            */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* CERTIFICATE_SELF_SIGNED        */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* CA_CERTIFICATE_MISSING         */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* END_CERTIFICATE_MISSING        */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* PRIVATE_KEY_MISSING            */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* CERTIFICATE_MISMATCH           */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* CERTIFICATE_STORAGE_PERMISSION */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* CERTIFICATE_STORAGE_FOLDER     */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* CERTIFICATE_STORAGE_LOCATION   */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* OUTGOING_SERVER_MISMATCH       */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* VERIFY_INCOMING_DISABLED       */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* VERIFY_ANSWER_DISABLED         */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* REQUIRE_CERTIFICATE_DISABLED   */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* MISSING_CERTIFICATE            */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* MISSING_AUTHORITY              */ SecurityValidationModel::SecurityLevel::WEAK       ,
}};

const TypedStateMachine< SecurityValidationModel::Severity , SecurityValidationModel::AccountSecurityFlaw >
SecurityValidationModelPrivate::flawSeverity = {{
   /* SRTP_DISABLED                  */ SecurityValidationModel::Severity::ISSUE   ,
   /* TLS_DISABLED                   */ SecurityValidationModel::Severity::ISSUE   ,
   /* CERTIFICATE_EXPIRED            */ SecurityValidationModel::Severity::WARNING ,
   /* CERTIFICATE_SELF_SIGNED        */ SecurityValidationModel::Severity::WARNING ,
   /* CA_CERTIFICATE_MISSING         */ SecurityValidationModel::Severity::ISSUE   ,
   /* END_CERTIFICATE_MISSING        */ SecurityValidationModel::Severity::ISSUE   ,
   /* PRIVATE_KEY_MISSING            */ SecurityValidationModel::Severity::ERROR   ,
   /* CERTIFICATE_MISMATCH           */ SecurityValidationModel::Severity::ERROR   ,
   /* CERTIFICATE_STORAGE_PERMISSION */ SecurityValidationModel::Severity::WARNING ,
   /* CERTIFICATE_STORAGE_FOLDER     */ SecurityValidationModel::Severity::INFORMATION ,
   /* CERTIFICATE_STORAGE_LOCATION   */ SecurityValidationModel::Severity::INFORMATION ,
   /* OUTGOING_SERVER_MISMATCH       */ SecurityValidationModel::Severity::WARNING ,
   /* VERIFY_INCOMING_DISABLED       */ SecurityValidationModel::Severity::ISSUE   ,
   /* VERIFY_ANSWER_DISABLED         */ SecurityValidationModel::Severity::ISSUE   ,
   /* REQUIRE_CERTIFICATE_DISABLED   */ SecurityValidationModel::Severity::ISSUE   ,
   /* MISSING_CERTIFICATE            */ SecurityValidationModel::Severity::ERROR   ,
   /* MISSING_AUTHORITY              */ SecurityValidationModel::Severity::ERROR   ,
}};

SecurityValidationModelPrivate::SecurityValidationModelPrivate(Account* account, SecurityValidationModel* parent) : q_ptr(parent),
m_pAccount(account), m_CurrentSecurityLevel(SecurityValidationModel::SecurityLevel::NONE)
{
   
}

SecurityValidationModel::SecurityValidationModel(Account* account) : QAbstractListModel(account),
d_ptr(new SecurityValidationModelPrivate(account,this))
{
   
}

SecurityValidationModel::~SecurityValidationModel()
{
   
}

QHash<int,QByteArray> SecurityValidationModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[Role::SeverityRole] = "SeverityRole";
   }
   return roles;
}

QVariant SecurityValidationModel::data( const QModelIndex& index, int role) const
{
   if (index.isValid())  {
      if (role == Qt::DisplayRole) {
         return SecurityValidationModelPrivate::messages[static_cast<int>( d_ptr->m_lCurrentFlaws[index.row()]->flaw() )];
      }
      else if (role == Role::SeverityRole) {
         return static_cast<int>(d_ptr->m_lCurrentFlaws[index.row()]->severity());
      }
      else if (role == Qt::DecorationRole) {
         return PixmapManipulationDelegate::instance()->serurityIssueIcon(index);
      }
   }
   return QVariant();
}

int SecurityValidationModel::rowCount( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return d_ptr->m_lCurrentFlaws.size();
}

Qt::ItemFlags SecurityValidationModel::flags( const QModelIndex& index) const
{
   if (!index.isValid()) return Qt::NoItemFlags;
   return Qt::ItemIsEnabled|Qt::ItemIsSelectable;
}

bool SecurityValidationModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

///Do some unsafe convertions to bypass Qt4 issues with C++11 enum class
SecurityFlaw* SecurityValidationModelPrivate::getFlaw(SecurityValidationModel::AccountSecurityFlaw _se, Certificate::Type _ty)
{
   if (! m_hFlaws[(int)_se][(int)_ty]) {
      m_hFlaws[(int)_se][(int)_ty] = new SecurityFlaw(_se,_ty);
   }
   return m_hFlaws[(int)_se][(int)_ty];
}

#define _F(_se,_ty) getFlaw(SecurityValidationModel::AccountSecurityFlaw::_se,_ty);
void SecurityValidationModelPrivate::update()
{
   m_lCurrentFlaws.clear();

   /**********************************
    *     Check general issues       *
    *********************************/

   /* If TLS is not enabled, everything else is worthless */
   if (!m_pAccount->isTlsEnabled()) {
      m_lCurrentFlaws << _F(TLS_DISABLED,Certificate::Type::NONE);
   }

   /* Check if the media stream is encrypted, it is something users
    * may care about if they get this far ;) */
   if (!m_pAccount->isSrtpEnabled()) {
      m_lCurrentFlaws << _F(SRTP_DISABLED,Certificate::Type::NONE);
   }

   /* The user certificate need to have a private key, otherwise it wont
    * be possible to encrypt anything */
   if (( m_pAccount->tlsCertificate() && m_pAccount->tlsCertificate()->hasPrivateKey() == Certificate::CheckValues::FAILED) && (m_pAccount->tlsPrivateKeyCertificate() && m_pAccount->tlsPrivateKeyCertificate()->exist()  == Certificate::CheckValues::FAILED)) {
      m_lCurrentFlaws << _F(PRIVATE_KEY_MISSING,m_pAccount->tlsPrivateKeyCertificate()->type());
   }

   /**********************************
    *      Certificates issues       *
    *********************************/
   QList<Certificate*> certs;
   certs << m_pAccount->tlsCaListCertificate() << m_pAccount->tlsCertificate() << m_pAccount->tlsPrivateKeyCertificate();
   foreach (Certificate* cert, certs) {
      if (cert->exist() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(END_CERTIFICATE_MISSING,cert->type());
      }
      if (cert->isNotExpired() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(CERTIFICATE_EXPIRED,cert->type());
      }
      if (cert->isNotSelfSigned() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(CERTIFICATE_SELF_SIGNED,cert->type());
      }
      if (cert->arePrivateKeyStoragePermissionOk() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(CERTIFICATE_STORAGE_PERMISSION,cert->type());
      }
      if (cert->arePublicKeyStoragePermissionOk() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(CERTIFICATE_STORAGE_PERMISSION,cert->type());
      }
      if (cert->arePrivateKeyDirectoryPermissionsOk() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(CERTIFICATE_STORAGE_FOLDER,cert->type());
      }
      if (cert->arePrivateKeyStorageLocationOk() == Certificate::CheckValues::FAILED) {
         m_lCurrentFlaws << _F(CERTIFICATE_STORAGE_LOCATION,cert->type());
      }
   }

   qSort(m_lCurrentFlaws.begin(),m_lCurrentFlaws.end(),[] (const SecurityFlaw* f1, const SecurityFlaw* f2) -> int {
      return (*f1) < (*f2);
   });
   for (int i=0;i<m_lCurrentFlaws.size();i++) {
      m_lCurrentFlaws[i]->d_ptr->m_Row = i;
   }

   emit q_ptr->layoutChanged();
}
#undef _F

QModelIndex SecurityValidationModel::getIndex(const SecurityFlaw* flaw)
{
   return index(flaw->d_ptr->m_Row,0);
}

QList<SecurityFlaw*> SecurityValidationModel::currentFlaws()
{
   return d_ptr->m_lCurrentFlaws;
}

void SecurityValidationModel::setTlsCaListCertificate( Certificate* cert )
{
   d_ptr->m_pCa = cert;
}

void SecurityValidationModel::setTlsCertificate( Certificate* cert )
{
   d_ptr->m_pCert = cert;
}

void SecurityValidationModel::setTlsPrivateKeyCertificate( Certificate* cert )
{
   d_ptr->m_pPrivateKey = cert;
}
