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
#include "codecmodel.h"

//Qt
#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtCore/QSortFilterProxyModel>
#include <QMimeData>

//DRing
#include <account_const.h>

//Ring
#include "account.h"
#include "dbus/configurationmanager.h"
#include "mime.h"
#include "callmodel.h"

class CodecModelPrivate : public QObject
{
   Q_OBJECT
public:
   CodecModelPrivate(CodecModel* parent);
   ///@struct CodecData store audio codec information
   struct CodecData {
      int              id        ;
      QString          name      ;
      QString          bitrate   ;
      QString          samplerate;
      QString          type      ;
   };

   //Attributes
   QList<CodecData*>      m_lCodecs  ;
   QMap<int,bool>         m_lEnabledCodecs;
   Account*               m_pAccount      ;
   QSortFilterProxyModel* m_pAudioProxy   ;
   QSortFilterProxyModel* m_pVideoProxy   ;
   QStringList            m_lMimes        ;

   //Helpers
   bool findCodec(int id);

private:
   CodecModel* q_ptr;
};

CodecModelPrivate::CodecModelPrivate(CodecModel* parent) : q_ptr(parent),
m_pAudioProxy(nullptr),m_pVideoProxy(nullptr)
{

}

///Constructor
CodecModel::CodecModel(Account* account) :
QAbstractListModel(account?(QObject*)account:(QObject*)QCoreApplication::instance()), d_ptr(new CodecModelPrivate(this))
{
   d_ptr->m_pAccount = account;
   setObjectName("CodecModel: "+(account?account->id():"Unknown"));
   d_ptr->m_lMimes << RingMimes::AUDIO_CODEC << RingMimes::VIDEO_CODEC;
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
      roles.insert(CodecModel::Role::ID        ,QByteArray("id"));
      roles.insert(CodecModel::Role::NAME      ,QByteArray("name"));
      roles.insert(CodecModel::Role::BITRATE   ,QByteArray("bitrate"));
      roles.insert(CodecModel::Role::SAMPLERATE,QByteArray("samplerate"));
      roles.insert(CodecModel::Role::TYPE      ,QByteArray("type"));
   }
   return roles;
}

///Model data
QVariant CodecModel::data(const QModelIndex& idx, int role) const {
   if(idx.column() == 0      && role == Qt::DisplayRole                   ) {
      return QVariant(d_ptr->m_lCodecs[idx.row()]->name);
   }
   else if(idx.column() == 0 && role == Qt::CheckStateRole                ) {
      return QVariant(d_ptr->m_lEnabledCodecs[d_ptr->m_lCodecs[idx.row()]->id] ? Qt::Checked : Qt::Unchecked);
   }
   else if (idx.column() == 0 && role == CodecModel::Role::NAME       ) {
      return d_ptr->m_lCodecs[idx.row()]->name;
   }
   else if (idx.column() == 0 && role == CodecModel::Role::BITRATE    ) {
      return d_ptr->m_lCodecs[idx.row()]->bitrate;
   }
   else if (idx.column() == 0 && role == CodecModel::Role::SAMPLERATE ) {
      return d_ptr->m_lCodecs[idx.row()]->samplerate;
   }
   else if (idx.column() == 0 && role == CodecModel::Role::ID         ) {
      return d_ptr->m_lCodecs[idx.row()]->id;
   }
   else if (idx.column() == 0 && role == CodecModel::Role::TYPE         ) {
      return d_ptr->m_lCodecs[idx.row()]->type;
   }
   return QVariant();
}

///Number of audio codecs
int CodecModel::rowCount(const QModelIndex& par) const {
   Q_UNUSED(par)
   return d_ptr->m_lCodecs.size();
}

///Model flags
Qt::ItemFlags CodecModel::flags(const QModelIndex& idx) const {
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable
                                            | Qt::ItemIsEnabled
                                            | Qt::ItemIsSelectable
                                            | Qt::ItemIsDragEnabled
                                            | Qt::ItemIsDropEnabled;
   return QAbstractItemModel::flags(idx);
}

///Set audio codec data
bool CodecModel::setData( const QModelIndex& idx, const QVariant &value, int role) {
   if (idx.column() == 0 && role == CodecModel::NAME) {
      d_ptr->m_lCodecs[idx.row()]->name = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == CodecModel::BITRATE) {
      d_ptr->m_lCodecs[idx.row()]->bitrate = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if(idx.column() == 0 && role == Qt::CheckStateRole) {
      d_ptr->m_lEnabledCodecs[d_ptr->m_lCodecs[idx.row()]->id] = value.toBool();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == CodecModel::SAMPLERATE) {
      d_ptr->m_lCodecs[idx.row()]->samplerate = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == CodecModel::ID) {
      d_ptr->m_lCodecs[idx.row()]->id = value.toInt();
      emit dataChanged(idx, idx);
      return true;
   }
   else if (idx.column() == 0 && role == CodecModel::TYPE) {
      d_ptr->m_lCodecs[idx.row()]->type = value.toString();
      emit dataChanged(idx, idx);
      return true;
   }
   return false;
}

///Add a new audio codec
QModelIndex CodecModel::add() {
   d_ptr->m_lCodecs << new CodecModelPrivate::CodecData;
   emit dataChanged(index(d_ptr->m_lCodecs.size()-1,0), index(d_ptr->m_lCodecs.size()-1,0));
   return index(d_ptr->m_lCodecs.size()-1,0);
}

///Remove audio codec at 'idx'
void CodecModel::remove(const QModelIndex& idx) {
   if (idx.isValid()) {
      CodecModelPrivate::CodecData* d = d_ptr->m_lCodecs[idx.row()];
      d_ptr->m_lCodecs.removeAt(idx.row());
      delete d;
      emit dataChanged(idx, index(d_ptr->m_lCodecs.size()-1,0));
   }
   else {
      qDebug() << "Failed to remove an invalid audio codec";
   }
}

///Remove everything
void CodecModel::clear()
{
   while(d_ptr->m_lCodecs.size()) {
      CodecModelPrivate::CodecData* d = d_ptr->m_lCodecs[0];
      d_ptr->m_lCodecs.removeAt(0);
      delete d;
   }
   d_ptr->m_lCodecs.clear();
   d_ptr->m_lEnabledCodecs.clear();
}

///Reload the codeclist
void CodecModel::reload()
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   QVector<uint> codecIdList = configurationManager.getCodecList();
      QVector<uint> activeCodecList = configurationManager.getActiveCodecList(d_ptr->m_pAccount->id());
      QStringList tmpNameList;

  foreach (const int aCodec, activeCodecList) {
     if (!d_ptr->findCodec(aCodec)) {

        const QMap<QString,QString> codec = configurationManager.getCodecDetails(d_ptr->m_pAccount->id(),aCodec);

        QModelIndex idx = add();
        setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::NAME        ] ,CodecModel::Role::NAME       );
        setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE ] ,CodecModel::Role::SAMPLERATE );
        setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::BITRATE     ] ,CodecModel::Role::BITRATE    );
        setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::TYPE        ] ,CodecModel::Role::TYPE       );
        setData(idx,QString::number(aCodec)  ,CodecModel::Role::ID         );
        setData(idx, Qt::Checked ,Qt::CheckStateRole               );

        if (codecIdList.indexOf(aCodec)!=-1)
           codecIdList.remove(codecIdList.indexOf(aCodec));
     }
  }

   foreach (const int aCodec, codecIdList) {
      if (!d_ptr->findCodec(aCodec)) {
         const QMap<QString,QString> codec = configurationManager.getCodecDetails(d_ptr->m_pAccount->id(),aCodec);
         const QModelIndex& idx = add();
         setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::NAME        ] ,CodecModel::Role::NAME       );
         setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::SAMPLE_RATE ] ,CodecModel::Role::SAMPLERATE );
         setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::BITRATE     ] ,CodecModel::Role::BITRATE    );
         setData(idx,codec[ DRing::Account::ConfProperties::CodecInfo::TYPE        ] ,CodecModel::Role::TYPE       );
         setData(idx,QString::number(aCodec)  ,CodecModel::Role::ID         );
         setData(idx, Qt::Unchecked ,Qt::CheckStateRole);
      }
   }
}

///Save details
void CodecModel::save()
{
   VectorUInt _codecList;
   for (int i = 0; i < rowCount();i++) {
      const QModelIndex& idx = index(i,0);
      if (data(idx,Qt::CheckStateRole) == Qt::Checked) {
         _codecList << data(idx,CodecModel::Role::ID).toInt();
      }
   }

   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   configurationManager.setActiveCodecList(d_ptr->m_pAccount->id(), _codecList);
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

QModelIndex CodecModel::getIndexofCodecByID(int id)
{
    for (int i=0; i < rowCount();i++) {
       const QModelIndex& idx = index(i,0);
       if (data(idx, CodecModel::Role::ID) == id) {
           qDebug() << "FOUND";
          return idx;
       }
    }
    return QModelIndex();
}

// ======== DnD Api =========

///Return valid payload types
int CodecModel::acceptedPayloadTypes() const
{
   return CallModel::DropPayloadType::AUDIO_CODEC | CallModel::DropPayloadType::VIDEO_CODEC;
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
      else if(data->hasFormat(RingMimes::VIDEO_CODEC) && row >= d_ptr->m_pVideoProxy->rowCount()) {
         destinationRow = 0;
      }
      else {
         destinationRow = row;
      }

      const int codecId = data->data(data->hasFormat(RingMimes::AUDIO_CODEC)
                                                    ? RingMimes::AUDIO_CODEC
                                                    : RingMimes::VIDEO_CODEC).toInt();
      const QModelIndex codecIdx = getIndexofCodecByID(codecId);

      beginRemoveRows(QModelIndex(), codecIdx.row(), codecIdx.row());
      CodecModelPrivate::CodecData* codecInfo = d_ptr->m_lCodecs[codecIdx.row()];
      d_ptr->m_lCodecs.removeAt(codecIdx.row());
      endRemoveRows();

      beginInsertRows(QModelIndex(), destinationRow, destinationRow);
      d_ptr->m_lCodecs.insert(destinationRow,codecInfo);
      endInsertRows();

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
         if(index.data(CodecModel::Role::TYPE).toString() == "AUDIO")
            mMimeData->setData(RingMimes::AUDIO_CODEC , index.data(CodecModel::Role::ID).toByteArray());
         else
            mMimeData->setData(RingMimes::VIDEO_CODEC , index.data(CodecModel::Role::ID).toByteArray());

      }
   }
   return mMimeData;
}


#include <codecmodel.moc>
