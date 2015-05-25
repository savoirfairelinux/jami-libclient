/****************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                          *
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
#ifndef CATEGORIZED_HISTORY_MODEL_H
#define CATEGORIZED_HISTORY_MODEL_H
//Base
#include "typedefs.h"
#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QStringList>

//Qt

//Ring
#include "call.h"
#include "collectionmanagerinterface.h"

//Typedef
typedef QMap<uint, Call*>  CallMap;
typedef QList<Call*>       CallList;

class HistoryItemNode;
class AbstractHistoryBackend;
class CategorizedHistoryModelPrivate;
class QSortFilterProxyModel;
class QItemSelectionModel;

//TODO split ASAP
///CategorizedHistoryModel: History call manager
class LIB_EXPORT CategorizedHistoryModel : public QAbstractItemModel, public CollectionManagerInterface<Call>
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:
   friend class HistoryItemNode;
   friend class HistoryTopLevelItem;

   //Properties
   Q_PROPERTY(bool hasCollections   READ hasCollections  )

   //Singleton
   static CategorizedHistoryModel* instance();

   //Getters
   int  acceptedPayloadTypes       () const;
   bool isHistoryLimited           () const;
   int  historyLimit               () const;
   const CallMap getHistoryCalls   () const;

   //Backend model implementation
   virtual bool clearAllCollections() const override;

   //Setters
   void setCategoryRole(int role);
   void setHistoryLimited(bool isLimited);
   void setHistoryLimit(int numberOfDays);

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   ) override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const  override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
   virtual QStringList   mimeTypes   (                                                             ) const override;
   virtual QMimeData*    mimeData    ( const QModelIndexList &indexes                              ) const override;
   virtual bool          dropMimeData( const QMimeData*, Qt::DropAction, int, int, const QModelIndex& ) override;
   virtual bool          insertRows  ( int row, int count, const QModelIndex & parent = QModelIndex() ) override;
   virtual QHash<int,QByteArray> roleNames() const override;

   struct LIB_EXPORT SortedProxy {
      enum class Categories {
         DATE      ,
         NAME      ,
         POPULARITY,
         LENGTH    ,
         SPENT_TIME,
         COUNT__
      };

      QSortFilterProxyModel* model                 () const;
      QAbstractItemModel   * categoryModel         () const;
      QItemSelectionModel  * categorySelectionModel() const;
      static CategorizedHistoryModel::SortedProxy* instance();
   };

private:
   //Constructor
   explicit CategorizedHistoryModel();
   ~CategorizedHistoryModel();
   QScopedPointer<CategorizedHistoryModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(CategorizedHistoryModel)

   //Static attributes
   static CategorizedHistoryModel* m_spInstance;

   //Backend interface
   virtual void collectionAddedCallback(CollectionInterface* collection) override;
   virtual bool addItemCallback(const Call* item) override;
   virtual bool removeItemCallback(const Call* item) override;

Q_SIGNALS:
   ///Emitted when the history change (new items, cleared)
   void historyChanged          (            );
   ///Emitted when a new item is added to prevent full reload
   void newHistoryCall          ( Call* call );
};

#endif
