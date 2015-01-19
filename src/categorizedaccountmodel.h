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
#ifndef CATEGORIZEDACCOUNTMODEL_H
#define CATEGORIZEDACCOUNTMODEL_H

#include "typedefs.h"
#include <QtCore/QAbstractItemModel>

class CategorizedAccountModelPrivate;

class LIB_EXPORT CategorizedAccountModel : public QAbstractItemModel {
   Q_OBJECT
public:
   explicit CategorizedAccountModel(QObject* parent = nullptr);
   virtual ~CategorizedAccountModel();
   static CategorizedAccountModel* instance();

   //Abstract model member
   virtual QVariant data        (const QModelIndex& index, int role = Qt::DisplayRole         ) const override;
   virtual int rowCount         (const QModelIndex& parent = QModelIndex()                    ) const override;
   virtual int columnCount      (const QModelIndex& parent = QModelIndex()                    ) const override;
   virtual Qt::ItemFlags flags  (const QModelIndex& index                                     ) const override;
   virtual bool setData         (const QModelIndex& index, const QVariant &value, int role    )       override;
   virtual QModelIndex   index  (int row, int column, const QModelIndex& parent=QModelIndex() ) const override;
   virtual QModelIndex parent   (const QModelIndex& index                                     ) const override;
   virtual QVariant headerData  (int section, Qt::Orientation orientation, int role           ) const override;

   //Getter
   QModelIndex mapToSource  (const QModelIndex& idx) const;
   QModelIndex mapFromSource(const QModelIndex& idx) const;


private:

   enum Categories {
      SERVER ,
      IP2IP  ,
   };

   const QScopedPointer<CategorizedAccountModelPrivate> d_ptr;

   //Singleton
   static CategorizedAccountModel* m_spInstance;

private Q_SLOTS:
   void slotDataChanged(const QModelIndex& tl,const QModelIndex& br);
   void slotLayoutchanged();

};

#endif
