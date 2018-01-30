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

#include "typedefs.h"

//Qt
#include <QtCore/QAbstractTableModel>
class QTimer;
class QItemSelectionModel;

//Ring
#include "collectionmanagerinterface.h"
class Account;
class RingtoneModelPrivate;
class Ringtone;

///CredentialModel: A model for account credentials
class LIB_EXPORT RingtoneModel : public QAbstractTableModel, public CollectionManagerInterface<Ringtone>
{
   Q_OBJECT
public:

   enum Role {
      FullPath  = 100,
   };

   virtual ~RingtoneModel();

   //Model functions
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                 ) const override;
   virtual bool  setData     ( const QModelIndex& index, const QVariant &value, int role)       override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getters
   Ringtone* currentRingTone(Account* a) const;
   QItemSelectionModel* selectionModel(Account* a) const;
   bool isPlaying();

   //Mutator
   void play(const QModelIndex& index);
   bool add(const QUrl& path, Account* autoSelect = nullptr);

   static RingtoneModel& instance();

private:
   explicit RingtoneModel(QObject* parent = nullptr);

   RingtoneModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(RingtoneModel)

   //Collection interface
   virtual void collectionAddedCallback(CollectionInterface* backend) override;
   virtual bool addItemCallback        (const Ringtone* item        ) override;
   virtual bool removeItemCallback     (const Ringtone* item        ) override;

};
