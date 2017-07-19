/****************************************************************************
 *   Copyright (C) 2012-2017 Savoir-faire Linux                          *
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
#include "codecmodel.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QItemSelectionModel>
#include <QMimeData>

//DRing
#include <account_const.h>

//Ring
#include "account.h"
#include "dbus/configurationmanager.h"
#include "mime.h"
#include "callmodel.h"
#include <private/matrixutils.h>

typedef void (CodecModelPrivate::*CodecModelFct)();

class CodecModelPrivate final : public QObject
{
   Q_OBJECT
public:
   CodecModelPrivate(CodecModel* parent);
   ///@struct CodecData store audio/video codec information
   struct CodecData {
      int              id         ;
      QString          name       ;
      QString          bitrate    ;
      QString          min_bitrate;
      QString          max_bitrate;
      QString          samplerate ;
      QString          type       ;
      QString          quality    ;
      QString          min_quality;
      QString          max_quality;
      QString          auto_quality_enabled;
   };

   //Attributes
   QList<CodecData*>      m_lCodecs        ;
   QMap<int,bool>         m_lEnabledCodecs ;
   Account*               m_pAccount       ;
   QSortFilterProxyModel* m_pAudioProxy    ;
   QSortFilterProxyModel* m_pVideoProxy    ;
   QStringList            m_lMimes         ;
   QItemSelectionModel*   m_pSelectionModel;
   CodecModel::EditState  m_EditState      ;
   static Matrix2D<CodecModel::EditState, CodecModel::EditAction,CodecModelFct> m_mStateMachine;

   //Callbacks
   QModelIndex add         (                        );
   void        remove      ( const QModelIndex& idx );
   void        clear       (                        );
   void        reload      (                        );
   void        save        (                        );
   void        nothing     (                        );
   void        modify      (                        );

   //Helpers
   bool findCodec(int id);
   QModelIndex getIndexofCodecByID(int id);
   inline void performAction(const CodecModel::EditAction action);

private:
   CodecModel* q_ptr;
};

#define CMP &CodecModelPrivate
Matrix2D<CodecModel::EditState, CodecModel::EditAction,CodecModelFct> CodecModelPrivate::m_mStateMachine ={{
   /*                     SAVE         MODIFY        RELOAD        CLEAR      */
   /* LOADING   */ {{ CMP::nothing, CMP::nothing, CMP::reload , CMP::nothing  }},
   /* READY     */ {{ CMP::nothing, CMP::modify , CMP::reload , CMP::clear    }},
   /* MODIFIED  */ {{ CMP::save   , CMP::nothing, CMP::reload , CMP::clear    }},
   /* OUTDATED  */ {{ CMP::save   , CMP::nothing, CMP::reload , CMP::clear    }},
   /* RELOADING */ {{ CMP::nothing, CMP::nothing, CMP::nothing, CMP::nothing  }},
}};
#undef CMP

CodecModelPrivate::CodecModelPrivate(CodecModel* parent) : q_ptr(parent),
m_pAudioProxy(nullptr),m_pVideoProxy(nullptr),m_pSelectionModel(nullptr)
{

}

///Constructor
CodecModel::CodecModel(Account* account) :
QAbstractListModel(account?(QObject*)account:(QObject*)QCoreApplication::instance()), d_ptr(new CodecModelPrivate(this))
{
   Q_ASSERT(account);
   d_ptr->m_EditState = CodecModel::EditState::LOADING;
   d_ptr->m_pAccount = account;

   // New accounts don't have ID yet, avoid a warning
   if (account && !account->isNew())
      setObjectName("CodecModel: "+(account?account->id():"Unknown"));

   d_ptr->m_lMimes << RingMimes::AUDIO_CODEC << RingMimes::VIDEO_CODEC;
   this << EditAction::RELOAD;
   d_ptr->m_EditState = CodecModel::EditState::READY;
}

CodecModel::~CodecModel()
{
   while (d_ptr->m_lCodecs.size()) {
      CodecModelPrivate::CodecData* c = d_ptr->m_lCodecs[0];
      d_ptr->m_lCodecs.removeAt(0);
      delete c;
   }
}

QHash<int,QByteArray> CodecModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles.insert(CodecModel::Role::ID         ,QByteArray("id"));
      roles.insert(CodecModel::Role::NAME       ,QByteArray("name"));
      roles.insert(CodecModel::Role::BITRATE    ,QByteArray("bitrate"));
      roles.insert(CodecModel::Role::MIN_BITRATE,QByteArray("min_bitrate"));
      roles.insert(CodecModel::Role::MAX_BITRATE,QByteArray("max_bitrate"));
      roles.insert(CodecModel::Role::SAMPLERATE ,QByteArray("samplerate"));
      roles.insert(CodecModel::Role::TYPE       ,QByteArray("type"));
      roles.insert(CodecModel::Role::QUALITY    ,QByteArray("quality"));
      roles.insert(CodecModel::Role::MIN_QUALITY,QByteArray("min_quality"));
      roles.insert(CodecModel::Role::MAX_QUALITY,QByteArray("max_quality"));
      roles.insert(CodecModel::Role::AUTO_QUALITY_ENABLED,QByteArray("autoQualityEnabled"));
   }
   return roles;
}

QItemSelectionModel* CodecModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel)
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<CodecModel*>(this));
   return d_ptr->m_pSelectionModel;
}

///Model data
QVariant CodecModel::data(const QModelIndex& idx, int role) const
{
    if (idx.column() != 0)
        return QVariant();

    switch (role) {
        case Qt::DisplayRole:
            return QVariant(d_ptr->m_lCodecs[idx.row()]->name);
        case Qt::CheckStateRole:
            return QVariant(d_ptr->m_lEnabledCodecs[d_ptr->m_lCodecs[idx.row()]->id] ? Qt::Checked : Qt::Unchecked);
        case CodecModel::Role::NAME:
            return d_ptr->m_lCodecs[idx.row()]->name;
        case CodecModel::Role::BITRATE:
            return d_ptr->m_lCodecs[idx.row()]->bitrate;
        case CodecModel::Role::MIN_BITRATE:
            return d_ptr->m_lCodecs[idx.row()]->min_bitrate;
        case CodecModel::Role::MAX_BITRATE:
            return d_ptr->m_lCodecs[idx.row()]->max_bitrate;
        case CodecModel::Role::SAMPLERATE:
            return d_ptr->m_lCodecs[idx.row()]->samplerate;
        case CodecModel::Role::ID:
            return d_ptr->m_lCodecs[idx.row()]->id;
        case CodecModel::Role::TYPE:
            return d_ptr->m_lCodecs[idx.row()]->type;
        case CodecModel::Role::QUALITY:
            return d_ptr->m_lCodecs[idx.row()]->quality;
        case CodecModel::Role::MIN_QUALITY:
            return d_ptr->m_lCodecs[idx.row()]->min_quality;
        case CodecModel::Role::MAX_QUALITY:
            return d_ptr->m_lCodecs[idx.row()]->max_quality;
        case CodecModel::Role::AUTO_QUALITY_ENABLED:
            return d_ptr->m_lCodecs[idx.row()]->auto_quality_enabled;
        default:
            return QVariant();
    }
    return QVariant();
}

///Number of audio codecs
int CodecModel::rowCount(const QModelIndex& par) const
{
   return par.isValid() ? 0 : d_ptr->m_lCodecs.size();
}

///Model flags
Qt::ItemFlags CodecModel::flags(const QModelIndex& idx) const
{
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable
                                            | Qt::ItemIsEnabled
                                            | Qt::ItemIsSelectable
                                            | Qt::ItemIsDragEnabled
                                            | Qt::ItemIsDropEnabled;
   return QAbstractItemModel::flags(idx);
}

///Set audio codec data
bool CodecModel::setData( const QModelIndex& idx, const QVariant &value, int role)
{
    if (idx.column() != 0)
        return false;

    switch (role) {
        case CodecModel::NAME :
            d_ptr->m_lCodecs[idx.row()]->name = value.toString();
            break;
        case CodecModel::BITRATE :
            d_ptr->m_lCodecs[idx.row()]->bitrate = value.toString();
            break;
        case CodecModel::MIN_BITRATE :
            d_ptr->m_lCodecs[idx.row()]->min_bitrate = value.toString();
            break;
        case CodecModel::MAX_BITRATE :
            d_ptr->m_lCodecs[idx.row()]->max_bitrate = value.toString();
            break;
        case Qt::CheckStateRole :
            d_ptr->m_lEnabledCodecs[d_ptr->m_lCodecs[idx.row()]->id] = value.toBool();
            break;
        case CodecModel::SAMPLERATE :
            d_ptr->m_lCodecs[idx.row()]->samplerate = value.toString();
            break;
        case CodecModel::ID :
            d_ptr->m_lCodecs[idx.row()]->id = value.toInt();
            break;
        case CodecModel::TYPE :
            d_ptr->m_lCodecs[idx.row()]->type = value.toString();
            break;
        case CodecModel::QUALITY :
            d_ptr->m_lCodecs[idx.row()]->quality = value.toString();
            break;
        case CodecModel::MIN_QUALITY :
            d_ptr->m_lCodecs[idx.row()]->min_quality = value.toString();
            break;
        case CodecModel::MAX_QUALITY :
            d_ptr->m_lCodecs[idx.row()]->max_quality = value.toString();
            break;
        case CodecModel::AUTO_QUALITY_ENABLED :
            d_ptr->m_lCodecs[idx.row()]->auto_quality_enabled = value.toString();
            break;
        default:
            return false;
    }

    //if we did not return yet, then we modified the codec
    emit dataChanged(idx, idx);
    this << EditAction::MODIFY;
    return true;
}

///Add a new audio codec
QModelIndex CodecModelPrivate::add()
{
   q_ptr->beginInsertRows(QModelIndex(), m_lCodecs.size()-1, m_lCodecs.size()-1);
   m_lCodecs << new CodecModelPrivate::CodecData;
   q_ptr->endInsertRows();
   emit q_ptr->dataChanged(q_ptr->index(m_lCodecs.size()-1,0), q_ptr->index(m_lCodecs.size()-1,0));
   q_ptr << CodecModel::EditAction::MODIFY;
   return q_ptr->index(m_lCodecs.size()-1,0);
}

///Remove audio codec at 'idx'
void CodecModelPrivate::remove(const QModelIndex& idx)
{
   if (idx.isValid()) {
      q_ptr->beginRemoveRows(QModelIndex(), idx.row(), idx.row());
      CodecModelPrivate::CodecData* d = m_lCodecs[idx.row()];
      m_lCodecs.removeAt(idx.row());
      delete d;
      q_ptr->endRemoveRows();
      emit q_ptr->dataChanged(idx, q_ptr->index(m_lCodecs.size()-1,0));
      q_ptr << CodecModel::EditAction::MODIFY;
   }
   else {
      qDebug() << "Failed to remove an invalid audio codec";
   }
}

///Remove everything
void CodecModelPrivate::clear()
{
   while(m_lCodecs.size()) {
      CodecModelPrivate::CodecData* d = m_lCodecs[0];
      m_lCodecs.removeAt(0);
      delete d;
   }
   m_lCodecs.clear();
   m_lEnabledCodecs.clear();
   m_EditState = CodecModel::EditState::READY;
}

///Reload the codeclist
void CodecModelPrivate::reload()
{
   m_EditState = CodecModel::EditState::RELOADING;

   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   QVector<uint> codecIdList = configurationManager.getCodecList();

   QVector<uint> activeCodecList = m_pAccount->isNew() ? codecIdList :
      configurationManager.getActiveCodecList(m_pAccount->id());

   QStringList tmpNameList;

   //TODO: the following method cannot update the order of the codecs if it
   //      changes in the daemon after it has been initially loaded in the client

   // load the active codecs first to get the correct order
   foreach (const int aCodec, activeCodecList) {

      const QMap<QString,QString> codec = configurationManager.getCodecDetails(
         m_pAccount->isNew()? QString() : m_pAccount->id(), aCodec
      );

      if (!findCodec(aCodec)) {
         const QModelIndex& idx = add();
         q_ptr->setData(idx,QString::number(aCodec), CodecModel::Role::ID);
      }

      // update the codec

      const auto& idx = getIndexofCodecByID(aCodec);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::NAME        ] ,CodecModel::Role::NAME       );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE ] ,CodecModel::Role::SAMPLERATE );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::BITRATE     ] ,CodecModel::Role::BITRATE    );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MIN_BITRATE ] ,CodecModel::Role::MIN_BITRATE);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MAX_BITRATE ] ,CodecModel::Role::MAX_BITRATE);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::TYPE        ] ,CodecModel::Role::TYPE       );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::QUALITY     ] ,CodecModel::Role::QUALITY    );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MIN_QUALITY ] ,CodecModel::Role::MIN_QUALITY);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MAX_QUALITY ] ,CodecModel::Role::MAX_QUALITY);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::AUTO_QUALITY_ENABLED] ,CodecModel::Role::AUTO_QUALITY_ENABLED);
      q_ptr->setData(idx, Qt::Checked ,Qt::CheckStateRole);

      // remove from list of all codecs, since we have already updated it
      if (codecIdList.indexOf(aCodec)!=-1)
         codecIdList.remove(codecIdList.indexOf(aCodec));
   }

   // now add add/update remaining (inactive) codecs
   foreach (const int aCodec, codecIdList) {

      const QMap<QString,QString> codec = configurationManager.getCodecDetails(
         m_pAccount->isNew()? QString() : m_pAccount->id(), aCodec
      );

      if (!findCodec(aCodec)) {
         const QModelIndex& idx = add();
         q_ptr->setData(idx,QString::number(aCodec), CodecModel::Role::ID);
      }

      // update the codec
      const auto& idx = getIndexofCodecByID(aCodec);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::NAME        ] ,CodecModel::Role::NAME       );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE ] ,CodecModel::Role::SAMPLERATE );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::BITRATE     ] ,CodecModel::Role::BITRATE    );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MIN_BITRATE ] ,CodecModel::Role::MIN_BITRATE);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MAX_BITRATE ] ,CodecModel::Role::MAX_BITRATE);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::TYPE        ] ,CodecModel::Role::TYPE       );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::QUALITY     ] ,CodecModel::Role::QUALITY    );
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MIN_QUALITY ] ,CodecModel::Role::MIN_QUALITY);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::MAX_QUALITY ] ,CodecModel::Role::MAX_QUALITY);
      q_ptr->setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::AUTO_QUALITY_ENABLED] ,CodecModel::Role::AUTO_QUALITY_ENABLED);
      q_ptr->setData(idx, Qt::Unchecked ,Qt::CheckStateRole);
   }

   m_EditState = CodecModel::EditState::READY;
}

///Save details
void CodecModelPrivate::save()
{
   //TODO there is a race condition, the account has to be saved first

   //Update active codec list
   VectorUInt _codecList;
   for (int i = 0; i < q_ptr->rowCount();i++) {
      const QModelIndex& idx = q_ptr->index(i,0);
      if (q_ptr->data(idx,Qt::CheckStateRole) == Qt::Checked) {
         _codecList << q_ptr->data(idx,CodecModel::Role::ID).toInt();
      }
   }

   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   configurationManager.setActiveCodecList(m_pAccount->id(), _codecList);

   //Update codec details
   for (int i=0; i < q_ptr->rowCount();i++) {
      const QModelIndex& idx = q_ptr->index(i,0);
      MapStringString codecDetails;
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::NAME        ] = q_ptr->data(idx,CodecModel::Role::NAME).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE ] = q_ptr->data(idx,CodecModel::Role::SAMPLERATE).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::BITRATE     ] = q_ptr->data(idx,CodecModel::Role::BITRATE).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::MIN_BITRATE ] = q_ptr->data(idx,CodecModel::Role::MIN_BITRATE).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::MAX_BITRATE ] = q_ptr->data(idx,CodecModel::Role::MAX_BITRATE).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::TYPE        ] = q_ptr->data(idx,CodecModel::Role::TYPE).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::QUALITY     ] = q_ptr->data(idx,CodecModel::Role::QUALITY).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::MIN_QUALITY ] = q_ptr->data(idx,CodecModel::Role::MIN_QUALITY).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::MAX_QUALITY ] = q_ptr->data(idx,CodecModel::Role::MAX_QUALITY).toString();
      codecDetails[ DRing::Account::ConfProperties::CodecInfo::AUTO_QUALITY_ENABLED] = q_ptr->data(idx,CodecModel::Role::AUTO_QUALITY_ENABLED).toString();

      qDebug() << "setting codec details for " << q_ptr->data(idx,CodecModel::Role::NAME).toString();

      configurationManager.setCodecDetails(m_pAccount->id(), q_ptr->data(idx,CodecModel::Role::ID).toUInt(), codecDetails);
   }
   m_EditState = CodecModel::EditState::READY;
}

void CodecModelPrivate::nothing()
{
   //nothing
}

void CodecModelPrivate::modify()
{
   m_EditState = CodecModel::EditState::MODIFIED;
   m_pAccount << Account::EditAction::MODIFY;
}

void CodecModelPrivate::performAction(const CodecModel::EditAction action)
{
   (this->*(m_mStateMachine[m_EditState][action]))();//FIXME don't use integer cast
}

CodecModel* CodecModel::operator<<(CodecModel::EditAction& action)
{
   performAction(action);
   return this;
}

CodecModel* operator<<(CodecModel* a, CodecModel::EditAction action)
{
   return (!a)?nullptr : (*a) << action;
}

///Change the current edition state
bool CodecModel::performAction(const CodecModel::EditAction action)
{
   CodecModel::EditState curState = d_ptr->m_EditState;
   d_ptr->performAction(action);
   return curState != d_ptr->m_EditState;
}

///Move a codec up in the priority list (using the selectionModel)
bool CodecModel::moveUp()
{
   if (d_ptr->m_pSelectionModel) {
      const QModelIndex& idx = d_ptr->m_pSelectionModel->currentIndex();
      if (dropMimeData(mimeData({idx}), Qt::MoveAction, idx.row()-1, idx.column(),idx.parent())) {
         d_ptr->m_pSelectionModel->setCurrentIndex(index(idx.row()-1,idx.column()), QItemSelectionModel::ClearAndSelect);
         return true;
      }
   }
   return false;
}

///Move a codec down in the priority list (using the selectionModel)
bool CodecModel::moveDown()
{
   if (d_ptr->m_pSelectionModel) {
      const QModelIndex& idx = d_ptr->m_pSelectionModel->currentIndex();
      if (dropMimeData(mimeData({idx}), Qt::MoveAction, idx.row()+1, idx.column(),idx.parent())) {
         d_ptr->m_pSelectionModel->setCurrentIndex(index(idx.row()+1,idx.column()), QItemSelectionModel::ClearAndSelect);
         return true;
      }
   }
   return false;
}

///Check is a codec is already in the list
bool CodecModelPrivate::findCodec(int id)
{
   foreach(const CodecData* data, m_lCodecs) {
      if (data->id == id)
         return true;
   }
   return false;
}

///Return valid payload types
int CodecModel::acceptedPayloadTypes() const
{
   return CallModel::DropPayloadType::AUDIO_CODEC | CallModel::DropPayloadType::VIDEO_CODEC;
}


QSortFilterProxyModel* CodecModel::videoCodecs() const
{
   if (!d_ptr->m_pVideoProxy) {
      d_ptr->m_pVideoProxy = new QSortFilterProxyModel(const_cast<CodecModel*>(this));
      d_ptr->m_pVideoProxy->setSourceModel(const_cast<CodecModel*>(this));
      d_ptr->m_pVideoProxy->setFilterRole(CodecModel::Role::TYPE);
      d_ptr->m_pVideoProxy->setFilterFixedString("VIDEO");
   }
   return d_ptr->m_pVideoProxy;
}


QSortFilterProxyModel* CodecModel::audioCodecs() const
{
   if (!d_ptr->m_pAudioProxy) {
      d_ptr->m_pAudioProxy = new QSortFilterProxyModel(const_cast<CodecModel*>(this));
      d_ptr->m_pAudioProxy->setSourceModel(const_cast<CodecModel*>(this));
      d_ptr->m_pAudioProxy->setFilterRole(CodecModel::Role::TYPE);
      d_ptr->m_pAudioProxy->setFilterFixedString("AUDIO");
   }
   return d_ptr->m_pAudioProxy;
}

// ======== DnD Api =========

QModelIndex CodecModelPrivate::getIndexofCodecByID(int id)
{
   for (int i=0; i < q_ptr->rowCount();i++) {
      const QModelIndex& idx = q_ptr->index(i,0);
      if (q_ptr->data(idx, CodecModel::Role::ID) == id)
         return idx;
   }
   return QModelIndex();
}

QStringList CodecModel::mimeTypes() const
{
   return d_ptr->m_lMimes;
}

bool CodecModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
{
   Q_UNUSED(action)
   if(parent.isValid() || column > 0) {
      qDebug() << "column invalid";
      return false;
   }

   if (data->hasFormat(RingMimes::AUDIO_CODEC) || data->hasFormat(RingMimes::VIDEO_CODEC)) {
      int destinationRow = -1;

      if(row < 0) {
         //drop on top
         destinationRow = d_ptr->m_lCodecs.size() - 1;
      }
      else if(row >= d_ptr->m_lCodecs.size()) {
         destinationRow = 0;
      }
      else if(data->hasFormat(RingMimes::VIDEO_CODEC) && row >= rowCount()) {
         destinationRow = 0;
      }
      else {
         destinationRow = row;
      }

      const int codecId = data->data(data->hasFormat(RingMimes::AUDIO_CODEC)
                                                    ? RingMimes::AUDIO_CODEC
                                                    : RingMimes::VIDEO_CODEC).toInt();

      const QModelIndex codecIdx = d_ptr->getIndexofCodecByID(codecId);

#ifdef Q_OS_WIN
      /* To understand why qtDestinationRow is needed
      http://doc.qt.io/qt-5/qabstractitemmodel.html#beginMoveRows */
      auto qtDestinationRow = destinationRow > codecIdx.row() ? destinationRow+1 : destinationRow;
      beginMoveRows(parent, codecIdx.row(), codecIdx.row(), parent, qtDestinationRow);
      d_ptr->m_lCodecs.move(codecIdx.row(), destinationRow);
      endMoveRows();
#else
      beginRemoveRows(QModelIndex(), codecIdx.row(), codecIdx.row());
      CodecModelPrivate::CodecData* codecInfo = d_ptr->m_lCodecs[codecIdx.row()];
      d_ptr->m_lCodecs.removeAt(codecIdx.row());
      endRemoveRows();

      beginInsertRows(QModelIndex(), destinationRow, destinationRow);
      d_ptr->m_lCodecs.insert(destinationRow,codecInfo);
      endInsertRows();
#endif

      this << EditAction::MODIFY;

      return true;
   }

   return false;
}

QMimeData* CodecModel::mimeData(const QModelIndexList& indexes) const
{
   QMimeData* mMimeData = new QMimeData();

   for (const QModelIndex& index : indexes) {
      if (index.isValid()) {
         qDebug() << "setting mime data for row: " << index.row();

         const QByteArray mime = (index.data(CodecModel::Role::TYPE).toString() == "AUDIO")
            ? RingMimes::AUDIO_CODEC
            : RingMimes::VIDEO_CODEC;

         mMimeData->setData(mime, index.data(CodecModel::Role::ID).toByteArray());
      }
   }

   return mMimeData;
}

Qt::DropActions CodecModel::supportedDragActions() const
{
   return Qt::MoveAction | Qt::TargetMoveAction;
}

Qt::DropActions CodecModel::supportedDropActions() const
{
   return Qt::MoveAction | Qt::TargetMoveAction;
}

#include <codecmodel.moc>
