/****************************************************************************
 *   Copyright (C) 2013-2016 by Savoir-faire Linux                          *
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
#include <QtCore/QCoreApplication>
#include <QtCore/QUrl>

//Ring
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"
#include "account.h"
#include "accountmodel.h"
#include "ringtone.h"
#include <localringtonecollection.h>

class RingtoneModelPrivate final : public QObject
{
   Q_OBJECT
public:

   RingtoneModelPrivate(RingtoneModel*);

   //Attributes
   QVector<Ringtone*>                   m_lRingtone        ;
   QTimer*                              m_pTimer           ;
   Ringtone*                            m_pCurrent         ;
   QHash<Account*,int>                  m_hCurrent         ;
   QHash<Account*,QItemSelectionModel*> m_hSelectionModels ;
   LocalRingtoneCollection*             m_pCollection      ;
   QHash<const Ringtone*,Account*>      m_hPendingSelection;
   bool                                 m_isPlaying        ;

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
  , m_isPlaying(false)
{

}

RingtoneModel::RingtoneModel(QObject* parent)
  : QAbstractTableModel(parent)
  , CollectionManagerInterface<Ringtone>(this)
  , d_ptr(new RingtoneModelPrivate(this))
{
   d_ptr->m_pCollection = addCollection<LocalRingtoneCollection>();
   QObject::connect(AccountModel::instance().selectionModel(),
                 &QItemSelectionModel::currentChanged,
                 [=](const QModelIndex &current, const QModelIndex &previous) {
                     Q_UNUSED(current)
                    if (d_ptr->m_isPlaying && previous.isValid()) {
                        auto acc = AccountModel::instance().getAccountByModelIndex(previous);
                        auto qIdx = d_ptr->m_hSelectionModels[acc]->currentIndex();
                        play(qIdx);
                    }
                 });
}

RingtoneModel& RingtoneModel::instance()
{
   static auto instance = new RingtoneModel(QCoreApplication::instance());
   return *instance;
}

RingtoneModel::~RingtoneModel()
{
   while (d_ptr->m_lRingtone.size()) {
      Ringtone* ringtone = d_ptr->m_lRingtone[0];
      d_ptr->m_lRingtone.removeAt(0);
      delete ringtone;
   }
   delete d_ptr;
}

QHash<int,QByteArray> RingtoneModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;
      roles[Role::FullPath  ] = "FullPath";
   }
   return roles;
}

bool RingtoneModel::isPlaying()
{
    return d_ptr->m_isPlaying;
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
   return 1;
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
      if (info->path() == rt)
         return i;
   }
   return 0;
}

QItemSelectionModel* RingtoneModel::selectionModel(Account* a) const
{
   if (!d_ptr->m_hSelectionModels[a]) {
      d_ptr->m_hSelectionModels[a] = new QItemSelectionModel(const_cast<RingtoneModel*>(this));
      d_ptr->m_hSelectionModels[a]->setCurrentIndex(index(d_ptr->currentIndex(a),0), QItemSelectionModel::ClearAndSelect);

      connect(d_ptr->m_hSelectionModels[a],&QItemSelectionModel::currentChanged, [a,this](const QModelIndex& idx) {
         if (idx.isValid()) {
            a->setRingtonePath(d_ptr->m_lRingtone[idx.row()]->path());
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
      CallManagerInterface& callManager = CallManager::instance();
      Q_NOREPLY callManager.startRecordedFilePlayback(info->path());
      if (!d_ptr->m_pTimer) {
         d_ptr->m_pTimer = new QTimer(this);
         d_ptr->m_pTimer->setInterval(10000);
         connect(d_ptr->m_pTimer,SIGNAL(timeout()),d_ptr,SLOT(slotStopTimer()));
      }
      else if (d_ptr->m_pTimer->isActive()) {
         d_ptr->m_pTimer->stop();
      }
      d_ptr->m_pTimer->start();
      d_ptr->m_isPlaying = true;
      emit dataChanged(index(idx.row(),0),index(idx.row(),1));
      d_ptr->m_pCurrent = info;
   }
}

void RingtoneModelPrivate::slotStopTimer()
{
   if (m_pCurrent) {
      CallManagerInterface& callManager = CallManager::instance();
      callManager.stopRecordedFilePlayback();
      m_isPlaying = false;
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

   if (auto a = d_ptr->m_hPendingSelection[item]) {

      if (auto sm = d_ptr->m_hSelectionModels[a])
         sm->setCurrentIndex(index(rowCount()-1,0), QItemSelectionModel::ClearAndSelect);
      else
         a->setRingtonePath(item->path());

      d_ptr->m_hPendingSelection[item] = nullptr;
   }

   return true;
}

bool RingtoneModel::removeItemCallback(const Ringtone* item)
{
   Q_UNUSED(item)
   return true;
}

bool RingtoneModel::add(const QUrl& path, Account* autoSelect)
{
   auto r = new Ringtone(this);
   r->setPath(path.toLocalFile());
   r->setName(QFile(path.toLocalFile()).fileName());

   if (autoSelect)
      d_ptr->m_hPendingSelection[r] = autoSelect;

   d_ptr->m_pCollection->add(r);

   //TODO check the file type
   //TODO avoid duplicates

   return true;
}

#include <ringtonemodel.moc>
