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

//Qt
#include <QtCore/QIdentityProxyModel>

//Ring
#include "account.h"
#include "certificatemodel.h"
#include "delegates/pixmapmanipulationdelegate.h"
#include "private/securityvalidationmodel_p.h"
#include "securityflaw.h"
#include "private/securityflaw_p.h"

#include <QtAlgorithms>

const QString SecurityValidationModelPrivate::messages[enum_class_size<SecurityValidationModel::AccountSecurityFlaw>()] = {
   /*SRTP_ENABLED                */QObject::tr("Your communication negotiation is secured, but not the media stream, please enable ZRTP or SDES"),
   /*TLS_ENABLED                 */QObject::tr("TLS is disabled, the negotiation wont be encrypted. Your communication will be vulnerable to "
                                   "snooping"),
   /*CERTIFICATE_MATCH           */QObject::tr("Your certificate and authority don't match, if your certificate require an authority, it wont work"),
   /*OUTGOING_SERVER_MATCH       */QObject::tr("The outgoring server specified doesn't match the hostname or the one included in the certificate"),
   /*VERIFY_INCOMING_ENABLED     */QObject::tr("The \"verify incoming certificate\" option is disabled, this leave you vulnarable to man in the middle attack"),
   /*VERIFY_ANSWER_ENABLED       */QObject::tr("The \"verify answer certificate\" option is disabled, this leave you vulnarable to man in the middle attack"),
   /*REQUIRE_CERTIFICATE_ENABLED */QObject::tr("None of your certificate provide a private key, this is required. Please select a private key"
                                       " or use a certificate with one built-in"),
   /*NOT_MISSING_CERTIFICATE     */QObject::tr("No certificate authority is provided, it wont be possible to validate if the answer certificates are valid. Some account may also not work."),
   /*NOT_MISSING_CERTIFICATE     */QObject::tr("No certificate has been provided. This is, for now, unsupported by Ring"),
};

static const QString s1 = QObject::tr("Your certificate is expired, please contact your system administrator.");
static const QString s2 = QObject::tr("Your certificate is self signed. This break the chain of trust.");

const TypedStateMachine< SecurityValidationModel::SecurityLevel , SecurityValidationModel::AccountSecurityFlaw >
SecurityValidationModelPrivate::maximumSecurityLevel = {{
   /* SRTP_ENABLED                     */ SecurityValidationModel::SecurityLevel::NONE        ,
   /* TLS_ENABLED                      */ SecurityValidationModel::SecurityLevel::NONE        ,
   /* CERTIFICATE_MATCH                */ SecurityValidationModel::SecurityLevel::WEAK        ,
   /* OUTGOING_SERVER_MATCH            */ SecurityValidationModel::SecurityLevel::MEDIUM      ,
   /* VERIFY_INCOMING_ENABLED          */ SecurityValidationModel::SecurityLevel::MEDIUM      ,
   /* VERIFY_ANSWER_ENABLED            */ SecurityValidationModel::SecurityLevel::MEDIUM      ,
   /* REQUIRE_CERTIFICATE_ENABLED      */ SecurityValidationModel::SecurityLevel::WEAK        ,
   /* NOT_MISSING_CERTIFICATE          */ SecurityValidationModel::SecurityLevel::WEAK        ,
   /* NOT_MISSING_AUTHORITY            */ SecurityValidationModel::SecurityLevel::WEAK        ,
}};

const TypedStateMachine< SecurityValidationModel::Severity , SecurityValidationModel::AccountSecurityFlaw >
SecurityValidationModelPrivate::flawSeverity = {{
   /* SRTP_ENABLED                      */ SecurityValidationModel::Severity::ISSUE           ,
   /* TLS_ENABLED                       */ SecurityValidationModel::Severity::ISSUE           ,
   /* CERTIFICATE_MATCH                 */ SecurityValidationModel::Severity::ERROR           ,
   /* OUTGOING_SERVER_MATCH             */ SecurityValidationModel::Severity::WARNING         ,
   /* VERIFY_INCOMING_ENABLED           */ SecurityValidationModel::Severity::ISSUE           ,
   /* VERIFY_ANSWER_ENABLED             */ SecurityValidationModel::Severity::ISSUE           ,
   /* REQUIRE_CERTIFICATE_ENABLED       */ SecurityValidationModel::Severity::ISSUE           ,
   /* NOT_MISSING_CERTIFICATE           */ SecurityValidationModel::Severity::WARNING         ,
   /* NOT_MISSING_AUTHORITY             */ SecurityValidationModel::Severity::ISSUE           ,
}};

const TypedStateMachine< SecurityValidationModel::SecurityLevel , Certificate::Checks > SecurityValidationModelPrivate::maximumCertificateSecurityLevel = {{
   /* HAS_PRIVATE_KEY                   */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* EXPIRED                           */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* STRONG_SIGNING                    */ SecurityValidationModel::SecurityLevel::WEAK       ,
   /* NOT_SELF_SIGNED                   */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* KEY_MATCH                         */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* PRIVATE_KEY_STORAGE_PERMISSION    */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* PRIVATE_KEY_DIRECTORY_PERMISSIONS */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* PUBLIC_KEY_DIRECTORY_PERMISSIONS  */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* PRIVATE_KEY_STORAGE_LOCATION      */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* PUBLIC_KEY_STORAGE_LOCATION       */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* PRIVATE_KEY_SELINUX_ATTRIBUTES    */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* PUBLIC_KEY_SELINUX_ATTRIBUTES     */ SecurityValidationModel::SecurityLevel::ACCEPTABLE ,
   /* EXIST                             */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* VALID                             */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* VALID_AUTHORITY                   */ SecurityValidationModel::SecurityLevel::MEDIUM     ,
   /* KNOWN_AUTHORITY                   */ SecurityValidationModel::SecurityLevel::ACCEPTABLE , //?
   /* NOT_REVOKED                       */ SecurityValidationModel::SecurityLevel::WEAK       ,
   /* AUTHORITY_MATCH                   */ SecurityValidationModel::SecurityLevel::NONE       ,
   /* EXPECTED_OWNER                    */ SecurityValidationModel::SecurityLevel::MEDIUM     , //?
   /* ACTIVATED                         */ SecurityValidationModel::SecurityLevel::MEDIUM     , //?
}};

const TypedStateMachine< SecurityValidationModel::Severity      , Certificate::Checks > SecurityValidationModelPrivate::certificateFlawSeverity = {{
   /* HAS_PRIVATE_KEY                   */ SecurityValidationModel::Severity::ERROR           ,
   /* EXPIRED                           */ SecurityValidationModel::Severity::WARNING         ,
   /* STRONG_SIGNING                    */ SecurityValidationModel::Severity::ISSUE           ,
   /* NOT_SELF_SIGNED                   */ SecurityValidationModel::Severity::WARNING         ,
   /* KEY_MATCH                         */ SecurityValidationModel::Severity::ERROR           ,
   /* PRIVATE_KEY_STORAGE_PERMISSION    */ SecurityValidationModel::Severity::WARNING         ,
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ SecurityValidationModel::Severity::WARNING         ,
   /* PRIVATE_KEY_DIRECTORY_PERMISSIONS */ SecurityValidationModel::Severity::WARNING         ,
   /* PUBLIC_KEY_DIRECTORY_PERMISSIONS  */ SecurityValidationModel::Severity::WARNING         ,
   /* PRIVATE_KEY_STORAGE_LOCATION      */ SecurityValidationModel::Severity::INFORMATION     ,
   /* PUBLIC_KEY_STORAGE_LOCATION       */ SecurityValidationModel::Severity::INFORMATION     ,
   /* PRIVATE_KEY_SELINUX_ATTRIBUTES    */ SecurityValidationModel::Severity::INFORMATION     ,
   /* PUBLIC_KEY_SELINUX_ATTRIBUTES     */ SecurityValidationModel::Severity::INFORMATION     ,
   /* EXIST                             */ SecurityValidationModel::Severity::ERROR           ,
   /* VALID                             */ SecurityValidationModel::Severity::ERROR           ,
   /* VALID_AUTHORITY                   */ SecurityValidationModel::Severity::WARNING         ,
   /* KNOWN_AUTHORITY                   */ SecurityValidationModel::Severity::WARNING         ,
   /* NOT_REVOKED                       */ SecurityValidationModel::Severity::ISSUE           ,
   /* AUTHORITY_MATCH                   */ SecurityValidationModel::Severity::ISSUE           ,
   /* EXPECTED_OWNER                    */ SecurityValidationModel::Severity::WARNING         ,
   /* ACTIVATED                         */ SecurityValidationModel::Severity::WARNING         ,
}};


/**
 * This class add a prefix in front of Qt::DisplayRole to add a disambiguation
 * when there is multiple certificates in the same SecurityValidationModel and
 * also add new roles such as the severity, BackgroundRole and DecorationRole
 */
class PrefixAndSeverityProxyModel : public QIdentityProxyModel
{
   Q_OBJECT

public:

   explicit PrefixAndSeverityProxyModel(const QString& prefix,QAbstractItemModel* parent);

   virtual QModelIndex index       ( int row                  , int column, const QModelIndex& parent ) const override;
   virtual QVariant    data        ( const QModelIndex& index , int role                              ) const override;
   virtual int         columnCount ( const QModelIndex& parent                                        ) const override;

   //Attributes
   QString m_Name;
};

/**
 * This model transform accounts attributes into security checks to validate if
 * some options reduce the security level.
 */
class AccountChecksModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   AccountChecksModel(const Account* a);

   //Model functions
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                 ) const override;
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

private:
   //Attributes
   const Account* m_pAccount;
   Matrix1D<SecurityValidationModel::AccountSecurityFlaw, Certificate::CheckValues> m_lCachedResults;

   //Helpers
   void update();
};

/**
 * This model take multiple listModels and append them one after the other
 * 
 * the trick for high performance is to known at compile time the sizes
 */
class CombinaisonProxyModel : public QAbstractTableModel
{
   Q_OBJECT

public:
   explicit CombinaisonProxyModel( QAbstractItemModel* publicCert ,
                                   QAbstractItemModel* caCert     ,
                                   QAbstractItemModel* account    ,
                                   QObject*            parent
                                 );

   //Model functions
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                 ) const override;
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

private:
   QVector<QAbstractItemModel*> m_lSources;

   //All source model, in order
   enum Src {
      CA = 0,
      PK = 1,
      AC = 2,
      ER = 3, //TODO
   };

   //This model expect a certain size, get each sections size
   constexpr static const short sizes[] = {
      enum_class_size< Certificate             :: Checks              > (),
      enum_class_size< Certificate             :: Checks              > (),
      enum_class_size< SecurityValidationModel :: AccountSecurityFlaw > (),
   };

   //Get the combined size
   constexpr inline static int totalSize() {
      return sizes[CA] + sizes[PK] + sizes[AC];
   }

   //Get a modex index from a value
   constexpr inline static int toModelIdx(const int value) {
      return (value >= sizes[CA] + sizes[PK] ?  AC : (
              value >= sizes[PK]             ?  PK :
                                                CA ) );
   }

   //Compute the correct index.row() offset
   constexpr inline static int fromFinal(const int value) {
      return (value >= sizes[CA] + sizes[PK] ?  value - sizes[CA] - sizes[PK] : (
              value >= sizes[PK]             ?  value - sizes[CA]             :
                                                value                         ) );
   }

};

SecurityValidationModelPrivate::SecurityValidationModelPrivate(Account* account, SecurityValidationModel* parent) :
q_ptr(parent), m_pAccount(account), m_CurrentSecurityLevel(SecurityValidationModel::SecurityLevel::NONE)
{
}



/*******************************************************************************
 *                                                                             *
 *                       PrefixAndSeverityProxyModel                           *
 *                                                                             *
 ******************************************************************************/

PrefixAndSeverityProxyModel::PrefixAndSeverityProxyModel(const QString& prefix, QAbstractItemModel* parent) :
   QIdentityProxyModel(parent),m_Name(prefix)
{
   setSourceModel(parent);
}

///It insert a second column with the source name
int PrefixAndSeverityProxyModel::columnCount( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return parent.isValid() ? 0 : 3;
}

QModelIndex PrefixAndSeverityProxyModel::index( int row, int column, const QModelIndex& parent) const
{
   if (column == 2)
      return createIndex(row,column);
   return QIdentityProxyModel::index(row,column,parent);
}

///Map items and add elements
QVariant PrefixAndSeverityProxyModel::data(const QModelIndex& index, int role) const
{
   if (index.isValid()) {

      Certificate::Checks c;
      if (QIdentityProxyModel::data(index,(int)CertificateModel::Role::isCheck).toBool() == true)
         c = qvariant_cast<Certificate::Checks>(QIdentityProxyModel::data(index,(int)CertificateModel::Role::check));
      else if (index.column() != 2) //That column doesn't exist in the source, the wont exist
         return QVariant();

      switch (index.column()) {
         case 0:
            switch(role) {
               case Qt::DecorationRole:
                  return PixmapManipulationDelegate::instance()->securityIssueIcon(index);
               case (int)SecurityValidationModel::Role::Severity:
                  return QVariant::fromValue(SecurityValidationModelPrivate::certificateFlawSeverity[c]);
            }
            break;
         //
         case 1: {
            switch(role) {
               case Qt::DisplayRole:
                  return m_Name;
               case (int)SecurityValidationModel::Role::Severity:
                  return QVariant::fromValue(SecurityValidationModelPrivate::certificateFlawSeverity[c]);
            }
            return QVariant();
         }
            break;
         //Map source column 1 to 2
         case 2: {
            const QModelIndex& srcIdx = sourceModel()->index(index.row(),1);
            c = qvariant_cast<Certificate::Checks>(srcIdx.data((int)CertificateModel::Role::check));

            switch(role) {
               case (int)SecurityValidationModel::Role::Severity:
                  return QVariant::fromValue(SecurityValidationModelPrivate::certificateFlawSeverity[c]);
            }

            return srcIdx.data(role);
         }
      }
   }

   return QIdentityProxyModel::data(index,role);
}



/*******************************************************************************
 *                                                                             *
 *                             AccountChecksModel                              *
 *                                                                             *
 ******************************************************************************/

AccountChecksModel::AccountChecksModel(const Account* a) : QAbstractTableModel(const_cast<Account*>(a)), m_pAccount(a)
{
   update();
}

QVariant AccountChecksModel::data( const QModelIndex& index, int role ) const
{
   if ((!index.isValid())
    || (index.row() < 0)
    || (index.row() >= enum_class_size<SecurityValidationModel::AccountSecurityFlaw>())
   )
      return QVariant();

   const SecurityValidationModel::AccountSecurityFlaw f = static_cast<SecurityValidationModel::AccountSecurityFlaw>(index.row());

   switch(role) {
      case (int)SecurityValidationModel::Role::Severity:
         return QVariant::fromValue(
            m_lCachedResults[f] == Certificate::CheckValues::UNSUPPORTED ?
               SecurityValidationModel::Severity::UNSUPPORTED : SecurityValidationModelPrivate::flawSeverity[f]
         );
   }

   switch (index.column()) {
      case 0:
         switch(role) {
            case Qt::DisplayRole:
               return SecurityValidationModelPrivate::messages[index.row()];
            case Qt::DecorationRole:
               return PixmapManipulationDelegate::instance()->securityIssueIcon(index);
         };
         break;
      case 1:
         switch(role) {
            case Qt::DisplayRole:
               return tr("Configuration");
         };
         break;
      case 2:
         switch(role) {
            case Qt::DisplayRole:
               if (m_lCachedResults[f] != Certificate::CheckValues::UNSUPPORTED)
                  return m_lCachedResults[f] == Certificate::CheckValues::PASSED ? true : false;
               break;
         };
         break;
   };

   return QVariant();
}

int AccountChecksModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : enum_class_size<SecurityValidationModel::AccountSecurityFlaw>();
}

int AccountChecksModel::columnCount( const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : 3;
}

Qt::ItemFlags AccountChecksModel::flags( const QModelIndex& index) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool AccountChecksModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

QHash<int,QByteArray> AccountChecksModel::roleNames() const
{
   return {};
}

void AccountChecksModel::update()
{
   // AccountSecurityFlaw::SRTP_DISABLED
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::SRTP_ENABLED                 ,
      m_pAccount->isSrtpEnabled                () ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

   // AccountSecurityFlaw::TLS_DISABLED
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::TLS_ENABLED                  ,
      m_pAccount->isTlsEnabled                 () ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

   // AccountSecurityFlaw::CERTIFICATE_MISMATCH
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::CERTIFICATE_MATCH            ,
      Certificate::CheckValues::UNSUPPORTED); //TODO

   // AccountSecurityFlaw::OUTGOING_SERVER_MISMATCH
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::OUTGOING_SERVER_MATCH        ,
      Certificate::CheckValues::UNSUPPORTED); //TODO

   // AccountSecurityFlaw::VERIFY_INCOMING_DISABLED
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::VERIFY_INCOMING_ENABLED      ,
      m_pAccount->isTlsVerifyServer            () ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

   // AccountSecurityFlaw::VERIFY_ANSWER_DISABLED
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::VERIFY_ANSWER_ENABLED        ,
      m_pAccount->isTlsVerifyClient            () ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

   // AccountSecurityFlaw::REQUIRE_CERTIFICATE_DISABLED
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::REQUIRE_CERTIFICATE_ENABLED  ,
      m_pAccount->isTlsRequireClientCertificate() ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

   // AccountSecurityFlaw::MISSING_CERTIFICATE
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::NOT_MISSING_CERTIFICATE      ,
      m_pAccount->tlsCertificate               () ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

   // AccountSecurityFlaw::MISSING_AUTHORITY
   m_lCachedResults.setAt( SecurityValidationModel::AccountSecurityFlaw::NOT_MISSING_AUTHORITY        ,
      m_pAccount->tlsCaListCertificate         () ?
         Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);

}



/*******************************************************************************
 *                                                                             *
 *                          CombinaisonProxyModel                              *
 *                                                                             *
 ******************************************************************************/

CombinaisonProxyModel::CombinaisonProxyModel(QAbstractItemModel* publicCert,
                                             QAbstractItemModel* caCert    ,
                                             QAbstractItemModel* account   ,
                                             QObject*            parent    )
 : QAbstractTableModel(parent), m_lSources({publicCert,caCert,account})
{}

QVariant CombinaisonProxyModel::data( const QModelIndex& index, int role) const
{
   const QAbstractItemModel* src = m_lSources[toModelIdx(index.row())];

   //Role::Severity will give ::UNSUPPORTED (aka, 0) if a model is missing
   //this is done on purpose

   //All "groups" will have empty items for unsupported checks

   return index.isValid() && src ? src->index(fromFinal(index.row()),index.column()).data(role) : QVariant();
}

int CombinaisonProxyModel::rowCount( const QModelIndex& parent) const
{
   return parent.isValid() ? 0 : totalSize();
}

int CombinaisonProxyModel::columnCount( const QModelIndex& parent) const
{
   return parent.isValid() ? 0 : 3;
}

Qt::ItemFlags CombinaisonProxyModel::flags( const QModelIndex& index) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool CombinaisonProxyModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

QHash<int,QByteArray> CombinaisonProxyModel::roleNames() const
{
   return {};
}



/*******************************************************************************
 *                                                                             *
 *                           SecurityValidationModel                           *
 *                                                                             *
 ******************************************************************************/

SecurityValidationModel::SecurityValidationModel(Account* account) : QSortFilterProxyModel(account),
d_ptr(new SecurityValidationModelPrivate(account,this))
{
   Certificate* caCert = d_ptr->m_pAccount->tlsCaListCertificate ();
   Certificate* pkCert = d_ptr->m_pAccount->tlsCertificate       ();

   if (account) {
      d_ptr->m_pCa         = caCert                                       ;
      d_ptr->m_pCert       = pkCert                                       ;
      d_ptr->m_pPrivateKey = d_ptr->m_pAccount->tlsPrivateKeyCertificate();
   }

   PrefixAndSeverityProxyModel* caProxy = caCert ? new PrefixAndSeverityProxyModel(tr("Authority" ),caCert->checksModel()) : nullptr;
   PrefixAndSeverityProxyModel* pkProxy = pkCert ? new PrefixAndSeverityProxyModel(tr("Public key"),pkCert->checksModel()) : nullptr;
   AccountChecksModel* accChecks = new AccountChecksModel(account);

   setSourceModel(new CombinaisonProxyModel(pkProxy,caProxy,accChecks,this));

   setSortRole((int)Role::Severity);
}

SecurityValidationModel::~SecurityValidationModel()
{

}

bool SecurityValidationModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
   const QModelIndex& idx  = sourceModel()->index(source_row,0,source_parent);
   const QModelIndex& idx2 = sourceModel()->index(source_row,2,source_parent);
   const Severity     s    = qvariant_cast<Severity>(idx.data((int)SecurityValidationModel::Role::Severity));
   return s != Severity::UNSUPPORTED && idx2.data(Qt::DisplayRole).toBool() == false;
}

QHash<int,QByteArray> SecurityValidationModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[(int)Role::Severity] = "Severity";
   }
   return roles;
}

void SecurityValidationModelPrivate::update()
{
   //TODO
}

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

#include <securityvalidationmodel.moc>
