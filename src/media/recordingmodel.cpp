/****************************************************************************
 *    Copyright (C) 2015-2019 Savoir-faire Linux Inc.                               *
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
#include "media.h"

//DRing
#include "dbus/configurationmanager.h"

struct RecordingNode final
{
   ~RecordingNode();

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
   media::Recording*       m_pRec     ;
   QVector<RecordingNode*> m_lChildren;
   RecordingNode*          m_pParent  ;

};

class RecordingModelPrivate final : public QObject
{
   Q_OBJECT
public:
   explicit RecordingModelPrivate(media::RecordingModel* parent);
   ~RecordingModelPrivate();

   //Attributes
   QVector<RecordingNode*>        m_lCategories             ;
   RecordingNode*                 m_pText          {nullptr};
   RecordingNode*                 m_pAudioVideo    {nullptr};
   int                            m_UnreadCount    { 0     };

   //RecordingNode*                 m_pFiles     ; //TODO uncomment when implemented in DRing

   void forwardInsertion(const QMap<QString,QString>& message, ContactMethod* cm, media::Media::Direction direction);
   void updateUnreadCount(const int count);

private:
    media::RecordingModel* q_ptr;
};

RecordingNode::RecordingNode(RecordingNode::Type type) :
   m_Type(type),m_pParent(nullptr), m_pRec(nullptr), m_Index(-1)
{

}

RecordingNode::~RecordingNode()
{
   foreach(RecordingNode* c, m_lChildren)
      delete c;
}

RecordingModelPrivate::RecordingModelPrivate(media::RecordingModel* parent) : q_ptr(parent),m_pText(nullptr),
m_pAudioVideo(nullptr)/*,m_pFiles(nullptr)*/
{

}

RecordingModelPrivate::~RecordingModelPrivate()
{
   if (m_pText)
      delete m_pText;

   if (m_pAudioVideo)
      delete m_pAudioVideo;
}

void RecordingModelPrivate::forwardInsertion(const QMap<QString,QString>& message, ContactMethod* cm, media::Media::Direction direction)
{
   Q_UNUSED(message)
   Q_UNUSED(direction)
}

void RecordingModelPrivate::updateUnreadCount(const int count)
{
    m_UnreadCount += count;
    if (m_UnreadCount <= 0) {
        m_UnreadCount = 0;
    }
    emit q_ptr->unreadMessagesCountChanged(m_UnreadCount);
}

media::RecordingModel::~RecordingModel()
{
   delete d_ptr;
}

media::RecordingModel::RecordingModel(QObject* parent) : QAbstractItemModel(parent), CollectionManagerInterface<Recording>(this),
d_ptr(new RecordingModelPrivate(this))
{
   setObjectName("RecordingModel");
}

media::RecordingModel& media::RecordingModel::instance()
{
    static auto instance = new RecordingModel(QCoreApplication::instance());
    return *instance;
}

QHash<int,QByteArray> media::RecordingModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
   }*/
   return roles;
}

//Do nothing
bool media::RecordingModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

///Get bookmark model data RecordingNode::Type and Call::Role
QVariant media::RecordingModel::data( const QModelIndex& index, int role) const
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
QVariant media::RecordingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
   Q_UNUSED(section)
   if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
      return QVariant(tr("Recordings"));
   return QVariant();
}

///Get the number of child of "parent"
int media::RecordingModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid())
      return d_ptr->m_lCategories.size();
   else {
      const RecordingNode* modelItem = static_cast<RecordingNode*>(parent.internalPointer());
      return modelItem->m_lChildren.size();
   }
}

Qt::ItemFlags media::RecordingModel::flags( const QModelIndex& index ) const
{
   if (!index.isValid())
      return Qt::NoItemFlags;

   return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

///There is only 1 column
int media::RecordingModel::columnCount ( const QModelIndex& parent) const
{
   Q_UNUSED(parent)
   return 1;
}

///Get the bookmark parent
QModelIndex media::RecordingModel::parent( const QModelIndex& idx) const
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
QModelIndex media::RecordingModel::index(int row, int column, const QModelIndex& parent) const
{
   if (column || (!parent.isValid() && row >= d_ptr->m_lCategories.size()))
      return QModelIndex();

   const RecordingNode* modelItem = parent.isValid() ? static_cast<const RecordingNode*>(parent.internalPointer()) : d_ptr->m_lCategories[row];

   if (row >= modelItem->m_lChildren.size())
      return QModelIndex();

   return createIndex(row,0,modelItem->m_lChildren[row]);
}

bool media::RecordingModel::addItemCallback(const Recording* item)
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

   if (item->type() == Recording::Type::TEXT)
      parent = d_ptr->m_pText;
   else if (item->type() == Recording::Type::AUDIO_VIDEO)
      parent = d_ptr->m_pAudioVideo;
   /*else if (item->type() == Recording::Type::FILE))
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

bool media::RecordingModel::removeItemCallback(const Recording* item)
{
   Q_UNUSED(item)
   return false;
}

bool media::RecordingModel::clearAllCollections() const
{
    foreach (CollectionInterface* backend, collections(CollectionInterface::SupportedFeatures::CLEAR)) {
        backend->clear();
    }
    return true;
}

///Deletes all recordings (which are possible to delete) and clears model
void media::RecordingModel::clear()
{
    beginResetModel();
    clearAllCollections();
    endResetModel();
}

void media::RecordingModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

///Set where the call recordings will be saved
void media::RecordingModel::setRecordPath(const QString& path)
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   configurationManager.setRecordPath(path);
}

///Return the path where recordings are going to be saved
QString media::RecordingModel::recordPath() const
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   return configurationManager.getRecordPath();
}

///are all calls recorded by default
bool media::RecordingModel::isAlwaysRecording() const
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   return configurationManager.getIsAlwaysRecording();
}

///Set if all calls needs to be recorded
void media::RecordingModel::setAlwaysRecording(bool record)
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   configurationManager.setIsAlwaysRecording   ( record );
}

int  media::RecordingModel::unreadCount() const
{
    return d_ptr->m_UnreadCount;
}

#include <recordingmodel.moc>
