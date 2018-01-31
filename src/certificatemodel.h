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
#pragma once

#include <QtCore/QAbstractItemModel>
#include "typedefs.h"

#include "certificate.h"
#include "itemdataroles.h"

class Account;

class CertificateModelPrivate;

class LIB_EXPORT CertificateModel : public QAbstractItemModel, public CollectionManagerInterface<Certificate>
{
   Q_OBJECT
public:
   friend class CertificateProxyModel;
   friend struct CertificateNode;
   friend class Certificate;
   friend class CertificatePrivate;
   friend class Account;
   friend class DaemonCertificateCollectionPrivate;
   friend class DaemonCertificateCollection;

   enum class Role {
      NodeType          = static_cast<int>(Ring::Role::UserRole) + 100,
      isDetail          ,
      isCheck           ,
      detail            ,
      check             ,
      requirePrivateKey ,

      // "Virtual" roles for each certificate values
      DetailRoleBase    = 1000,
   };

   enum class Columns {
      NAME  = 0,
      VALUE = 1,
   };

   enum class NodeType {
      CATEGORY         = 0,
      CERTIFICATE      = 1,
      DETAILS_CATEGORY = 2,
      DETAILS          = 3,
   };

   //Constructor
   explicit CertificateModel(QObject* parent = nullptr);
   virtual ~CertificateModel();

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   )       override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation, int role = Qt::DisplayRole    ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   QAbstractItemModel* singleCertificateModel(const QModelIndex& idx) const;

   //Mutator
   Certificate* getCertificateFromPath(const QString& path, Certificate::Type type = Certificate::Type::NONE);
   Certificate* getCertificateFromPath(const QString& path, Account* a);
   Certificate* getCertificateFromId(const QString& id, Account* a = nullptr, const QString& category = QString());

   //Singleton
   static CertificateModel& instance();

private:

   //Backend interface
   virtual void collectionAddedCallback(CollectionInterface* collection) override;
   virtual bool addItemCallback(const Certificate* item) override;
   virtual bool removeItemCallback(const Certificate* item) override;

   CertificateModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(CertificateModel)
};
Q_DECLARE_METATYPE(CertificateModel::NodeType)
