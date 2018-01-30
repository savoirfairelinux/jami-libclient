/****************************************************************************
 *   Copyright (C) 2013-2018 Savoir-faire Linux                          *
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

//Qt
#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>

//Ring
#include "typedefs.h"
#include "collectionmanagerinterface.h"

//Ring
class Macro;
class MacroModelPrivate;

///MacroModel: DTMF emulators model
class LIB_EXPORT MacroModel : public QAbstractItemModel, public CollectionManagerInterface<Macro>
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
   friend class Macro;
   friend class MacroPrivate;

public:

   /**
    * Interface to interpret DTMFs instead of using the daemon directly
    *
    * This can be useful is a client want to add animations
    */
   class MacroListener {
   public:
      explicit MacroListener() {}
      virtual void addDTMF(const QString& sequence) = 0;
      virtual ~MacroListener() {}
   };

   static MacroModel& instance();
   static void addListener(MacroListener* interface);

   enum MacroFields {
      Name        = 100,
      Category    = 101,
      Delay       = 102,
      Description = 103,
      Sequence    = 104
   };

   //Getters
   Macro* getCurrentMacro(); //TODO replace with a selection model

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   ) override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;

private:
   //Singleton constructor
   explicit MacroModel(QObject* parent = nullptr);
   virtual ~MacroModel();

   MacroModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(MacroModel)

   //Collection interface
   virtual void collectionAddedCallback(CollectionInterface* backend) override;
   virtual bool addItemCallback        (const Macro* item           ) override;
   virtual bool removeItemCallback     (const Macro* item           ) override;

public Q_SLOTS:
   Macro* newMacro(const QString& id = QString());
   bool removeMacro(const QModelIndex& idx);
   void setCurrent(const QModelIndex& current, const QModelIndex& previous); //TODO use a selectionmodel
   void save();

Q_SIGNALS:
   void addAction(const QVariant&);
   void selectMacro(Macro* macro);
};

