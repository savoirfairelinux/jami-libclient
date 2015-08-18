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
#include "recordingmodel.h"

//Qt
#include <QtCore/QMimeData>
#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>

//Ring
#include "recording.h"
#include "localtextrecordingcollection.h"

//DRing
#include "dbus/configurationmanager.h"

struct RecordingNode
{

   enum class Type {
      TOP_LEVEL,
      SESSION  ,
      TAG      ,
   };

   RecordingNode(RecordingNode::Type type);

   //Attributes
   RecordingNode::Type     m_Type     ;
   int                     m_Index    ;
   QString                 m_CatName  ;
   Media::Recording*       m_pRec     ;
   QVector<RecordingNode*> m_lChildren;
   RecordingNode*          m_pParent  ;

};

class RecordingModelPrivate
{
public:
   RecordingModelPrivate(Media::RecordingModel* parent);

   //Attributes
   QVector<RecordingNode*>        m_lCategories             ;
   static Media::RecordingModel*  m_spInstance              ;
   RecordingNode*                 m_pText                   ;
   RecordingNode*                 m_pAudioVideo             ;
   LocalTextRecordingCollection*  m_pTextRecordingCollection;
   //RecordingNode*                 m_pFiles     ; //TODO uncomment when implemented in DRing

private:
   Media::RecordingModel* q_ptr;
};

Media::RecordingModel* RecordingModelPrivate::m_spInstance = nullptr;

RecordingNode::RecordingNode(RecordingNode::Type type) :
   m_Type(type),m_pParent(nullptr), m_pRec(nullptr), m_Index(-1)
{

}

RecordingModelPrivate::RecordingModelPrivate(Media::RecordingModel* parent) : q_ptr(parent),m_pText(nullptr),
m_pAudioVideo(nullptr)/*,m_pFiles(nullptr)*/
{

}

Media::RecordingModel::~RecordingModel()
{
   delete d_ptr;
}

Media::RecordingModel::RecordingModel(QObject* parent) : QAbstractItemModel(parent), CollectionManagerInterface<Media::Recording>(this),
d_ptr(new RecordingModelPrivate(this))
{
   setObjectName("RecordingModel");

   d_ptr->m_pTextRecordingCollection = addCollection<LocalTextRecordingCollection>();

   d_ptr->m_pTextRecordingCollection->listId([](const QList<CollectionInterface::Element>& e) {
      //TODO
      Q_UNUSED(e);
   });
}

Media::RecordingModel* Media::RecordingModel::instance()
{
   if (! RecordingModelPrivate::m_spInstance )
      RecordingModelPrivate::m_spInstance = new RecordingModel(QCoreApplication::instance());
   return RecordingModelPrivate::m_spInstance;
}

QHash<int,QByteArray> Media::RecordingModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
   }*/
   return roles;
}

//Do nothing
bool Media::RecordingModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

///Get bookmark model data RecordingNode::Type and Call::Role
QVariant Media::RecordingModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();

   //RecordingNode* modelItem = static_cast<RecordingNode*>(index.internalPointer());

   switch (role) {
      case Qt::DisplayRole:
         return "foo";
   }

   return QVariant();
}

///Get header data
QVariant Media::RecordingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("Recordings"));
   return QVariant();
}

///Get the number of child of "parent"
int Media::RecordingModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid())
      return d_ptr->m_lCategories.size();
   else {
      const RecordingNode* modelItem = static_cast<RecordingNode*>(parent.internalPointer());
      return modelItem->m_lChildren.size();
   }
}

Qt::ItemFlags Media::RecordingModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

///There is only 1 column
int Media::RecordingModel::columnCount ( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1;
}

///Get the bookmark parent
QModelIndex Media::RecordingModel::parent( const QModelIndex& idx) const
{
   if (!idx.isValid())
      return QModelIndex();

   RecordingNode* modelItem = static_cast<RecordingNode*>(idx.internalPointer());

   if (modelItem->m_Type == RecordingNode::Type::SESSION) {
      RecordingNode* item = static_cast<RecordingNode*>(modelItem)->m_pParent;

      if (item)
         return createIndex(item->m_Index,0,item);

   }

   return QModelIndex();
} //parent

///Get the index
QModelIndex Media::RecordingModel::index(int row, int column, const QModelIndex& parent) const
{
   if (column || (!parent.isValid() && row >= d_ptr->m_lCategories.size()))
      return QModelIndex();

   const RecordingNode* modelItem = parent.isValid() ? static_cast<const RecordingNode*>(parent.internalPointer()) : d_ptr->m_lCategories[row];

   if (row >= modelItem->m_lChildren.size())
      return QModelIndex();

   return createIndex(row,0,modelItem->m_lChildren[row]);
}

bool Media::RecordingModel::addItemCallback(const Media::Recording* item)
{
   Q_UNUSED(item)

   //Create some categories
   if (d_ptr->m_lCategories.size() == 0) {
      d_ptr->m_pText = new RecordingNode(RecordingNode::Type::TOP_LEVEL);
      d_ptr->m_pText->m_CatName = tr("Text messages");
      d_ptr->m_pText->m_Index = 0;
      d_ptr->m_lCategories << d_ptr->m_pText;

      d_ptr->m_pAudioVideo = new RecordingNode(RecordingNode::Type::TOP_LEVEL);
      d_ptr->m_pAudioVideo->m_CatName = tr("Audio/Video");
      d_ptr->m_pAudioVideo->m_Index = 1;
      d_ptr->m_lCategories << d_ptr->m_pAudioVideo;

      /*d_ptr->m_pFiles = new RecordingNode(RecordingNode::Type::TOP_LEVEL);
      d_ptr->m_pFiles->m_CatName = tr("Files");
      d_ptr->m_pFiles->m_Index = 2;
      d_ptr->m_lCategories << m_pFiles;*/
   }

   //Categorize by general media group
   RecordingNode* parent = nullptr;

   if (item->type() == Media::Recording::Type::TEXT)
      parent = d_ptr->m_pText;
   else if (item->type() == Media::Recording::Type::AUDIO_VIDEO)
      parent = d_ptr->m_pAudioVideo;
   /*else if (item->type() == Media::Recording::Type::FILE))
      parent = d_ptr->m_pFiles;*/

   //Insert the item
   if (parent) {

      beginInsertRows(index(parent->m_Index,0),parent->m_lChildren.size(),parent->m_lChildren.size());

      RecordingNode* n = new RecordingNode       ( RecordingNode::Type::SESSION );
      n->m_pRec        = const_cast<Recording*>  ( item                         );
      n->m_Index       = parent->m_lChildren.size(                              );
      parent->m_lChildren << n;

      endInsertRows();

      return true;
   }

   return false;
}

bool Media::RecordingModel::removeItemCallback(const Media::Recording* item)
{
   Q_UNUSED(item)
   return false;
}

bool Media::RecordingModel::clearAllCollections() const
{
   foreach (CollectionInterface* backend, collections()) {
      if (backend->supportedFeatures() & CollectionInterface::SupportedFeatures::ADD) {
         backend->clear();
      }
   }
   return true;
}

void Media::RecordingModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

///Set where the call recordings will be saved
void Media::RecordingModel::setRecordPath(const QUrl& path)
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   configurationManager.setRecordPath(path.path());
}

///Return the path where recordings are going to be saved
QUrl Media::RecordingModel::recordPath() const
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   return QUrl(configurationManager.getRecordPath());
}

///are all calls recorded by default
bool Media::RecordingModel::isAlwaysRecording() const
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   return configurationManager.getIsAlwaysRecording();
}

///Set if all calls needs to be recorded
void Media::RecordingModel::setAlwaysRecording(bool record)
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   configurationManager.setIsAlwaysRecording   ( record );
}

///Create or load the recording associated with the ContactMethod cm
Media::TextRecording* Media::RecordingModel::createTextRecording(const ContactMethod* cm)
{
   Media::TextRecording* r = d_ptr->m_pTextRecordingCollection->createFor(cm);

   return r;
}
