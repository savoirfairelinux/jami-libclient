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
#ifndef RECORDINGMODEL_H
#define RECORDINGMODEL_H

#include <QtCore/QAbstractItemModel>
#include <QtCore/QHash>
#include <QtCore/QStringList>

//Ring
#include "collectionmanagerinterface.h"
#include "collectioninterface.h"
#include "typedefs.h"
#include "contactmethod.h"

class RecordingModelPrivate;
class ContactMethod;

namespace Media {
   class Recording;
   class TextRecording;
   class AVRecording;

/**
 * This model host the Ring recordings. Recording sessions span one or
 * more media, themselves possibly spanning multiple communications. They
 * can be paused indefinitely and resumed. Those events cause the recording
 * to be tagged at a specific point.
 *
 * The purpose of this model is mostly to track the recordings and handle
 * housekeeping task. It could also be used to manage recordings, move them
 * and so on.
 */
class LIB_EXPORT RecordingModel :  public QAbstractItemModel, public CollectionManagerInterface<Recording>
{
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:

   //Constructor
   virtual ~RecordingModel() {}
   explicit RecordingModel(QObject* parent);

   virtual bool clearAllCollections() const override;

   //Model implementation
   virtual bool          setData     ( const QModelIndex& index, const QVariant &value, int role   )       override;
   virtual QVariant      data        ( const QModelIndex& index, int role = Qt::DisplayRole        ) const override;
   virtual int           rowCount    ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual Qt::ItemFlags flags       ( const QModelIndex& index                                    ) const override;
   virtual int           columnCount ( const QModelIndex& parent = QModelIndex()                   ) const override;
   virtual QModelIndex   parent      ( const QModelIndex& index                                    ) const override;
   virtual QModelIndex   index       ( int row, int column, const QModelIndex& parent=QModelIndex()) const override;
   virtual QVariant      headerData  ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
   virtual QHash<int,QByteArray> roleNames() const override;

   //Getter
   bool isAlwaysRecording() const;
   QUrl recordPath       () const;

   //Setter
   void setAlwaysRecording( bool        record );
   void setRecordPath     ( const QUrl& path   );

   //Mutator
   TextRecording* createTextRecording(const ContactMethod* cm);

   //Singleton
   static RecordingModel* instance();

private:
   RecordingModelPrivate* d_ptr;
   Q_DECLARE_PRIVATE(RecordingModel)

   //Collection interface
   virtual void collectionAddedCallback(CollectionInterface* backend) override;
   virtual bool addItemCallback        (const Recording* item       ) override;
   virtual bool removeItemCallback     (const Recording* item       ) override;
};

}

#endif
