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

SecurityValidationModelPrivate::SecurityValidationModelPrivate(Account* account, SecurityValidationModel* parent) : q_ptr(parent),
m_pAccount(account), m_CurrentSecurityLevel(SecurityValidationModel::SecurityLevel::NONE)
{
}

PrefixAndSeverityProxyModel::PrefixAndSeverityProxyModel(const QString& prefix, QAbstractItemModel* parent) : QIdentityProxyModel(parent),m_Name(prefix)
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
      switch (index.column()) {
         case 0:
            switch(role) {
               case Qt::DecorationRole:
                  return PixmapManipulationDelegate::instance()->serurityIssueIcon(index);
               case (int)SecurityValidationModel::Role::Severity:
                  return QVariant::fromValue(SecurityValidationModel::Severity::WARNING);
            }
            break;
         //
         case 1: {
            switch(role) {
               case Qt::DisplayRole:
                  return m_Name;
               case (int)SecurityValidationModel::Role::Severity:
                  return QVariant::fromValue(SecurityValidationModel::Severity::WARNING);
            }
            return QVariant();
         }
            break;
         //Map source column 1 to 2
         case 2: {
            switch(role) {
               case (int)SecurityValidationModel::Role::Severity:
                  return QVariant::fromValue(SecurityValidationModel::Severity::WARNING);
            }

            const QModelIndex& srcIdx = sourceModel()->index(index.row(),1);
            return srcIdx.data(role);
         }
      }
   }

   return QIdentityProxyModel::data(index,role);
}

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

   setSourceModel(new CombinaisonProxyModel(pkProxy,caProxy,nullptr,this));

   setSortRole((int)Role::Severity);
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
