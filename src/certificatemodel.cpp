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

struct CertificateNode {

   enum class Level {
      CERTIFICATE      = 0,
      DETAILS_CATEGORY = 1,
      DETAILS          = 2,
   };

   CertificateNode();
   QVector<CertificateNode*> m_lChildren;
   CertificateNode* m_pParent;
   Certificate* m_pCertificate;
   CertificateNode::Level m_Level;
};

class CertificateModelPrivate
{
public:
   virtual ~CertificateModelPrivate();
   QVector<CertificateNode*> m_lTopLevelNodes;
};

CertificateModelPrivate::~CertificateModelPrivate()
{
   foreach(CertificateNode* node, m_lTopLevelNodes) {
      delete node->m_pCertificate;
      delete node;
   }
}

CertificateNode::CertificateNode() : m_pParent(nullptr), m_pCertificate(nullptr)
{
   
}

CertificateModel::CertificateModel(QObject* parent) : QAbstractItemModel(parent),
 d_ptr(new CertificateModelPrivate())
{
   
}

CertificateModel::~CertificateModel()
{
   delete d_ptr;
}

bool CertificateModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   return false;
}

QVariant CertificateModel::data( const QModelIndex& index, int role) const
{
   
}

int CertificateModel::rowCount( const QModelIndex& parent) const
{
   
}

Qt::ItemFlags CertificateModel::flags( const QModelIndex& index) const
{
   
}

int CertificateModel::columnCount( const QModelIndex& parent) const
{
   return 1;
}

QModelIndex CertificateModel::parent( const QModelIndex& index) const
{
   
}

QModelIndex CertificateModel::index( int row, int column, const QModelIndex& parent) const
{
   
}

QVariant CertificateModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
   
}

QStringList CertificateModel::mimeTypes() const
{
   
}

QMimeData* CertificateModel::mimeData( const QModelIndexList &indexes) const
{
   
}

bool CertificateModel::dropMimeData( const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent )
{
   
}

