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
#ifndef MANAGERMODEL_H
#define MANAGERMODEL_H

#include <QtCore/QAbstractListModel>

//Qt
#include <QtCore/QStringList>
class QItemSelectionModel;

//Ring
#include "../typedefs.h"

class ManagerModelPrivate;

namespace Audio {

class LIB_EXPORT ManagerModel   : public QAbstractListModel {
   Q_OBJECT
public:

   enum class Manager {
      ALSA =0,
      PULSE=1,
      JACK =2,
      ERROR=3,
   };
   explicit ManagerModel(const QObject* parent);
   virtual ~ManagerModel();

   //Models function
   virtual QVariant      data    ( const QModelIndex& index, int role = Qt::DisplayRole ) const override;
   virtual int           rowCount( const QModelIndex& parent = QModelIndex()            ) const override;
   virtual Qt::ItemFlags flags   ( const QModelIndex& index                             ) const override;
   virtual bool          setData ( const QModelIndex& index, const QVariant &value, int role)   override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getters
   QItemSelectionModel* selectionModel() const;
   Manager              currentManager() const;

Q_SIGNALS:
   void currentManagerChanged(Manager);

private:
   QScopedPointer<ManagerModelPrivate> d_ptr;
   Q_DECLARE_PRIVATE(ManagerModel)
};

}

#endif
