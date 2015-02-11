/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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

//LibSTDC++
#include <functional>

//Ring
#include "certificate.h"

struct CertificateNode {

   enum class Level {
      CATEGORY         = 0,
      CERTIFICATE      = 1,
      DETAILS_CATEGORY = 2,
      DETAILS          = 3,
   };

   CertificateNode(int index, Level level, CertificateNode* parent, Certificate* cert);
   void setStrings(const QString& col1, const QVariant& col2, const QString& tooltip);

   //Attributes
   QVector<CertificateNode*> m_lChildren   ;
   CertificateNode*          m_pParent     ;
   Certificate*              m_pCertificate;
   CertificateNode::Level    m_Level       ;
   int                       m_Index       ;
   QString                   m_Col1        ;
   QVariant                  m_Col2        ;
   QString                   m_ToolTip     ;
   std::function<void()>     m_fLoader     ;
   bool                      m_IsLoaded    ;
};

class CertificateModelPrivate
{
public:
   CertificateModelPrivate(CertificateModel* parent);
   virtual ~CertificateModelPrivate();

   //Helper
   CertificateNode* defaultCategory();
   CertificateNode* addToTree(Certificate* cert, CertificateNode* category = nullptr);

   //Attributes
   QVector<CertificateNode*>   m_lTopLevelNodes  ;
   QHash<QString,Certificate*> m_hCertificates   ;
   CertificateNode*            m_pDefaultCategory;

   //Singleton
   static CertificateModel* m_spInstance;

private:
   CertificateModel* q_ptr;
};

CertificateModel* CertificateModelPrivate::m_spInstance = nullptr;

CertificateModelPrivate::~CertificateModelPrivate()
{
   foreach(CertificateNode* node, m_lTopLevelNodes) {
      delete node->m_pCertificate;
      delete node;
   }
}

CertificateNode::CertificateNode(int index, Level level, CertificateNode* parent, Certificate* cert) :
   m_pParent(parent), m_pCertificate(cert), m_Level(level), m_Index(index), m_IsLoaded(true)
{

}

CertificateModelPrivate::CertificateModelPrivate(CertificateModel* parent) : q_ptr(parent),
 m_pDefaultCategory(nullptr)
{

}

CertificateModel::CertificateModel(QObject* parent) : QAbstractItemModel(parent),
 d_ptr(new CertificateModelPrivate(this))
{
   setObjectName("CertificateModel");
}

CertificateModel::~CertificateModel()
{
   delete d_ptr;
}

CertificateModel* CertificateModel::instance()
{
   if (!CertificateModelPrivate::m_spInstance)
      CertificateModelPrivate::m_spInstance = new CertificateModel(QCoreApplication::instance());
   return CertificateModelPrivate::m_spInstance;
}

QHash<int,QByteArray> CertificateModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

void CertificateNode::setStrings(const QString& col1, const QVariant& col2, const QString& tooltip)
{
   m_Col1    = col1;
   m_Col2    = col2;
   m_ToolTip = tooltip;
}

CertificateNode* CertificateModelPrivate::defaultCategory()
{
   if (!m_pDefaultCategory) {
      const int idx = m_hCertificates.size();

      m_pDefaultCategory = new CertificateNode(idx, CertificateNode::Level::CATEGORY, nullptr, nullptr);
      m_pDefaultCategory->setStrings(QObject::tr("Default"),QObject::tr("Certificate not associated with a group"),QString());

      q_ptr->beginInsertRows(QModelIndex(), idx, idx);
      m_lTopLevelNodes << m_pDefaultCategory;
      q_ptr->endInsertRows();
   }

   return m_pDefaultCategory;
}

CertificateNode* CertificateModelPrivate::addToTree(Certificate* cert, CertificateNode* category)
{
   if (!category)
      category = defaultCategory();

   const int idx = category->m_lChildren.size();

   CertificateNode* node = new CertificateNode(idx, CertificateNode::Level::CERTIFICATE, category, cert);
   node->setStrings(QObject::tr("A certificate"),QObject::tr("An organisation"),QString());


   const QModelIndex parent = q_ptr->createIndex(category->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),category);
   q_ptr->beginInsertRows(parent, idx, idx);
   category->m_lChildren << node;
   q_ptr->endInsertRows();

   //Lazy loaded function to reduce the overhead of this (mostly hidden) model
   node->m_fLoader = [this,node,cert]() {
      node->m_IsLoaded = true;
      const QModelIndex index = q_ptr->createIndex(node->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),node);

      //Insert the check and details categories
      q_ptr->beginInsertRows(index, 0, static_cast<int>(CertificateModel::Columns::NAME ));
      CertificateNode* details = new CertificateNode(static_cast<int>(CertificateModel::Columns::NAME ), CertificateNode::Level::DETAILS_CATEGORY, node, nullptr);
      CertificateNode* checks  = new CertificateNode(static_cast<int>(CertificateModel::Columns::VALUE), CertificateNode::Level::DETAILS_CATEGORY, node, nullptr);
      details->setStrings(QObject::tr("Details"),QString(),QObject::tr("The content of the certificate")       );
      checks ->setStrings(QObject::tr("Checks") ,QString(),QObject::tr("Various security related information") );
      node->m_lChildren << details; node->m_lChildren << checks;
      q_ptr->endInsertRows();

      const int detailsC(enum_class_size<Certificate::Details>()), checksC(enum_class_size<Certificate::Checks>());

      //Insert the details
      const QModelIndex detailsI(q_ptr->createIndex(details->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),details));
      q_ptr->beginInsertRows(detailsI, static_cast<int>(CertificateModel::Columns::NAME ), detailsC);
      for (const Certificate::Details detail : EnumIterator<Certificate::Details>()) {
         CertificateNode* d = new CertificateNode(details->m_lChildren.size(), CertificateNode::Level::DETAILS, details, nullptr);
         d->setStrings(cert->getName(detail),cert->detailResult(detail),cert->getDescription(detail)       );
         details->m_lChildren << d;
      }
      q_ptr->endInsertRows();

      //Insert the checks
      const QModelIndex checksI(q_ptr->createIndex(checks->m_Index,static_cast<int>(CertificateModel::Columns::NAME ),checks));
      q_ptr->beginInsertRows(checksI, static_cast<int>(CertificateModel::Columns::NAME ), checksC);
      for (const Certificate::Checks check : EnumIterator<Certificate::Checks>()) {
         if (cert->checkResult(check) != Certificate::CheckValues::UNSUPPORTED) {
            CertificateNode* d = new CertificateNode(checks->m_lChildren.size(), CertificateNode::Level::DETAILS, checks, nullptr);
            d->setStrings(cert->getName(check),static_cast<bool>(cert->checkResult(check)),cert->getDescription(check));
            checks->m_lChildren << d;
         }
      }
      q_ptr->endInsertRows();
   };
   node->m_IsLoaded = false;

   return node;
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
      switch(role) {
         case Qt::DisplayRole:
         case Qt::EditRole:
            return index.column()?node->m_Col2:node->m_Col1;
         case Qt::ToolTipRole:
            return node->m_ToolTip;
      };
   return QVariant();
}

int CertificateModel::rowCount( const QModelIndex& parent) const
{
   if (!parent.isValid())
      return d_ptr->m_lTopLevelNodes.size();
   else {
      const CertificateNode* node = static_cast<CertificateNode*>(parent.internalPointer());

      //Load that info only when it is needed
      if (node->m_Level == CertificateNode::Level::CERTIFICATE && (!node->m_IsLoaded))
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

Certificate* CertificateModel::getCertificate(const QUrl& path, Certificate::Type type)
{
   Q_UNUSED(type)
   const QString p = path.path();

   Certificate* cert = d_ptr->m_hCertificates[p];

   //The certificate is not loaded yet
   if (!cert) {
      cert = new Certificate(path);

      //Add it to the model
      d_ptr->addToTree(cert);
   }

   return cert;
}