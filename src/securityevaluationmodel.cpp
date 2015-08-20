/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#include "securityevaluationmodel.h"

//Qt
#include <QtCore/QIdentityProxyModel>
#include <QtCore/QTimer>

//Ring
#include "account.h"
#include "certificatemodel.h"
#include "interfaces/instances.h"
#include "interfaces/pixmapmanipulatori.h"
#include "private/securityevaluationmodel_p.h"
#include "securityflaw.h"
#include "private/securityflaw_p.h"
#include "private/certificate_p.h"

#include <QtAlgorithms>

const QString SecurityEvaluationModelPrivate::messages[enum_class_size<SecurityEvaluationModel::AccountSecurityChecks>()] = {
   /*SRTP_ENABLED                */QObject::tr("Your media streams are not encrypted, please enable ZRTP or SDES"),
   /*TLS_ENABLED                 */QObject::tr("TLS is disabled, the negotiation wont be encrypted. Your communication will be vulnerable to "
                                   "snooping"),
   /*CERTIFICATE_MATCH           */QObject::tr("Your certificate and authority don't match, if your certificate require an authority, it won't work"),
   /*OUTGOING_SERVER_MATCH       */QObject::tr("The outgoing server specified doesn't match the hostname or the one included in the certificate"),
   /*VERIFY_INCOMING_ENABLED     */QObject::tr("The \"verify incoming certificate\" option is disabled, this leave you vulnerable to man in the middle attack"),
   /*VERIFY_ANSWER_ENABLED       */QObject::tr("The \"verify answer certificate\" option is disabled, this leave you vulnerable to man in the middle attack"),
   /*REQUIRE_CERTIFICATE_ENABLED */QObject::tr("None of your certificate provide a private key, this is required. Please select a private key"
                                       " or use a certificate with one built-in"),
   /*NOT_MISSING_CERTIFICATE     */QObject::tr("No certificate authority is provided, it won't be possible to validate if the answer certificates are valid. Some account may also not work."),
   /*NOT_MISSING_CERTIFICATE     */QObject::tr("No certificate has been provided. This is, for now, unsupported by Ring"),
};

static const QString s1 = QObject::tr("Your certificate is expired, please contact your system administrator.");
static const QString s2 = QObject::tr("Your certificate is self signed. This break the chain of trust.");

const TypedStateMachine< SecurityEvaluationModel::SecurityLevel , SecurityEvaluationModel::AccountSecurityChecks >
SecurityEvaluationModelPrivate::maximumSecurityLevel = {{
   /* SRTP_ENABLED                     */ SecurityEvaluationModel::SecurityLevel::NONE        ,
   /* TLS_ENABLED                      */ SecurityEvaluationModel::SecurityLevel::NONE        ,
   /* CERTIFICATE_MATCH                */ SecurityEvaluationModel::SecurityLevel::WEAK        ,
   /* OUTGOING_SERVER_MATCH            */ SecurityEvaluationModel::SecurityLevel::MEDIUM      ,
   /* VERIFY_INCOMING_ENABLED          */ SecurityEvaluationModel::SecurityLevel::MEDIUM      ,
   /* VERIFY_ANSWER_ENABLED            */ SecurityEvaluationModel::SecurityLevel::MEDIUM      ,
   /* REQUIRE_CERTIFICATE_ENABLED      */ SecurityEvaluationModel::SecurityLevel::WEAK        ,
   /* NOT_MISSING_CERTIFICATE          */ SecurityEvaluationModel::SecurityLevel::WEAK        ,
   /* NOT_MISSING_AUTHORITY            */ SecurityEvaluationModel::SecurityLevel::NONE        , //This wont work
}};

const TypedStateMachine< SecurityEvaluationModel::Severity , SecurityEvaluationModel::AccountSecurityChecks >
SecurityEvaluationModelPrivate::flawSeverity = {{
   /* SRTP_ENABLED                      */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* TLS_ENABLED                       */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* CERTIFICATE_MATCH                 */ SecurityEvaluationModel::Severity::ERROR           ,
   /* OUTGOING_SERVER_MATCH             */ SecurityEvaluationModel::Severity::WARNING         ,
   /* VERIFY_INCOMING_ENABLED           */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* VERIFY_ANSWER_ENABLED             */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* REQUIRE_CERTIFICATE_ENABLED       */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* NOT_MISSING_CERTIFICATE           */ SecurityEvaluationModel::Severity::WARNING         ,
   /* NOT_MISSING_AUTHORITY             */ SecurityEvaluationModel::Severity::ERROR           ,
}};

const TypedStateMachine< SecurityEvaluationModel::SecurityLevel , Certificate::Checks > SecurityEvaluationModelPrivate::maximumCertificateSecurityLevel = {{
   /* HAS_PRIVATE_KEY                   */ SecurityEvaluationModel::SecurityLevel::NONE       ,
   /* EXPIRED                           */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* STRONG_SIGNING                    */ SecurityEvaluationModel::SecurityLevel::WEAK       ,
   /* NOT_SELF_SIGNED                   */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* KEY_MATCH                         */ SecurityEvaluationModel::SecurityLevel::NONE       ,
   /* PRIVATE_KEY_STORAGE_PERMISSION    */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* PRIVATE_KEY_DIRECTORY_PERMISSIONS */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* PUBLIC_KEY_DIRECTORY_PERMISSIONS  */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* PRIVATE_KEY_STORAGE_LOCATION      */ SecurityEvaluationModel::SecurityLevel::ACCEPTABLE ,
   /* PUBLIC_KEY_STORAGE_LOCATION       */ SecurityEvaluationModel::SecurityLevel::ACCEPTABLE ,
   /* PRIVATE_KEY_SELINUX_ATTRIBUTES    */ SecurityEvaluationModel::SecurityLevel::ACCEPTABLE ,
   /* PUBLIC_KEY_SELINUX_ATTRIBUTES     */ SecurityEvaluationModel::SecurityLevel::ACCEPTABLE ,
   /* EXIST                             */ SecurityEvaluationModel::SecurityLevel::NONE       ,
   /* VALID                             */ SecurityEvaluationModel::SecurityLevel::NONE       ,
   /* VALID_AUTHORITY                   */ SecurityEvaluationModel::SecurityLevel::MEDIUM     ,
   /* KNOWN_AUTHORITY                   */ SecurityEvaluationModel::SecurityLevel::ACCEPTABLE , //TODO figure out of the impact of this
   /* NOT_REVOKED                       */ SecurityEvaluationModel::SecurityLevel::WEAK       ,
   /* AUTHORITY_MATCH                   */ SecurityEvaluationModel::SecurityLevel::NONE       ,
   /* EXPECTED_OWNER                    */ SecurityEvaluationModel::SecurityLevel::MEDIUM     , //TODO figure out of the impact of this
   /* ACTIVATED                         */ SecurityEvaluationModel::SecurityLevel::MEDIUM     , //TODO figure out of the impact of this
}};

static const Matrix1D<Certificate::Checks, bool> relevantWithoutPrivateKey = {
   { Certificate::Checks::HAS_PRIVATE_KEY                  , false },
   { Certificate::Checks::EXPIRED                          , true  },
   { Certificate::Checks::STRONG_SIGNING                   , true  },
   { Certificate::Checks::NOT_SELF_SIGNED                  , true  },
   { Certificate::Checks::KEY_MATCH                        , false },
   { Certificate::Checks::PRIVATE_KEY_STORAGE_PERMISSION   , true  },
   { Certificate::Checks::PUBLIC_KEY_STORAGE_PERMISSION    , true  },
   { Certificate::Checks::PRIVATE_KEY_DIRECTORY_PERMISSIONS, true  },
   { Certificate::Checks::PUBLIC_KEY_DIRECTORY_PERMISSIONS , true  },
   { Certificate::Checks::PRIVATE_KEY_STORAGE_LOCATION     , true  },
   { Certificate::Checks::PUBLIC_KEY_STORAGE_LOCATION      , true  },
   { Certificate::Checks::PRIVATE_KEY_SELINUX_ATTRIBUTES   , true  },
   { Certificate::Checks::PUBLIC_KEY_SELINUX_ATTRIBUTES    , true  },
   { Certificate::Checks::EXIST                            , true  },
   { Certificate::Checks::VALID                            , true  },
   { Certificate::Checks::VALID_AUTHORITY                  , true  },
   { Certificate::Checks::KNOWN_AUTHORITY                  , true  },
   { Certificate::Checks::NOT_REVOKED                      , true  },
   { Certificate::Checks::AUTHORITY_MATCH                  , true  },
   { Certificate::Checks::EXPECTED_OWNER                   , true  },
   { Certificate::Checks::ACTIVATED                        , true  },
};

const TypedStateMachine< SecurityEvaluationModel::Severity      , Certificate::Checks > SecurityEvaluationModelPrivate::certificateFlawSeverity = {{
   /* HAS_PRIVATE_KEY                   */ SecurityEvaluationModel::Severity::ERROR           ,
   /* EXPIRED                           */ SecurityEvaluationModel::Severity::WARNING         ,
   /* STRONG_SIGNING                    */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* NOT_SELF_SIGNED                   */ SecurityEvaluationModel::Severity::WARNING         ,
   /* KEY_MATCH                         */ SecurityEvaluationModel::Severity::ERROR           ,
   /* PRIVATE_KEY_STORAGE_PERMISSION    */ SecurityEvaluationModel::Severity::WARNING         ,
   /* PUBLIC_KEY_STORAGE_PERMISSION     */ SecurityEvaluationModel::Severity::WARNING         ,
   /* PRIVATE_KEY_DIRECTORY_PERMISSIONS */ SecurityEvaluationModel::Severity::WARNING         ,
   /* PUBLIC_KEY_DIRECTORY_PERMISSIONS  */ SecurityEvaluationModel::Severity::WARNING         ,
   /* PRIVATE_KEY_STORAGE_LOCATION      */ SecurityEvaluationModel::Severity::INFORMATION     ,
   /* PUBLIC_KEY_STORAGE_LOCATION       */ SecurityEvaluationModel::Severity::INFORMATION     ,
   /* PRIVATE_KEY_SELINUX_ATTRIBUTES    */ SecurityEvaluationModel::Severity::INFORMATION     ,
   /* PUBLIC_KEY_SELINUX_ATTRIBUTES     */ SecurityEvaluationModel::Severity::INFORMATION     ,
   /* EXIST                             */ SecurityEvaluationModel::Severity::ERROR           ,
   /* VALID                             */ SecurityEvaluationModel::Severity::ERROR           ,
   /* VALID_AUTHORITY                   */ SecurityEvaluationModel::Severity::WARNING         ,
   /* KNOWN_AUTHORITY                   */ SecurityEvaluationModel::Severity::WARNING         ,
   /* NOT_REVOKED                       */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* AUTHORITY_MATCH                   */ SecurityEvaluationModel::Severity::ISSUE           ,
   /* EXPECTED_OWNER                    */ SecurityEvaluationModel::Severity::WARNING         ,
   /* ACTIVATED                         */ SecurityEvaluationModel::Severity::WARNING         ,
}};


/**
 * This class add a prefix in front of Qt::DisplayRole to add a disambiguation
 * when there is multiple certificates in the same SecurityEvaluationModel and
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

   enum class Columns {
      MESSAGE ,
      SOURCE  ,
      RESULT  ,
      COUNT__
   };

   AccountChecksModel(const Account* a);

   //Model functions
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                 ) const override;
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Helpers
   void update();

private:
   //Attributes
   const Account* m_pAccount;
   Matrix1D<SecurityEvaluationModel::AccountSecurityChecks, Certificate::CheckValues> m_lCachedResults;
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
   //Attributes
   QVector<QAbstractItemModel*> m_lSources;

   ///All source model, in order
   enum Src {
      CA = 0, /*!< Rows allocated for the certificate authority */
      PK = 1, /*!< Rows allocated for the public certificate    */
      AC = 2, /*!< Rows allocated for the account settions      */
      ER = 3, /*!< TODO Rows allocated for runtime error        */
   };

   ///This model expect a certain size, get each sections size
   constexpr static const short sizes[] = {
      enum_class_size< Certificate             :: Checks                > (),
      enum_class_size< Certificate             :: Checks                > (),
      enum_class_size< SecurityEvaluationModel :: AccountSecurityChecks > (),
   };

   ///Get the combined size
   constexpr inline static int totalSize() {
      return sizes[CA] + sizes[PK] + sizes[AC]+1;
   }

   ///Get a model index from a value
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

constexpr const short CombinaisonProxyModel::sizes[];

///Create a callback map for signals to avoid a large switch(){} in the code
static const Matrix1D<SecurityEvaluationModel::Severity, void(SecurityEvaluationModel::*)()> m_lSignalMap = {{
   /* UNSUPPORTED   */ nullptr                                           ,
   /* INFORMATION   */ &SecurityEvaluationModel::informationCountChanged ,
   /* WARN1NG       */ &SecurityEvaluationModel::warningCountChanged     ,
   /* ISSUE         */ &SecurityEvaluationModel::issueCountChanged       ,
   /* ERROR         */ &SecurityEvaluationModel::errorCountChanged       ,
   /* FATAL_WARNING */ &SecurityEvaluationModel::fatalWarningCountChanged,
}};

SecurityEvaluationModelPrivate::SecurityEvaluationModelPrivate(Account* account, SecurityEvaluationModel* parent) :
 QObject(parent),q_ptr(parent), m_pAccount(account),m_isScheduled(false),
 m_CurrentSecurityLevel(SecurityEvaluationModel::SecurityLevel::NONE),m_pAccChecks(nullptr),
 m_SeverityCount{
      /* UNSUPPORTED   */ 0,
      /* INFORMATION   */ 0,
      /* WARN1NG       */ 0,
      /* ISSUE         */ 0,
      /* ERROR         */ 0,
      /* FATAL_WARNING */ 0,
   }
{
   //Make sure the security level is updated if something change
   QObject::connect(parent,&SecurityEvaluationModel::layoutChanged , this,&SecurityEvaluationModelPrivate::update);
   QObject::connect(parent,&SecurityEvaluationModel::dataChanged   , this,&SecurityEvaluationModelPrivate::update);
   QObject::connect(parent,&SecurityEvaluationModel::rowsInserted  , this,&SecurityEvaluationModelPrivate::update);
   QObject::connect(parent,&SecurityEvaluationModel::rowsRemoved   , this,&SecurityEvaluationModelPrivate::update);
   QObject::connect(parent,&SecurityEvaluationModel::modelReset    , this,&SecurityEvaluationModelPrivate::update);
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

      Certificate::Checks c = Certificate::Checks::HAS_PRIVATE_KEY;
      if (QIdentityProxyModel::data(index,(int)CertificateModel::Role::isCheck).toBool() == true)
         c = qvariant_cast<Certificate::Checks>(QIdentityProxyModel::data(index,(int)CertificateModel::Role::check));
      else if (index.column() != 2) //That column doesn't exist in the source, the wont exist
         return QVariant();

      switch (index.column()) {
         case (int)AccountChecksModel::Columns::MESSAGE:
            switch(role) {
               case Qt::DecorationRole:
                  return Interfaces::pixmapManipulator().securityIssueIcon(index);
               case (int)SecurityEvaluationModel::Role::Severity:
                  return QVariant::fromValue(SecurityEvaluationModelPrivate::certificateFlawSeverity[c]);
               case (int)SecurityEvaluationModel::Role::SecurityLevel:
                  return QVariant::fromValue(SecurityEvaluationModelPrivate::maximumCertificateSecurityLevel[c]);
            }
            break;
         //
         case (int)AccountChecksModel::Columns::SOURCE: {
            switch(role) {
               case Qt::DisplayRole:
                  return m_Name;
               case (int)SecurityEvaluationModel::Role::Severity:
                  return QVariant::fromValue(SecurityEvaluationModelPrivate::certificateFlawSeverity[c]);
               case (int)SecurityEvaluationModel::Role::SecurityLevel:
                  return QVariant::fromValue(SecurityEvaluationModelPrivate::maximumCertificateSecurityLevel[c]);
            }
            return QVariant();
         }
            break;
         //Map source column 1 to 2
         case (int)AccountChecksModel::Columns::RESULT: {
            const QModelIndex& srcIdx = sourceModel()->index(index.row(),1);
            c = qvariant_cast<Certificate::Checks>(srcIdx.data((int)CertificateModel::Role::check));

            switch(role) {
               case (int)SecurityEvaluationModel::Role::Severity:
                  return QVariant::fromValue(SecurityEvaluationModelPrivate::certificateFlawSeverity[c]);
               case (int)SecurityEvaluationModel::Role::SecurityLevel:
                  return QVariant::fromValue(SecurityEvaluationModelPrivate::maximumCertificateSecurityLevel[c]);
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
    || (index.row() >= enum_class_size<SecurityEvaluationModel::AccountSecurityChecks>())
   )
      return QVariant();

   const SecurityEvaluationModel::AccountSecurityChecks f = static_cast<SecurityEvaluationModel::AccountSecurityChecks>(index.row());

   switch(role) {
      case (int)SecurityEvaluationModel::Role::Severity:
         return QVariant::fromValue(
            m_lCachedResults[f] == Certificate::CheckValues::UNSUPPORTED ?
               SecurityEvaluationModel::Severity::UNSUPPORTED : SecurityEvaluationModelPrivate::flawSeverity[f]
         );
      case (int)SecurityEvaluationModel::Role::SecurityLevel:
         return QVariant::fromValue(
            //If the check is unsupported then using "COMPLETE" wont affect the algorithm output
            // if n < current then n else current end will always be "current" when n == maximum
            m_lCachedResults[f] == Certificate::CheckValues::UNSUPPORTED ?
               SecurityEvaluationModel::SecurityLevel::COMPLETE : SecurityEvaluationModelPrivate::maximumSecurityLevel[f]
         );
   }

   switch (index.column()) {
      case (int)AccountChecksModel::Columns::MESSAGE:
         switch(role) {
            case Qt::DisplayRole:
               return SecurityEvaluationModelPrivate::messages[index.row()];
            case Qt::DecorationRole:
               return Interfaces::pixmapManipulator().securityIssueIcon(index);
         };
         break;
      case (int)AccountChecksModel::Columns::SOURCE:
         switch(role) {
            case Qt::DisplayRole:
               return tr("Configuration");
         };
         break;
      case (int)AccountChecksModel::Columns::RESULT:
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
   return parent.isValid() ? 0 : enum_class_size<SecurityEvaluationModel::AccountSecurityChecks>();
}

int AccountChecksModel::columnCount( const QModelIndex& parent ) const
{
   return parent.isValid() ? 0 : enum_class_size<AccountChecksModel::Columns>();
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

//This could have been an inlined function too
#define SET_CHECK_VALUE(check,condition) {Certificate::CheckValues c = Certificate::CheckValues::UNSUPPORTED;\
   bool isSet = m_lCachedResults.isSet(check); if (isSet) c = m_lCachedResults[check]; m_lCachedResults.setAt( check ,\
   condition? Certificate::CheckValues::PASSED : Certificate::CheckValues::FAILED);\
   changed |= (!isSet) || c != m_lCachedResults[check];}

void AccountChecksModel::update()
{
   bool changed = false;

   // AccountSecurityChecks::SRTP_DISABLED
   SET_CHECK_VALUE(SecurityEvaluationModel::AccountSecurityChecks::SRTP_ENABLED                  ,
      m_pAccount->isSrtpEnabled                () || m_pAccount->protocol() == Account::Protocol::RING
   );

   // AccountSecurityChecks::TLS_DISABLED
   SET_CHECK_VALUE( SecurityEvaluationModel::AccountSecurityChecks::TLS_ENABLED                  ,
      m_pAccount->isTlsEnabled                 ()
   );

   // AccountSecurityChecks::CERTIFICATE_MISMATCH
   m_lCachedResults.setAt( SecurityEvaluationModel::AccountSecurityChecks::CERTIFICATE_MATCH     ,
      Certificate::CheckValues::UNSUPPORTED); //TODO

   // AccountSecurityChecks::OUTGOING_SERVER_MISMATCH
   m_lCachedResults.setAt( SecurityEvaluationModel::AccountSecurityChecks::OUTGOING_SERVER_MATCH ,
      Certificate::CheckValues::UNSUPPORTED); //TODO

   // AccountSecurityChecks::VERIFY_INCOMING_DISABLED
   SET_CHECK_VALUE( SecurityEvaluationModel::AccountSecurityChecks::VERIFY_INCOMING_ENABLED      ,
      m_pAccount->isTlsVerifyServer            ()
   );

   // AccountSecurityChecks::VERIFY_ANSWER_DISABLED
   SET_CHECK_VALUE( SecurityEvaluationModel::AccountSecurityChecks::VERIFY_ANSWER_ENABLED        ,
      m_pAccount->isTlsVerifyClient            ()
   );

   // AccountSecurityChecks::REQUIRE_CERTIFICATE_DISABLED
   SET_CHECK_VALUE( SecurityEvaluationModel::AccountSecurityChecks::REQUIRE_CERTIFICATE_ENABLED  ,
      m_pAccount->isTlsRequireClientCertificate()
   );

   // AccountSecurityChecks::MISSING_CERTIFICATE
   SET_CHECK_VALUE( SecurityEvaluationModel::AccountSecurityChecks::NOT_MISSING_CERTIFICATE      ,
      m_pAccount->tlsCertificate               ()
   );

   // AccountSecurityChecks::MISSING_AUTHORITY
   SET_CHECK_VALUE( SecurityEvaluationModel::AccountSecurityChecks::NOT_MISSING_AUTHORITY        ,
      m_pAccount->tlsCaListCertificate         ()
   );

   if (changed)
      emit dataChanged(index(0,2),index(rowCount()-1,2));
}
#undef SET_CHECK_VALUE


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
{
   for (int i = 0; i < m_lSources.size(); i++) {
      const QAbstractItemModel* m = m_lSources[i];
      if (m) {
         connect(m, &QAbstractItemModel::dataChanged, [this,i](const QModelIndex& tl, const QModelIndex& br) {

            int offset =0;
            for (int j = 0; j < i;j++)
               offset += sizes[j];


            emit this->dataChanged(this->index(offset+tl.row(), br.column()), this->index(offset+br.row(), br.column()));
         });
      }
   }
}

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
 *                           SecurityEvaluationModel                           *
 *                                                                             *
 ******************************************************************************/

SecurityEvaluationModel::SecurityEvaluationModel(Account* account) : QSortFilterProxyModel(account),
d_ptr(new SecurityEvaluationModelPrivate(account,this))
{
   Certificate* caCert = d_ptr->m_pAccount->tlsCaListCertificate ();
   Certificate* pkCert = d_ptr->m_pAccount->tlsCertificate       ();

   SecurityEvaluationModelPrivate::getCertificateSeverityProxy(caCert);
   SecurityEvaluationModelPrivate::getCertificateSeverityProxy(pkCert);

   d_ptr->m_pAccChecks = new AccountChecksModel(account);

   d_ptr->update();

   setSourceModel(new CombinaisonProxyModel(pkCert ? pkCert->d_ptr->m_pSeverityProxy : nullptr, caCert ? caCert->d_ptr->m_pSeverityProxy : nullptr, d_ptr->m_pAccChecks,this));

   setSortRole((int)Role::Severity);
}

SecurityEvaluationModel::~SecurityEvaluationModel()
{
   delete d_ptr;
}

bool SecurityEvaluationModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
   const QModelIndex& idx  = sourceModel()->index(source_row,0,source_parent);
   const QModelIndex& idx2 = sourceModel()->index(source_row,2,source_parent);
   const Severity     s    = qvariant_cast<Severity>(idx.data((int)SecurityEvaluationModel::Role::Severity));
   return s != Severity::UNSUPPORTED && idx2.data(Qt::DisplayRole).toBool() == false;
}

QHash<int,QByteArray> SecurityEvaluationModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[(int)Role::Severity] = "Severity";
   }
   return roles;
}

void SecurityEvaluationModelPrivate::update()
{
   //As this can be called multiple time, only perform the checks once per event loop cycle
   if (!m_isScheduled) {
      m_pAccChecks->update();

#if QT_VERSION >= 0x050400
      QTimer::singleShot(0,this,&SecurityEvaluationModelPrivate::updateReal);
      m_isScheduled = true;
#else //Too bad for Qt < 5.3 users
      updateReal();
#endif

   }
}

QAbstractItemModel* SecurityEvaluationModelPrivate::getCertificateSeverityProxy(Certificate* c)
{
   if (!c)
      return nullptr;

   if (!c->d_ptr->m_pSeverityProxy)
      c->d_ptr->m_pSeverityProxy = new PrefixAndSeverityProxyModel(tr("Authority" ),c->checksModel());

   return c->d_ptr->m_pSeverityProxy;
}

SecurityEvaluationModel::SecurityLevel SecurityEvaluationModelPrivate::maxSecurityLevel(QAbstractItemModel* m, int* counter)
{
   typedef SecurityEvaluationModel::Severity      Severity     ;
   typedef SecurityEvaluationModel::SecurityLevel SecurityLevel;

   SecurityLevel maxLevel = SecurityLevel::COMPLETE;

   for (int i=0; i < m->rowCount();i++) {
      const QModelIndex&  idx      = m->index(i,0);

      const Severity      severity = qvariant_cast<Severity>(
         idx.data((int) SecurityEvaluationModel::Role::Severity)
      );

      //Ignore items without severity
      const QVariant levelVariant = idx.data((int) SecurityEvaluationModel::Role::SecurityLevel );

      const SecurityLevel level    = levelVariant.canConvert<SecurityLevel>() ? qvariant_cast<SecurityLevel>(
         levelVariant
      ) : maxLevel;

      //Increment the count
      if (counter)
         counter[static_cast<int>(severity)]++;

      const bool forceIgnore = idx.data((int)CertificateModel::Role::requirePrivateKey).toBool();

      //Update the maximum level
      maxLevel = level < maxLevel && !forceIgnore ? level : maxLevel;
   }

   return maxLevel;
}

void SecurityEvaluationModelPrivate::updateReal()
{
   typedef SecurityEvaluationModel::Severity      Severity     ;
   typedef SecurityEvaluationModel::SecurityLevel SecurityLevel;

   int countCache[enum_class_size<SecurityEvaluationModel::Severity>()];

   //Reset the counter
   for (const Severity s : EnumIterator<Severity>()) {
      countCache     [(int)s] = m_SeverityCount[(int)s];
      m_SeverityCount[(int)s] = 0                      ;
   }

   SecurityLevel maxLevel = maxSecurityLevel(q_ptr, m_SeverityCount);

   //Notify
   for (const Severity s : EnumIterator<Severity>()) {
      if (countCache[(int)s] != m_SeverityCount[(int)s] && m_lSignalMap[s])
         (q_ptr->*m_lSignalMap[s])();
   }

   //Update the security level
   if (m_CurrentSecurityLevel != maxLevel) {
      m_CurrentSecurityLevel = maxLevel;

      emit q_ptr->securityLevelChanged();
   }

   m_isScheduled = false;
}

QModelIndex SecurityEvaluationModel::getIndex(const SecurityFlaw* flaw)
{
   return index(flaw->d_ptr->m_Row,0);
}

QList<SecurityFlaw*> SecurityEvaluationModel::currentFlaws()
{
   return d_ptr->m_lCurrentFlaws;
}

SecurityEvaluationModel::SecurityLevel SecurityEvaluationModel::securityLevel() const
{
   return d_ptr->m_CurrentSecurityLevel;
}

SecurityEvaluationModel::SecurityLevel SecurityEvaluationModelPrivate::certificateSecurityLevel(const Certificate* c, bool forceIgnorePrivateKey)
{
   typedef SecurityEvaluationModel::SecurityLevel SecurityLevel;

   SecurityLevel maxLevelWithPriv    = SecurityLevel::COMPLETE;
   SecurityLevel maxLevelWithoutPriv = SecurityLevel::COMPLETE;

   const bool ignorePrivateKey = forceIgnorePrivateKey || (c->requirePrivateKey() == false);

   if (c->d_ptr->m_hasLoadedSecurityLevel) {
      if (ignorePrivateKey)
         return c->d_ptr->m_SecurityLevelWithoutPriv;
      else
         return c->d_ptr->m_SecurityLevelWithPriv;
   }

   for (const Certificate::Checks check : EnumIterator<Certificate::Checks>()) {
      const bool relevant = relevantWithoutPrivateKey[check];
      if (c->checkResult(check) == Certificate::CheckValues::FAILED) {
         if (relevant) {
            const SecurityLevel checkLevel = maximumCertificateSecurityLevel[check];
            maxLevelWithoutPriv = checkLevel < maxLevelWithoutPriv ? checkLevel : maxLevelWithoutPriv;
         }
         const SecurityLevel checkLevel = maximumCertificateSecurityLevel[check];
         maxLevelWithPriv = checkLevel < maxLevelWithPriv ? checkLevel : maxLevelWithPriv;
      }
   }

   c->d_ptr->m_hasLoadedSecurityLevel   = true;
   c->d_ptr->m_SecurityLevelWithoutPriv = maxLevelWithoutPriv;
   c->d_ptr->m_SecurityLevelWithPriv    = maxLevelWithPriv;

   return ignorePrivateKey ? c->d_ptr->m_SecurityLevelWithoutPriv : c->d_ptr->m_SecurityLevelWithPriv;
}

//Map the array to getters
int SecurityEvaluationModel::informationCount             () const
{ return d_ptr->m_SeverityCount[ (int)Severity::INFORMATION   ]; }
int SecurityEvaluationModel::warningCount                 () const
{ return d_ptr->m_SeverityCount[ (int)Severity::WARNING       ]; }
int SecurityEvaluationModel::issueCount                   () const
{ return d_ptr->m_SeverityCount[ (int)Severity::ISSUE         ]; }
int SecurityEvaluationModel::errorCount                   () const
{ return d_ptr->m_SeverityCount[ (int)Severity::ERROR         ]; }
int SecurityEvaluationModel::fatalWarningCount            () const
{ return d_ptr->m_SeverityCount[ (int)Severity::FATAL_WARNING ]; }

#include <securityevaluationmodel.moc>
