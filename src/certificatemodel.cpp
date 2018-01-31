/****************************************************************************
 *   Copyright (C) 2015-2018 Savoir-faire Linux                               *
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
#include "certificatemodel.h"

//Qt
#include <QtCore/QCoreApplication>
#include <QtCore/QObject>
#include <QtCore/QCryptographicHash>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>
#include <QtCore/QAbstractProxyModel>

//LibSTDC++
#include <functional>

//Dring
#include "dbus/configurationmanager.h"
#include "security_const.h"

//Ring
#include "certificate.h"
#include "contactmethod.h"
#include "account.h"
#include "foldercertificatecollection.h"
#include "daemoncertificatecollection.h"
#include "private/matrixutils.h"
#include "private/certificatemodel_p.h"
#include "accountmodel.h"

/*
 * This data structure is a graph wrapping the certificates in different contexts.
 *
 * For example, a single certificate can be allowed for an account and banned for
 * another. Then, there is also the notion of "known" certificates for an account
 * that's independent from the certificate state.
 *
 * This class store bunch of "groups" associated with accounts. Each certificate
 * can be part of multiple groups.
 *
 * Levels:
 * 1: Root (tree)
 * 2: Groups (tree)
 * 3: Certificate (graph)
 * 4: Certificate elements
 * 5.1: Certificate details
 * 5.2: Certificate security checks
 *
 * Structures:
 * CertificateNode: metadata associated with the certificate
 * CertificateNodeWrapper: A single path in the graph
 *
 * It is a graph, but it's usually handled as a tree. The only layer of the tree
 * with graph characteristic is the certificate level.
 *
 * The purpose of this is to be able to create model proxies for various type
 * of information based on the same memory tree.
 *
 */

enum class DetailType : uchar
{
   NONE  ,
   DETAIL,
   CHECK ,
};


struct CertificateNode {

   CertificateNode(int index, CertificateModel::NodeType level, CertificateNode* parent, Certificate* cert);
   ~CertificateNode();
   void setStrings(const QString& col1, const QVariant& col2, const QString& tooltip);

   //Attributes
   QVector<CertificateNode*>  m_lChildren      ;
   CertificateNode*           m_pParent        ;
   Certificate*               m_pCertificate   ;
   CertificateModel::NodeType m_Level          ;
   DetailType                 m_DetailType     ;
   int                        m_Index          ;
   int                        m_EnumClassDetail;
   QString                    m_Col1           ;
   QVariant                   m_Col2           ;
   QString                    m_ToolTip        ;
   std::function<void()>      m_fLoader        ;
   bool                       m_IsLoaded       ;
   int                        m_CatIdx         ;
   unsigned long long         m_fIsPartOf      ;
   QHash<Account*,CertificateNode*> m_hSiblings;
};

class CertificateProxyModel final : public QAbstractProxyModel
{
   Q_OBJECT
public:
   CertificateProxyModel(CertificateModel* parent, CertificateNode* root);

   //Model implementation
   virtual QModelIndex mapFromSource( const QModelIndex& sourceIndex                              ) const override;
   virtual QModelIndex mapToSource  ( const QModelIndex& proxyIndex                               ) const override;
   virtual QModelIndex index        ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QModelIndex parent       ( const QModelIndex& index                                    ) const override;
   virtual int         rowCount     ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual int         columnCount  ( const QModelIndex& parent = QModelIndex()                   ) const override;

private:
   CertificateNode* m_pRoot;
};

//TODO remove
static FolderCertificateCollection* m_pFallbackCollection = nullptr;
static DaemonCertificateCollection* m_pFallbackDaemonCollection = nullptr;

const Matrix1D<Certificate::Status, const char*> CertificateModelPrivate::m_StatusMap = {{
/* Certificate::Status::UNDEFINED      */ DRing::Certificate::Status::UNDEFINED,
/* Certificate::Status::ALLOWED        */ DRing::Certificate::Status::ALLOWED  ,
/* Certificate::Status::BANNED         */ DRing::Certificate::Status::BANNED   ,
/* Certificate::Status::REVOKED        */ ""                                   ,
/* Certificate::Status::REVOKED_ALLOWED*/ DRing::Certificate::Status::ALLOWED  ,
}};

CertificateModelPrivate::~CertificateModelPrivate()
{
   foreach(CertificateNode* node, m_lTopLevelNodes) {
      delete node->m_pCertificate;
      delete node;
   }
}

CertificateNode::CertificateNode(int index, CertificateModel::NodeType level, CertificateNode* parent, Certificate* cert) :
   m_pParent(parent), m_pCertificate(cert), m_Level(level), m_Index(index), m_IsLoaded(true),m_DetailType(DetailType::NONE),
   m_EnumClassDetail(0),m_CatIdx(-1),m_fIsPartOf(0)
{
   if (level == CertificateModel::NodeType::CATEGORY )
      m_CatIdx = ++(CertificateModel::instance().d_ptr->m_GroupCounter);

   CertificateNode* sibling = CertificateModel::instance().d_ptr->m_hNodes[cert];

   if (parent && sibling && parent->m_Level == CertificateModel::NodeType::CATEGORY)
      sibling->m_fIsPartOf |= (0x01 << parent->m_CatIdx);
   else
      CertificateModel::instance().d_ptr->m_hNodes[cert] = this;

   if (parent && parent->m_Level == CertificateModel::NodeType::CATEGORY)
      m_fIsPartOf |= (0x01 << parent->m_CatIdx);
}

CertificateNode::~CertificateNode()
{
   for (CertificateNode* c : m_lChildren)
      delete c;
}

CertificateModelPrivate::CertificateModelPrivate(CertificateModel* parent) : QObject(parent), q_ptr(parent),
 m_pDefaultCategory(nullptr), m_CertLoader(), m_GroupCounter(-1)
{
    connect(&ConfigurationManager::instance(), &ConfigurationManagerInterface::certificateStateChanged, this, &CertificateModelPrivate::slotCertificateStateChanged);
}

CertificateModel::CertificateModel(QObject* parent) : QAbstractItemModel(parent), CollectionManagerInterface<Certificate>(this),
 d_ptr(new CertificateModelPrivate(this))
{
   setObjectName("CertificateModel");
   //TODO replace with something else
   m_pFallbackCollection = addCollection<FolderCertificateCollection,QString,FlagPack<FolderCertificateCollection::Options>, QString>(QString(),
      FolderCertificateCollection::Options::FALLBACK | FolderCertificateCollection::Options::READ_WRITE,
      QObject::tr("Local certificate store")
   );
   m_pFallbackDaemonCollection = addCollection<DaemonCertificateCollection,Account*,DaemonCertificateCollection::Mode>(
      nullptr,
      DaemonCertificateCollection::Mode::ALLOWED
   );

   m_pFallbackCollection->load();
}

CertificateModel::~CertificateModel()
{
}

CertificateModel& CertificateModel::instance()
{
    static auto instance = new CertificateModel(QCoreApplication::instance());
    return *instance;
}

QHash<int,QByteArray> CertificateModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[static_cast<int>(Role::NodeType)] = "nodeType";

      //Add all the details, this isn't safe because of tr(), but good enough for debugging
      for (const Certificate::Details d :  EnumIterator<Certificate::Details>()) {
         QString name = Certificate::getName(d).toLatin1();
         while (name.indexOf(' ') != -1) {
            const int idx = name.indexOf(' ')  ;
            name          = name.remove(idx,1) ;
            name[idx]     = name[idx].toUpper();
            name[0  ]     = name[0].toLower  ();
         }
         roles[static_cast<int>(Role::DetailRoleBase)+static_cast<int>(d)] = name.toLatin1();
      }

      roles[static_cast<int>(Role::isDetail)] = "isDetail";
      roles[static_cast<int>(Role::isCheck )] = "isCheck" ;
      roles[static_cast<int>(Role::detail  )] = "detail"  ;
      roles[static_cast<int>(Role::check   )] = "check"   ;
   }
   return roles;
}

void CertificateNode::setStrings(const QString& col1, const QVariant& col2, const QString& tooltip)
{
   m_Col1    = col1   ;
   m_Col2    = col2   ;
   m_ToolTip = tooltip;
}

CertificateNode* CertificateModelPrivate::createCategory(const QString& name, const QString& col2, const QString& tooltip )
{
   QMutexLocker l(&m_CertLoader);
   const int idx = m_lTopLevelNodes.size();

   // This should be avoided whenever possible. Having a duplicate would be
   // both a memory leak and a potential collision attack (far fetched).
   // This code avoids leak, but has side effects.
   if (m_hStrToCat.contains(name)) {
      qWarning() << "Trying to create a certificate node with an already used id."
         " This can have unforneen consequences";
      return m_hStrToCat[name];
   }

   CertificateNode* n = new CertificateNode(idx, CertificateModel::NodeType::CATEGORY, nullptr, nullptr);
   n->setStrings(name,col2,tooltip);

   q_ptr->beginInsertRows(QModelIndex(), idx, idx);
   m_lTopLevelNodes << n;
   q_ptr->endInsertRows();

   m_hStrToCat[name] = n;

   return n;
}

CertificateNode* CertificateModelPrivate::defaultCategory()
{
   if (!m_pDefaultCategory) {
      m_pDefaultCategory = createCategory(QObject::tr("Default"),QObject::tr("Certificate not associated with a group"),QString());
   }

   return m_pDefaultCategory;
}

QModelIndex CertificateModelPrivate::createIndex(int r ,int c , void* p)
{
   return q_ptr->createIndex(r,c,p);
}

void CertificateModelPrivate::removeFromTree(CertificateNode* node)
{
   CertificateNode* parent = node->m_pParent;
   const QModelIndex parentIdx = q_ptr->createIndex(parent->m_Index,0,parent);

   q_ptr->beginRemoveRows(parentIdx, node->m_Index, node->m_Index);
   parent->m_lChildren.removeAt(node->m_Index);

   for(int i = node->m_Index; i < parent->m_lChildren.size(); i++)
      parent->m_lChildren[i]->m_Index--;

   q_ptr->endRemoveRows();

   //FIXME Those nodes are indexed in a few hashes, clear them
   delete node;
}

void CertificateModelPrivate::removeFromTree(Certificate* c, CertificateNode* category)
{
   if ((!c) || (!category))
      return;

   const QVector<CertificateNode*> nodes = category->m_lChildren;

   for (CertificateNode* n : nodes) {
      if (n->m_pCertificate == c)
         removeFromTree(n);
   }
}

CertificateNode* CertificateModelPrivate::getCategory(const Account* a)
{
   CertificateNode* cat = m_hAccToCat.value(a);

   if (!cat) {
      cat = createCategory(a->alias(),QString(),QString());
      m_hAccToCat[a] = cat;
   }

   return cat;
}

//For convenience
CertificateNode* CertificateModelPrivate::addToTree(Certificate* cert, Account* a)
{

   if (!a)
      return addToTree(cert);

   CertificateNode* cat = getCategory(a);

   return addToTree(cert,cat);
}

void CertificateModelPrivate::regenChecks(Certificate* cert)
{
   CertificateNode* n = m_hNodes.value(cert);

   if (!n)
      return;

   //There could be a loading race condition
   if (n->m_lChildren.size() >= 2)
      loadChecks(n->m_lChildren[1], cert);
}

//[Re]generate the checks
void CertificateModelPrivate::loadChecks(CertificateNode* checks, Certificate* cert)
{
   QMutexLocker locker(&m_CertLoader);
   const QModelIndex checksI(q_ptr->createIndex(checks->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),checks));

   //Clear the existing nodes
   if (checks->m_lChildren.size()) {
      q_ptr->beginRemoveRows(checksI, 0, checks->m_lChildren.size());
      const QList<CertificateNode*> nodes;

      for (CertificateNode* n : nodes)
         delete n;

      checks->m_lChildren.clear();

      q_ptr->endRemoveRows();
   }

   //Add the new ones
   for (const Certificate::Checks check : EnumIterator<Certificate::Checks>()) {
      if (cert->checkResult(check) != Certificate::CheckValues::UNSUPPORTED) {
         q_ptr->beginInsertRows(checksI, checks->m_lChildren.size(), checks->m_lChildren.size());
         CertificateNode* d = new CertificateNode(checks->m_lChildren.size(), CertificateModel::NodeType::DETAILS, checks, nullptr);
         d->setStrings(cert->getName(check),static_cast<bool>(cert->checkResult(check)),cert->getDescription(check));
         d->m_DetailType = DetailType::CHECK;
         d->m_pCertificate = cert;
         d->m_EnumClassDetail = static_cast<int>(check);
         checks->m_lChildren << d;
         q_ptr->endInsertRows();
      } else {
         // unsupported check, not inserting
      }
   }
}

CertificateNode* CertificateModelPrivate::addToTree(Certificate* cert, CertificateNode* category)
{
   if (!category)
      category = defaultCategory();

   // defaultCategory(); can request the mutex
   QMutexLocker locker(&m_CertLoader);

   //Do not add it twice
   CertificateNode* node = CertificateModel::instance().d_ptr->m_hNodes.value(cert);

   if (isPartOf(node, category))
      return node;

   const int idx = category->m_lChildren.size();

   node = new CertificateNode(idx, CertificateModel::NodeType::CERTIFICATE, category, cert);
   node->setStrings(QObject::tr("A certificate"),QObject::tr("An organisation"),QString());

   const QModelIndex parent = q_ptr->createIndex(category->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),category);
   q_ptr->beginInsertRows(parent, idx, idx);
   category->m_lChildren << node;
   q_ptr->endInsertRows();

   //Lazy loaded function to reduce the overhead of this (mostly hidden) model
   node->m_fLoader = [this,node,cert]() {
      CertificateNode* checks = nullptr;
      { // mutex
      node->m_Col1 = cert->detailResult(Certificate::Details::PUBLIC_KEY_ID).toString();
      node->m_IsLoaded = true;
      const QModelIndex index = q_ptr->createIndex(node->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),node);

      CertificateNode* details = nullptr;

      //Insert the check and details categories
      { // mutex
      QMutexLocker locker(&m_CertLoader);
      q_ptr->beginInsertRows(index, 0, 1);
      details = new CertificateNode(static_cast<int>(CertificateModel::Columns::NAME ), CertificateModel::NodeType::DETAILS_CATEGORY, node, nullptr);
      checks = new CertificateNode(static_cast<int>(CertificateModel::Columns::VALUE), CertificateModel::NodeType::DETAILS_CATEGORY, node, nullptr);
      details->setStrings(QObject::tr("Details"),QString(),QObject::tr("The content of the certificate")       );
      checks ->setStrings(QObject::tr("Checks") ,QString(),QObject::tr("Various security related information") );
      node->m_lChildren << details;
      node->m_lChildren << checks;
      q_ptr->endInsertRows();
      } // mutex

      static const int detailsC(enum_class_size<Certificate::Details>());

      const QModelIndex detailsI(q_ptr->createIndex(details->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),details));

      // Make sure the lazy-loaded details have been created (otherwise it will deadlock)
      for (const Certificate::Details detail : EnumIterator<Certificate::Details>()) {
         cert->getName       (detail);
         cert->detailResult  (detail);
         cert->getDescription(detail);
      }

      //Insert the details
      { // mutex
      QMutexLocker locker(&m_CertLoader);
      q_ptr->beginInsertRows(detailsI, 0, detailsC - 1);
      for (const Certificate::Details detail : EnumIterator<Certificate::Details>()) {
         CertificateNode* d = new CertificateNode(details->m_lChildren.size(), CertificateModel::NodeType::DETAILS, details, nullptr);
         d->setStrings(cert->getName(detail),cert->detailResult(detail),cert->getDescription(detail));
         d->m_DetailType = DetailType::DETAIL;
         d->m_pCertificate = cert;
         d->m_EnumClassDetail = static_cast<int>(detail);
         details->m_lChildren << d;
      }
      q_ptr->endInsertRows();
      } // mutex

      } // mutex

      //Insert the checks
      this->loadChecks(checks, cert);
   };
   node->m_IsLoaded = false;

   return node;
}

bool CertificateModelPrivate::isPartOf(CertificateNode* sibling, CertificateNode* list)
{
    if (!list) {
        qWarning() << "CertificateModelPrivate::isPartOf called on empty list";
        return false;
    }

    return sibling && sibling->m_fIsPartOf & (1 << list->m_CatIdx);
}

bool CertificateModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)

   return false;
}

QVariant CertificateModel::data( const QModelIndex& index, int role) const
{
   Q_UNUSED(role)
   if (!index.isValid())
      return QVariant();
   const CertificateNode* node = static_cast<CertificateNode*>(index.internalPointer());

   if (!node)
      return QVariant();

   if (node->m_Level == NodeType::CERTIFICATE) {

      // If the certificate has a contactMethod, implement the generic LRC roles
      // this is used to match trust requests or white/blacklist entries with
      // peoples.
      if (node->m_pCertificate && node->m_pCertificate->contactMethod() && (
          role == Qt::DisplayRole || role == Qt::DecorationRole || (
             role > static_cast<int>(Ring::Role::Object)
              && role < static_cast<int>(Ring::Role::UnreadTextMessageCount)
          )
      )) {
         switch(role) {
            case static_cast<int>(Ring::Role::Object):
               return QVariant::fromValue(node->m_pCertificate);
            case static_cast<int>(Ring::Role::ObjectType):
               return QVariant::fromValue(Ring::ObjectType::Certificate);
            default:
               return node->m_pCertificate->contactMethod()->roleData(role);
         }
      }
      // Add the details as roles for certificates
      else if (role >= static_cast<int>(Role::DetailRoleBase) && role < static_cast<int>(Role::DetailRoleBase)+enum_class_size<Certificate::Details>()) {
         Certificate* cert = node->m_pCertificate;
         if (cert) {
            return cert->detailResult(static_cast<Certificate::Details>(role - static_cast<int>(Role::DetailRoleBase)));
         }
      }
   }

   switch(role) {
      case Qt::DisplayRole:
      case Qt::EditRole:
         return index.column()?node->m_Col2:node->m_Col1;
      case Qt::ToolTipRole:
         return node->m_ToolTip;
      case static_cast<int>(Role::NodeType):
         return QVariant::fromValue(node->m_Level);
   };

   switch (node->m_Level) {
      case CertificateModel::NodeType::DETAILS         :
         switch(role) {
            case (int)Role::isDetail:
               return node->m_DetailType == DetailType::DETAIL;
            case (int)Role::isCheck:
               return node->m_DetailType == DetailType::CHECK;
            case (int)Role::detail:
               if (node->m_DetailType == DetailType::DETAIL)
                  return QVariant::fromValue(static_cast<Certificate::Details>(node->m_EnumClassDetail));
               break;
            case (int)Role::check:
               if (node->m_DetailType == DetailType::CHECK)
                  return QVariant::fromValue(static_cast<Certificate::Checks>(node->m_EnumClassDetail));
               break;
            case (int)Role::requirePrivateKey:
               return node->m_pCertificate ? node->m_pCertificate->requirePrivateKey() : false;
         }
         break;
      case CertificateModel::NodeType::CERTIFICATE     :
      case CertificateModel::NodeType::DETAILS_CATEGORY:
      case CertificateModel::NodeType::CATEGORY        :
         break;
   }

   return QVariant();
}

int CertificateModel::rowCount( const QModelIndex& parent) const
{
   if (!parent.isValid())
      return d_ptr->m_lTopLevelNodes.size();
   else {
      const CertificateNode* node = static_cast<CertificateNode*>(parent.internalPointer());

      //Load that info only when it is needed
      if (node->m_Level == CertificateModel::NodeType::CERTIFICATE && (!node->m_IsLoaded))
         node->m_fLoader();
      return node->m_lChildren.size();
   }
}

Qt::ItemFlags CertificateModel::flags( const QModelIndex& index) const
{
   return Qt::ItemIsEnabled | (index.column()==static_cast<int>(Columns::VALUE)?Qt::ItemIsEditable:Qt::NoItemFlags);
}

int CertificateModel::columnCount( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 2;
}

QModelIndex CertificateModel::parent( const QModelIndex& index) const
{
   if (!index.isValid())
      return QModelIndex();

   const CertificateNode* node = static_cast<CertificateNode*>(index.internalPointer());

   return node->m_pParent ? createIndex(node->m_pParent->m_Index, index.column(), node->m_pParent) : QModelIndex();
}

QModelIndex CertificateModel::index( int row, int column, const QModelIndex& parent) const
{
   if((!parent.isValid()) && row >= 0 && row < d_ptr->m_lTopLevelNodes.size() && column < 2)
      return createIndex( row, column, d_ptr->m_lTopLevelNodes[row]);
   else if (parent.isValid() && row >= 0) {
      const CertificateNode* node = static_cast<CertificateNode*>(parent.internalPointer());
      if (node->m_lChildren.size() > row)
         return createIndex( row , column, node->m_lChildren[row]);
   }
   return QModelIndex();
}

QVariant CertificateModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QObject::tr("Header");

   return QVariant();
}

/**
 *Create a certificate. It will later be registerer in the daemon
 *
 * @note The path has to be a file, but can contain multiple certificates
 */
Certificate* CertificateModel::getCertificateFromPath(const QString& path, Account* a)
{
   if (!a)
      return getCertificateFromPath(path,Certificate::Type::CALL);

   CertificateNode* cat  = d_ptr->getCategory(a);

   Certificate* cert = d_ptr->m_hCertificates.value(path);

   if (!cert) {
      cert = new Certificate(path, Certificate::Type::NONE);
      cert->setCollection(m_pFallbackDaemonCollection);

      { // mutex
      QMutexLocker(&d_ptr->m_CertInsertion);
      d_ptr->m_hCertificates[path.toLatin1()] = cert;
      } // mutex

      //Add it to the model
      d_ptr->addToTree(cert,a);
   }

   CertificateNode* node = d_ptr->m_hNodes.value(cert);

   if (node) {
      if (node->m_pParent != cat) {
         CertificateNode* node2 = d_ptr->addToTree(cert,cat);
         node->m_hSiblings[a] = node2;
      }
   }
   return cert;
}

Certificate* CertificateModel::getCertificateFromPath(const QString& path, Certificate::Type type)
{
   const QString id = path;

   Certificate* cert = d_ptr->m_hCertificates.value(id);

   //The certificate is not loaded yet
   if (!cert) {
      cert = new Certificate(path, type);
      cert->setCollection(m_pFallbackDaemonCollection);

      { // mutex
      QMutexLocker(&d_ptr->m_CertInsertion);
      d_ptr->m_hCertificates[path.toLatin1()] = cert;
      } // mutex

      //Add it to the model
      d_ptr->addToTree(cert);
   }

   return cert;
}

Certificate* CertificateModel::getCertificateFromId(const QString& id, Account* a, const QString& category)
{
   Certificate* cert = d_ptr->m_hCertificates.value(id);

   //The certificate is not loaded yet
   if (!cert) {
      cert = new Certificate(id);

      { // mutex
      QMutexLocker(&d_ptr->m_CertInsertion);
      d_ptr->m_hCertificates[id.toLatin1()] = cert;
      } // mutex

      if ((!a) && (!category.isEmpty())) {
         CertificateNode* cat = d_ptr->m_hStrToCat.value(category);

         if (!cat) {
            cat = d_ptr->createCategory(category, a?QString("%1 certificates").arg(a->alias()):QString(), QString());
         }

         d_ptr->addToTree(cert,cat);
      }
      else {
         //Add it to the model
         d_ptr->addToTree(cert, a);
      }
   }

   return cert;
}

CertificateProxyModel::CertificateProxyModel(CertificateModel* parent, CertificateNode* root) : QAbstractProxyModel(parent),m_pRoot(root)
{
   //For debugging
   switch (root->m_Level) {
      case CertificateModel::NodeType::CERTIFICATE     :
         setObjectName(root->m_pCertificate->path());
         break;
      case CertificateModel::NodeType::DETAILS_CATEGORY:
         setObjectName(root->m_pParent->m_pCertificate->path());
         break;
      case CertificateModel::NodeType::DETAILS         :
         setObjectName(root->m_pParent->m_pParent->m_pCertificate->path());
         break;
      case CertificateModel::NodeType::CATEGORY        :
         break;
   }
   setSourceModel(parent);
}

QModelIndex CertificateProxyModel::mapFromSource(const QModelIndex& sourceIndex) const
{
   if (!sourceIndex.isValid())
      return QModelIndex();
   CertificateModel::NodeType type = qvariant_cast<CertificateModel::NodeType>(sourceIndex.data((int)CertificateModel::Role::NodeType));
   switch (type) {
      case CertificateModel::NodeType::CATEGORY        :
      case CertificateModel::NodeType::CERTIFICATE     :
         return QModelIndex();
      case CertificateModel::NodeType::DETAILS_CATEGORY:
      case CertificateModel::NodeType::DETAILS         :
         return createIndex(sourceIndex.row(),sourceIndex.column(),sourceIndex.internalPointer());
   }
   return QModelIndex();
}

QModelIndex CertificateProxyModel::mapToSource(const QModelIndex& proxyIndex) const
{
   return CertificateModel::instance().d_ptr->createIndex(proxyIndex.row(),proxyIndex.column(),proxyIndex.internalPointer());
}

QModelIndex CertificateProxyModel::index( int row, int column, const QModelIndex& parent) const
{
   if ((parent.isValid() && parent.model() != this) || column > 1)
      return QModelIndex();

   CertificateNode* node = parent.isValid()?static_cast<CertificateNode*>(parent.internalPointer()):m_pRoot;

   if ((!node) || row >= node->m_lChildren.size())
      return QModelIndex();

   return createIndex(row, column, node->m_lChildren[row]);
}

QModelIndex CertificateProxyModel::parent( const QModelIndex& index ) const
{
   if ((index.model() != this) || (!index.isValid()))
      return QModelIndex();

   CertificateNode* node = static_cast<CertificateNode*>(index.internalPointer());

   if (!node)
      return QModelIndex();

   return (node->m_pParent == m_pRoot)?QModelIndex() : createIndex(node->m_pParent->m_Index, index.column(), node->m_pParent);
}

int CertificateProxyModel::rowCount( const QModelIndex& parent ) const
{
   return parent.isValid()? sourceModel()->rowCount(mapToSource(parent)) : m_pRoot->m_lChildren.size();
}

int CertificateProxyModel::columnCount( const QModelIndex& parent ) const
{
   return sourceModel()->columnCount(mapToSource(parent));
}

QAbstractItemModel* CertificateModelPrivate::getModelCommon(CertificateNode* node)
{
   if (node) {
      if (node->m_Level == CertificateModel::NodeType::CERTIFICATE && (!node->m_IsLoaded))
         node->m_fLoader();

      CertificateProxyModel* m = new CertificateProxyModel(q_ptr,node);

      return m;
   }

   return nullptr;
}

/**
 * This model is a proxy of CertificateModel with only the current certificate
 *
 * Please note that the object ownership will be transferred. To avoid memory
 * leaks, the users of this object must delete it once they are done with it.
 */
QAbstractItemModel* CertificateModelPrivate::model(const Certificate* cert) const
{
   if (!cert)
      return nullptr;
   return const_cast<CertificateModelPrivate*>(this)->getModelCommon(m_hNodes.value(cert));
}

/**
 * Return the list of security checks performed on the certificate as a model
 */
QAbstractItemModel* CertificateModelPrivate::checksModel(const Certificate* cert) const
{
   if (!cert)
      return nullptr;

   CertificateNode* node = m_hNodes.value(cert);

   if (!node)
      return nullptr;

   if (node->m_Level == CertificateModel::NodeType::CERTIFICATE && (!node->m_IsLoaded))
         node->m_fLoader();

   if (node->m_lChildren.size() < 2)
      return nullptr;

   return const_cast<CertificateModelPrivate*>(this)->getModelCommon(node->m_lChildren[1]);
}

/**
 * This model is a proxy of CertificateModel with only the current certificate
 *
 * @param idx An index from a CertificateModel or one of its proxies
 *
 * Please note that the object ownership will be transferred. To avoid memory
 * leaks, the users of this object must delete it once they are done with it.
 */
QAbstractItemModel* CertificateModel::singleCertificateModel(const QModelIndex& idx) const
{
   if ((!idx.isValid()))
      return nullptr;

   QModelIndex index = idx;
   while(index.model() != this) {
      QAbstractProxyModel* m = qobject_cast<QAbstractProxyModel*>(const_cast<QAbstractItemModel*>(idx.model()));
      if (!m)
         break;

      index = m->mapToSource(index);
   }

   if ((!index.isValid()))
      return nullptr;

   CertificateNode* node = static_cast<CertificateNode*>(idx.internalPointer());
   return d_ptr->getModelCommon(node);
}

/**
 * Create a view of the CertificateModel with only the certificates
 * associated with an account. This doesn't contain the account
 * own certificates.
 */
QAbstractItemModel* CertificateModelPrivate::createKnownList(const Account* a) const
{
   CertificateNode* cat = const_cast<CertificateModelPrivate*>(this)->getCategory(a);
   return new CertificateProxyModel(const_cast<CertificateModel*>(q_ptr),cat);
}

QAbstractItemModel* CertificateModelPrivate::createBannedList(const Account* a) const
{
   QAbstractItemModel* m = m_hAccBan.value(a);

   if (m)
      return m;

   CertificateNode* cat = const_cast<CertificateModelPrivate*>(this)->createCategory(a->id()+"_"+DRing::Certificate::Status::BANNED,QString(),QString());

   m = new CertificateProxyModel(const_cast<CertificateModel*>(q_ptr),cat);

   m_hAccBan[a] = m;
   m_hAccBanCat[a] = cat;

   return m;
}

QAbstractItemModel* CertificateModelPrivate::createAllowedList(const Account* a) const
{
   QAbstractItemModel* m = m_hAccAllow.value(a);

   if (m)
      return m;

   CertificateNode* cat = const_cast<CertificateModelPrivate*>(this)->createCategory(a->id()+"_"+DRing::Certificate::Status::ALLOWED,QString(),QString());

   m = new CertificateProxyModel(const_cast<CertificateModel*>(q_ptr),cat);

   m_hAccAllow[a] = m;
   m_hAccAllowCat[a] = cat;

   return m;
}

bool CertificateModelPrivate::allowCertificate(Certificate* c, Account* a)
{
   if ((!a) || (!c))
      return false;

   //Make sure the lists exist
   createAllowedList(a);
   createBannedList(a);

   CertificateNode* allow   = m_hAccAllowCat .value( a );
   CertificateNode* ban     = m_hAccBanCat   .value( a );
   CertificateNode* sibling = m_hNodes       .value( c );

   //Check if it's already there
   if (isPartOf(sibling, allow))
      return true;

   //Notify the daemon
   ConfigurationManager::instance().setCertificateStatus(
      a->id      (),
      c->remoteId(),
      DRing::Certificate::Status::ALLOWED
   );

   //Check if the certificate isn't banned
   if (isPartOf(sibling, ban))
      removeFromTree(c, ban);

   //Add it
   addToTree(c, allow);

   return true;
}

/**
 * Ban a certificate
 * @warning This method have a O(N) complexity where N is the number of
 * allowed certificate for account a
 */
bool CertificateModelPrivate::banCertificate(Certificate* c, Account* a)
{
   if ((!a) || (!c))
      return false;

   //Make sure the lists exist
   createAllowedList(a);
   createBannedList(a);

   CertificateNode* allow   = m_hAccAllowCat .value( a );
   CertificateNode* ban     = m_hAccBanCat   .value( a );
   CertificateNode* sibling = m_hNodes       .value( c );

   //Check if it's already there
   if (isPartOf(sibling, ban))
      return true;

   //Notify the daemon
   ConfigurationManager::instance().setCertificateStatus(
      a->id      (),
      c->remoteId(),
      DRing::Certificate::Status::BANNED
   );

   //Check if the certificate isn't allowed
   if (isPartOf(sibling, allow))
      removeFromTree(c, allow);

   //Add it
   addToTree(c, ban);

   return true;
}

void CertificateModelPrivate::slotCertificateStateChanged(const QString& accountId, const QString& certId, const QString& state)
{
    if( auto a = AccountModel::instance().getById(accountId.toLatin1())) {
        auto c = q_ptr->getCertificateFromId(certId, a);

        //Make sure the lists exist
        createAllowedList(a);
        createBannedList(a);

        CertificateNode* allow   = m_hAccAllowCat.value(a);
        CertificateNode* ban     = m_hAccBanCat.value(a) ;
        CertificateNode* sibling = m_hNodes.value(c);

        if (state == DRing::Certificate::Status::ALLOWED) {
            //Remove it from the ban list
            if (isPartOf(sibling, ban))
                removeFromTree(c, ban);

            //Add it to the allow list
            if (not isPartOf(sibling, allow) )
                addToTree(c, allow);
        } else if (state == DRing::Certificate::Status::BANNED) {
            //Remove it from the allow list
            if (isPartOf(sibling, allow))
                removeFromTree(c, allow);

            //Add it to the ban list
            if (not isPartOf(sibling, ban) )
                addToTree(c, ban);
        } else if (state == DRing::Certificate::Status::UNDEFINED) {
            //should not happen
            qWarning() << "certificate status changed to UNDEFINED" << certId << "for account" << accountId;
        } else {
            //should not happen either
            qWarning() << "unknown certificate status" << certId << state << "for account" << accountId;
        }
    } else {
        qWarning() << "certificate status changed for unknown account" << accountId << certId;
    }
}

void CertificateModel::collectionAddedCallback(CollectionInterface* collection)
{
   Q_UNUSED(collection)
}

bool CertificateModel::addItemCallback(const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool CertificateModel::removeItemCallback(const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}


#include <certificatemodel.moc>
