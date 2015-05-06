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
#include "managermodel.h"

//Qt
#include <QtCore/QItemSelectionModel>

//Ring
#include "dbus/configurationmanager.h"
#include "settings.h"

class ManagerModelPrivate : public QObject
{
   Q_OBJECT
public:
   ManagerModelPrivate(Audio::ManagerModel* parent);
   class ManagerName {
   public:
      constexpr static const char* PULSEAUDIO = "pulseaudio";
      constexpr static const char* ALSA       = "alsa"      ;
      constexpr static const char* JACK       = "jack"      ;
   };

   QStringList m_lDeviceList;
   QList<Audio::ManagerModel::Manager> m_lSupportedManagers;
   mutable QItemSelectionModel* m_pSelectionModel;

private:
   Audio::ManagerModel* q_ptr;

public Q_SLOTS:
   void slotSelectionChanged(const QModelIndex& idx);
};

ManagerModelPrivate::ManagerModelPrivate(Audio::ManagerModel* parent) : q_ptr(parent),
   m_pSelectionModel(nullptr)
{

}

///Constructor
Audio::ManagerModel::ManagerModel(const QObject* parent) : QAbstractListModel(const_cast<QObject*>(parent)),
d_ptr(new ManagerModelPrivate(this))
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   const QStringList managers = configurationManager.getSupportedAudioManagers();
   foreach(const QString& m,managers) {
      if (m == ManagerModelPrivate::ManagerName::PULSEAUDIO) {
         d_ptr->m_lSupportedManagers << Manager::PULSE;
         d_ptr->m_lDeviceList << "Pulse Audio";
      }
      else if (m == ManagerModelPrivate::ManagerName::ALSA) {
         d_ptr->m_lSupportedManagers << Manager::ALSA;
         d_ptr->m_lDeviceList<< "ALSA";
      }
      else if (m == ManagerModelPrivate::ManagerName::JACK) {
         d_ptr->m_lSupportedManagers << Manager::JACK;
         d_ptr->m_lDeviceList<< "Jack";
      }
      else
         qDebug() << "Unsupported audio manager" << m;
   }
}

///Destructor
Audio::ManagerModel::~ManagerModel()
{
   d_ptr->m_lDeviceList.clear();
}

QHash<int,QByteArray> Audio::ManagerModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Re-implement QAbstractListModel data
QVariant Audio::ManagerModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();
   switch(role) {
      case Qt::DisplayRole:
         return d_ptr->m_lDeviceList[index.row()];
   };
   return QVariant();
}

///Re-implement QAbstractListModel rowCount
int Audio::ManagerModel::rowCount( const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return d_ptr->m_lDeviceList.size();
}

///Re-implement QAbstractListModel flags
Qt::ItemFlags Audio::ManagerModel::flags( const QModelIndex& index ) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///This model is read only
bool Audio::ManagerModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

/**
 * This model allow automatic synchronization of the audio manager
 */
QItemSelectionModel* Audio::ManagerModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<Audio::ManagerModel*>(this));
      connect(d_ptr->m_pSelectionModel,&QItemSelectionModel::currentChanged,d_ptr.data(),&ManagerModelPrivate::slotSelectionChanged);

      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      const QString manager = configurationManager.getAudioManager();

      if (manager == ManagerModelPrivate::ManagerName::PULSEAUDIO)
         d_ptr->m_pSelectionModel->setCurrentIndex( index((int)Manager::PULSE,0) , QItemSelectionModel::ClearAndSelect );
      else if (manager == ManagerModelPrivate::ManagerName::ALSA)
         d_ptr->m_pSelectionModel->setCurrentIndex( index((int)Manager::ALSA,0)  , QItemSelectionModel::ClearAndSelect );
      else if (manager == ManagerModelPrivate::ManagerName::JACK)
         d_ptr->m_pSelectionModel->setCurrentIndex( index((int)Manager::JACK,0)  , QItemSelectionModel::ClearAndSelect );
   }

   return d_ptr->m_pSelectionModel;
}

void ManagerModelPrivate::slotSelectionChanged(const QModelIndex& idx)
{
   if (!idx.isValid())
      return;

   bool ret = true;
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   switch (m_lSupportedManagers[idx.row()]) {
      case Audio::ManagerModel::Manager::PULSE:
         ret = configurationManager.setAudioManager(ManagerModelPrivate::ManagerName::PULSEAUDIO);
         Audio::Settings::instance()->reload();
         break;
      case Audio::ManagerModel::Manager::ALSA:
         ret = configurationManager.setAudioManager(ManagerModelPrivate::ManagerName::ALSA);
         Audio::Settings::instance()->reload();
         break;
      case Audio::ManagerModel::Manager::JACK:
         ret = configurationManager.setAudioManager(ManagerModelPrivate::ManagerName::JACK);
         Audio::Settings::instance()->reload();
         break;
      case Audio::ManagerModel::Manager::ERROR:
         break;
   };
   if (!ret) {
      emit q_ptr->currentManagerChanged(q_ptr->currentManager());
   }
   return;
}

Audio::ManagerModel::Manager Audio::ManagerModel::currentManager() const
{
   const int idx = selectionModel()->currentIndex().row();
   return idx>=0 ? d_ptr->m_lSupportedManagers[idx] : Manager::ERROR;
}

#include <managermodel.moc>
