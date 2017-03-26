/****************************************************************************
 *   Copyright (C) 2015-2017 by Savoir-faire Linux                          *
 *   Copyright (C) 2017 by Emmanuel Lepage Vallee                           *
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

// STD
#include <vector>

//Qt
#include <QtCore/QMimeData>
#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>

//Ring
#include "recording.h"
#include "textrecording.h"
#include "avrecording.h"
#include "media.h"
#include "call.h"
#include "localtextrecordingcollection.h"

//DRing
#include "dbus/configurationmanager.h"

struct RecordingNode final
{
    // Types
    enum class Type {
        TOP_LEVEL,
        SESSION  ,
        TAG      ,
    };

    // Constructors
    RecordingNode(RecordingNode::Type type);
    ~RecordingNode();

    //Attributes
    RecordingNode::Type         m_Type      {       };
    int                         m_Index     { -1    };
    QString                     m_CatName   {       };
    Media::Recording*           m_pRec      {nullptr};
    std::vector<RecordingNode*> m_lChildren {       };
    RecordingNode*              m_pParent   {nullptr};
};

namespace Media {

class RecordingModelPrivate final : public QObject
{
   Q_OBJECT
public:
    explicit RecordingModelPrivate(RecordingModel* parent);
    ~RecordingModelPrivate();

    // Attributes
    std::vector<RecordingNode*>    m_lCategories             ;
    RecordingNode*                 m_pText          {nullptr};
    RecordingNode*                 m_pAudioVideo    {nullptr};
    LocalTextRecordingCollection*  m_pTextRecordingCollection;
    int                            m_UnreadCount             ;

    // Helpers
    void initCategories();
    void forwardInsertion(TextRecording* r, ContactMethod* cm, Media::Direction direction);
    void updateUnreadCount(const int count);
    void emitChangedProxy();

    Q_DISABLE_COPY(RecordingModelPrivate)
    Q_DECLARE_PUBLIC(RecordingModel)

private:
    RecordingModel* q_ptr;
};

} // namespace Media::

RecordingNode::RecordingNode(RecordingNode::Type type) : m_Type(type)
{
}

RecordingNode::~RecordingNode()
{
    foreach(RecordingNode* c, m_lChildren)
        delete c;
}

Media::RecordingModelPrivate::RecordingModelPrivate(RecordingModel* parent) : q_ptr(parent),m_pText(nullptr),
m_pAudioVideo(nullptr)
{

}

Media::RecordingModelPrivate::~RecordingModelPrivate()
{
    if (m_pTextRecordingCollection)
        delete m_pTextRecordingCollection;

    if (m_pText)
        delete m_pText;

    if (m_pAudioVideo)
        delete m_pAudioVideo;
}

void Media::RecordingModelPrivate::forwardInsertion(TextRecording* r, ContactMethod* cm, Media::Media::Direction direction)
{
    Q_UNUSED(direction)
    emit q_ptr->newTextMessage(r, cm);

}

void Media::RecordingModelPrivate::updateUnreadCount(const int count)
{
    m_UnreadCount += count;
    if (m_UnreadCount <= 0) {
        m_UnreadCount = 0;
    }
    emit q_ptr->unreadMessagesCountChanged(m_UnreadCount);
}

Media::RecordingModel::~RecordingModel()
{
    delete d_ptr;
}

Media::RecordingModel::RecordingModel(QObject* parent) : QAbstractItemModel(parent), CollectionManagerInterface<Recording>(this),
d_ptr(new RecordingModelPrivate(this))
{
    setObjectName("RecordingModel");

    d_ptr->m_pTextRecordingCollection = addCollection<LocalTextRecordingCollection>();

    d_ptr->m_pTextRecordingCollection->listId([](const QList<CollectionInterface::Element>& e) {
        //TODO
        Q_UNUSED(e);
    });
}

Media::RecordingModel& Media::RecordingModel::instance()
{
    static auto instance = new RecordingModel(QCoreApplication::instance());
    return *instance;
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

    const auto modelItem = static_cast<RecordingNode*>(index.internalPointer());

    if (modelItem->m_Type == RecordingNode::Type::TOP_LEVEL) {
        if (index.column() == 0 && role == Qt::DisplayRole)
            return modelItem->m_CatName;
        if (index.column() == 1 && role == Qt::DisplayRole)
            return static_cast<int>(modelItem->m_lChildren.size());

        return {};
    }

    const TextRecording* tRec  = nullptr;
    const AVRecording*   avRec = nullptr;

    if (modelItem->m_pRec->type() == Recording::Type::TEXT)
        tRec = static_cast<const TextRecording*>(modelItem->m_pRec);
    else if (modelItem->m_pRec->type() == Recording::Type::AUDIO_VIDEO)
        avRec = static_cast<const AVRecording*>(modelItem->m_pRec);

    switch(index.column()) {
        case 0:
            switch (role) {
                case Qt::DisplayRole:
                    if (tRec && tRec->peers().size())
                        return tRec->peers().first()->primaryName();
                    if (avRec && avRec->call())
                        return avRec->call()->peerContactMethod()->primaryName();
            }
            break;
        case 1:
            switch (role) {
                case Qt::DisplayRole:
                    if (tRec && tRec->peers().size())
                        return tRec->size();
                    if (avRec && avRec->call())
                        return tr("N/A");
            }
            break;
    }

    return QVariant();
}

///Get header data
QVariant Media::RecordingModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(section)
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
            case 0:
                return QVariant(tr("Recordings"));
            case 1:
                return QVariant(tr("Count"));
        }
    }
    return QVariant();
}

///Get the number of child of "parent"
int Media::RecordingModel::rowCount( const QModelIndex& parent ) const
{
    if (!parent.isValid())
        return d_ptr->m_lCategories.size();

    // Only the first column has a tree
    if (parent.column())
        return 0;

    const auto modelItem = static_cast<RecordingNode*>(parent.internalPointer());

    return modelItem->m_lChildren.size();
}

Qt::ItemFlags Media::RecordingModel::flags( const QModelIndex& index ) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

int Media::RecordingModel::columnCount ( const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    if (!parent.isValid())
        return 2;

    const auto modelItem = static_cast<RecordingNode*>(parent.internalPointer());

    return modelItem->m_lChildren.size() ? 2 : 0;
}

///Get the bookmark parent
QModelIndex Media::RecordingModel::parent( const QModelIndex& idx) const
{
    if (!idx.isValid())
        return {};

    const auto modelItem = static_cast<RecordingNode*>(idx.internalPointer());

    if (modelItem->m_Type != RecordingNode::Type::SESSION)
        return {};

    if (auto item = static_cast<RecordingNode*>(modelItem)->m_pParent)
        return createIndex(item->m_Index, 0, item);

    return {};
} //parent

///Get the index
QModelIndex Media::RecordingModel::index(int row, int column, const QModelIndex& parent) const
{
    const int count = static_cast<int>(d_ptr->m_lCategories.size());

    if (column > 1 || row < 0 || ((!parent.isValid()) && row >= count))
        return {};

    if (!parent.isValid())
        return createIndex(row, column, d_ptr->m_lCategories[row]);

    const auto modelItem = static_cast<const RecordingNode*>(parent.internalPointer());

    if (row >= static_cast<int>(modelItem->m_lChildren.size()))
        return {};

    return createIndex(row, column, modelItem->m_lChildren[row]);
}

void Media::RecordingModelPrivate::initCategories()
{
    if (m_lCategories.size())
        return;

    //Create some categories
    q_ptr->beginInsertRows({}, 0 , 1);
    m_pText = new RecordingNode(RecordingNode::Type::TOP_LEVEL);
    m_pText->m_CatName = tr("Text messages");
    m_pText->m_Index   = 0;
    m_lCategories.push_back(m_pText);

    m_pAudioVideo = new RecordingNode(RecordingNode::Type::TOP_LEVEL);
    m_pAudioVideo->m_CatName = tr("Audio/Video");
    m_pAudioVideo->m_Index   = 1;
    m_lCategories.push_back(m_pAudioVideo);
    q_ptr->endInsertRows();
}

bool Media::RecordingModel::addItemCallback(const Recording* item)
{
    Q_UNUSED(item)

    d_ptr->initCategories();

    //Categorize by general media group
    RecordingNode* parent = nullptr;

    if (item->type() == Recording::Type::TEXT)
        parent = d_ptr->m_pText;
    else if (item->type() == Recording::Type::AUDIO_VIDEO)
        parent = d_ptr->m_pAudioVideo;

    if (!parent)
        return false;

    //Insert the item]
    const int idx = parent->m_lChildren.size();
    beginInsertRows(index(parent->m_Index,0), idx, idx);

    RecordingNode* n = new RecordingNode       ( RecordingNode::Type::SESSION );
    n->m_pRec        = const_cast<Recording*>  ( item );
    n->m_Index       = idx;
    n->m_pParent     = parent;
    parent->m_lChildren.push_back(n);

    endInsertRows();

    if (item->type() == Recording::Type::TEXT) {
        const TextRecording* r = static_cast<const TextRecording*>(item);
        connect(r, &TextRecording::unreadCountChange, d_ptr, &RecordingModelPrivate::updateUnreadCount);
        connect(r, &TextRecording::messageInserted  , [n, this, parent](
            const QMap<QString,QString>&, ContactMethod* cm, Media::Media::Direction d
        ){
            const auto par = index(parent->m_Index, 0);

            emit dataChanged(
                index(n->m_Index, 0, par),
                index(n->m_Index, 1, par)
            );

            if (n->m_pRec->type() == Recording::Type::TEXT)
                d_ptr->forwardInsertion(
                    static_cast<TextRecording*>(n->m_pRec), cm, d
                );
        });
    }

    return true;
}

bool Media::RecordingModel::removeItemCallback(const Recording* item)
{
    Q_UNUSED(item)
    return false;
}

bool Media::RecordingModel::clearAllCollections() const
{
    foreach (CollectionInterface* backend, collections(CollectionInterface::SupportedFeatures::CLEAR)) {
        backend->clear();
    }
    return true;
}

///Deletes all recordings (which are possible to delete) and clears model
void Media::RecordingModel::clear()
{
    beginResetModel();
    clearAllCollections();
    endResetModel();
}

void Media::RecordingModel::collectionAddedCallback(CollectionInterface* backend)
{
    Q_UNUSED(backend)
}

///Set where the call recordings will be saved
void Media::RecordingModel::setRecordPath(const QString& path)
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    configurationManager.setRecordPath(path);
}

///Return the path where recordings are going to be saved
QString Media::RecordingModel::recordPath() const
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    return configurationManager.getRecordPath();
}

///are all calls recorded by default
bool Media::RecordingModel::isAlwaysRecording() const
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    return configurationManager.getIsAlwaysRecording();
}

///Set if all calls needs to be recorded
void Media::RecordingModel::setAlwaysRecording(bool record)
{
    ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
    configurationManager.setIsAlwaysRecording   ( record );
}

int  Media::RecordingModel::unreadCount() const
{
    return d_ptr->m_UnreadCount;
}

///Create or load the recording associated with the ContactMethod cm
Media::TextRecording* Media::RecordingModel::createTextRecording(const ContactMethod* cm)
{
    TextRecording* r = d_ptr->m_pTextRecordingCollection->createFor(cm);

    return r;
}

#include <recordingmodel.moc>
