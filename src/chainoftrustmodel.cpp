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
#include "chainoftrustmodel.h"

#include <certificate.h>
#include <extensions/securityevaluationextension.h>
#include <securityevaluationmodel.h>

struct ChainedCertificateNode
{
   ChainedCertificateNode(Certificate* c) : m_pCertificate(c),m_pParent(nullptr),m_pChild(nullptr){}
   Certificate* m_pCertificate;
   ChainedCertificateNode* m_pParent;
   ChainedCertificateNode* m_pChild;
};

class ChainOfTrustModelPrivate
{
public:
   Certificate* m_pCertificate;
   ChainedCertificateNode* m_pRoot;
};

ChainOfTrustModel::ChainOfTrustModel(Certificate* c) : QAbstractItemModel(c), d_ptr(new ChainOfTrustModelPrivate())
{
   d_ptr->m_pCertificate = c;

   ChainedCertificateNode* n = nullptr;

   Certificate* cn = c;

   while (cn) {
      ChainedCertificateNode* prev = n;

      n               = new ChainedCertificateNode(cn);
      n   ->m_pChild  = prev                          ;

      if (prev)
         prev->m_pParent = n                          ;

      Certificate* next = cn->signedBy();

      //Prevent infinite loop of self signed certificates
      cn = next == cn ? nullptr : next;
   }

   d_ptr->m_pRoot = n;

   emit layoutChanged();
}

ChainOfTrustModel::~ChainOfTrustModel()
{
   delete d_ptr;
}

QHash<int,QByteArray> ChainOfTrustModel::roleNames() const
{
   static QHash<int,QByteArray> roles;

   if (roles.isEmpty()) {
      roles[(int) Role::OBJECT] = "object";
   }

   return roles;
}

QVariant ChainOfTrustModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   ChainedCertificateNode* c = static_cast<ChainedCertificateNode*>(index.internalPointer());

   switch(role) {
      case Qt::DisplayRole:
         return c->m_pCertificate->publicKeyId();
      case Qt::DecorationRole:
         if (c->m_pCertificate->extension<SecurityEvaluationExtension>()) {
            return c->m_pCertificate->extension<SecurityEvaluationExtension>()->securityLevelIcon(c->m_pCertificate);
         }
         break;
      case (int) Role::OBJECT:
         return QVariant::fromValue(c->m_pCertificate);
      case (int) Role::SECURITY_LEVEL:
         if (c->m_pCertificate->extension<SecurityEvaluationExtension>()) {
            return QVariant::fromValue(
               c->m_pCertificate->extension<SecurityEvaluationExtension>()->securityLevel(c->m_pCertificate)
            );
         }
         break;
   }

   return QVariant();
}

bool ChainOfTrustModel::setData( const QModelIndex& index, const QVariant &value, int role )
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )

   return false;
}

int ChainOfTrustModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid())
      return 1;

   ChainedCertificateNode* c = static_cast<ChainedCertificateNode*>(parent.internalPointer());

   //The last node is the "leaf" certificate that is being viewed
   return c->m_pChild ? 1 : 0;
}

Qt::ItemFlags ChainOfTrustModel::flags( const QModelIndex& index ) const
{
   return index.isValid() ? Qt::ItemIsEnabled | Qt::ItemIsSelectable : Qt::NoItemFlags;
}

int ChainOfTrustModel::columnCount( const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return 1;
}

QModelIndex ChainOfTrustModel::parent( const QModelIndex& idx ) const
{
   if (!idx.isValid())
      return QModelIndex();

   ChainedCertificateNode* c = static_cast<ChainedCertificateNode*>(idx.internalPointer());

   //Root
   if (!c->m_pParent)
      return QModelIndex();

   return createIndex(0,0,c->m_pParent);
}

QModelIndex ChainOfTrustModel::index( int row, int column, const QModelIndex& parent ) const
{
   if (row || column)
      return QModelIndex();

   if (!parent.isValid())
      return createIndex(0,0,d_ptr->m_pRoot);

   ChainedCertificateNode* c = static_cast<ChainedCertificateNode*>(parent.internalPointer());

   return createIndex(0,0,c->m_pChild);
}

QVariant ChainOfTrustModel::headerData( int section, Qt::Orientation o, int role ) const
{
   if ((!section) && role == Qt::DisplayRole && o == Qt::Horizontal)
      return tr("Chain of trust");

   return QVariant();
}