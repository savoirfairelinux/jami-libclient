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
#include "ringtonemodel.h"

//Qt
#include <QtCore/QTimer>
#include <QtCore/QFileInfo>
#include <QtCore/QItemSelectionModel>

//Ring
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"
#include "account.h"
#include "ringtone.h"
#include <localringtonecollection.h>

class RingtoneModelPrivate : public QObject
{
   Q_OBJECT
public:

   RingtoneModelPrivate(RingtoneModel*);

   //Attributes
   QVector<Ringtone*>                   m_lRingtone       ;
   QTimer*                              m_pTimer          ;
   Ringtone*                            m_pCurrent        ;
   QHash<Account*,int>                  m_hCurrent        ;
   QHash<Account*,QItemSelectionModel*> m_hSelectionModels;

   //Helpers
   int currentIndex(Account* a) const;

private:
   RingtoneModel* q_ptr;

public Q_SLOTS:
   void slotStopTimer();

};

RingtoneModelPrivate::RingtoneModelPrivate(RingtoneModel* parent)
  : q_ptr(parent)
  , m_pTimer(nullptr)
  , m_pCurrent(nullptr)
{

}

RingtoneModel::RingtoneModel(QObject* parent)
  : QAbstractTableModel(parent)
  , CollectionManagerInterface<Ringtone>(this)
  , d_ptr(new RingtoneModelPrivate(this))
{
//    ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   addCollection<LocalRingtoneCollection>();
}

RingtoneModel* RingtoneModel::instance()
{
   static RingtoneModel* ins = new RingtoneModel(QCoreApplication::instance());
   return ins;
}

RingtoneModel::~RingtoneModel()
{
   while (d_ptr->m_lRingtone.size()) {
      Ringtone* ringtone = d_ptr->m_lRingtone[0];
      d_ptr->m_lRingtone.removeAt(0);
      delete ringtone;
   }
}

QHash<int,QByteArray> RingtoneModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[Role::IsPlaying ] = "IsPlaying";
      roles[Role::FullPath  ] = "FullPath";
   }
   return roles;
}

QVariant RingtoneModel::data( const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();
   const Ringtone* info = d_ptr->m_lRingtone[index.row()];
   switch (index.column()) {
      case 0:
         switch (role) {
            case Qt::DisplayRole:
               return info->name();
            case Role::IsPlaying:
               return info->isPlaying();
            case Role::FullPath:
               return info->path();
         };
         break;
      case 1:
         switch (role) {
            case Role::FullPath:
               return info->path();
         };
         break;
   };
   return QVariant();
}

int RingtoneModel::rowCount( const QModelIndex& parent ) const
{
   if (!parent.isValid())
      return d_ptr->m_lRingtone.size();
   return 0;
}

int RingtoneModel::columnCount( const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return 2; //Name, then an empty one for widgets
}

Qt::ItemFlags RingtoneModel::flags( const QModelIndex& index ) const
{
   if (index.isValid() && !index.parent().isValid())
      return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   return Qt::NoItemFlags;
}

///This is a read only model
bool RingtoneModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role )
   return false;
}

Ringtone* RingtoneModel::currentRingTone(Account* a) const
{
   if ((!a) || (!d_ptr->m_hSelectionModels[a]))
      return nullptr;

   const QModelIndex& idx = d_ptr->m_hSelectionModels[a]->currentIndex();

   return idx.isValid() ? d_ptr->m_lRingtone[idx.row()] : nullptr;
}

int RingtoneModelPrivate::currentIndex(Account* a) const
{
   const QString rt = a->ringtonePath();
   for (int i=0;i<m_lRingtone.size();i++) {
      Ringtone* info = m_lRingtone[i];
      if (info->path().path() == rt)
         return i;
   }
   return -1;
}

QItemSelectionModel* RingtoneModel::selectionModel(Account* a) const
{
   if (!d_ptr->m_hSelectionModels[a]) {
      d_ptr->m_hSelectionModels[a] = new QItemSelectionModel(const_cast<RingtoneModel*>(this));
      d_ptr->m_hSelectionModels[a]->setCurrentIndex(index(d_ptr->currentIndex(a),0), QItemSelectionModel::ClearAndSelect);

      connect(d_ptr->m_hSelectionModels[a],&QItemSelectionModel::currentChanged, [a,this](const QModelIndex& idx) {
         if (idx.isValid()) {
            a->setRingtonePath(d_ptr->m_lRingtone[idx.row()]->path().path());
         }
      });

   }

   return d_ptr->m_hSelectionModels[a];
}

void RingtoneModel::play(const QModelIndex& idx)
{
   if (idx.isValid()) {
      Ringtone* info = d_ptr->m_lRingtone[idx.row()];
      if (d_ptr->m_pCurrent && info == d_ptr->m_pCurrent) {
         d_ptr->slotStopTimer();
         return;
      }
      CallManagerInterface& callManager = DBus::CallManager::instance();
      Q_NOREPLY callManager.startRecordedFilePlayback(info->path().path());
      if (!d_ptr->m_pTimer) {
         d_ptr->m_pTimer = new QTimer(this);
         d_ptr->m_pTimer->setInterval(10000);
         connect(d_ptr->m_pTimer,SIGNAL(timeout()),d_ptr,SLOT(slotStopTimer()));
      }
      else if (d_ptr->m_pTimer->isActive()) {
         d_ptr->m_pTimer->stop();
      }
      d_ptr->m_pTimer->start();
      info->setIsPlaying(true);
      emit dataChanged(index(idx.row(),0),index(idx.row(),1));
      d_ptr->m_pCurrent = info;
   }
}

void RingtoneModelPrivate::slotStopTimer()
{
   if (m_pCurrent) {
      CallManagerInterface& callManager = DBus::CallManager::instance();
      callManager.stopRecordedFilePlayback(m_pCurrent->path().path());
      m_pCurrent->setIsPlaying(false);
      const QModelIndex& idx = q_ptr->index(m_lRingtone.indexOf(m_pCurrent),0);
      emit q_ptr->dataChanged(idx,q_ptr->index(idx.row(),1));
      m_pCurrent = nullptr;
      m_pTimer->stop();
   }
}

void RingtoneModel::collectionAddedCallback(CollectionInterface* backend)
{
   Q_UNUSED(backend)
}

bool RingtoneModel::addItemCallback(const Ringtone* item)
{
   Q_UNUSED(item)
   beginInsertRows(QModelIndex(),d_ptr->m_lRingtone.size(),d_ptr->m_lRingtone.size());
   d_ptr->m_lRingtone << const_cast<Ringtone*>(item);
   endInsertRows();
   return true;
}

bool RingtoneModel::removeItemCallback(const Ringtone* item)
{
   Q_UNUSED(item)
   return true;
}

#include <ringtonemodel.moc>
