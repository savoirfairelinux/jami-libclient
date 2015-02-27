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
#include "outputdevicemodel.h"

//Qt
#include <QtCore/QItemSelectionModel>

//Ring
#include "dbus/configurationmanager.h"
#include "dbus/callmanager.h"
#include "settings.h"

class OutputDeviceModelPrivate : public QObject
{
   Q_OBJECT
public:
   OutputDeviceModelPrivate(Audio::OutputDeviceModel* parent);
   QStringList m_lDeviceList;
   mutable QItemSelectionModel* m_pSelectionModel;

private:
   Audio::OutputDeviceModel* q_ptr;
};


OutputDeviceModelPrivate::OutputDeviceModelPrivate(Audio::OutputDeviceModel* parent) : q_ptr(parent),
    m_pSelectionModel(nullptr)
{

}

///Constructor
Audio::OutputDeviceModel::OutputDeviceModel(const QObject* parent) : QAbstractListModel(const_cast<QObject*>(parent)),
d_ptr(new OutputDeviceModelPrivate(this))
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   d_ptr->m_lDeviceList = configurationManager.getAudioOutputDeviceList();
}

///Destructor
Audio::OutputDeviceModel::~OutputDeviceModel()
{

}

QHash<int,QByteArray> Audio::OutputDeviceModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Re-implement QAbstractListModel data
QVariant Audio::OutputDeviceModel::data( const QModelIndex& index, int role) const
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
int Audio::OutputDeviceModel::rowCount( const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return d_ptr->m_lDeviceList.size();
}

///Re-implement QAbstractListModel flags
Qt::ItemFlags Audio::OutputDeviceModel::flags( const QModelIndex& index ) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///This model is read only
bool Audio::OutputDeviceModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QItemSelectionModel* Audio::OutputDeviceModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<Audio::OutputDeviceModel*>(this));
      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      const QStringList currentDevices = configurationManager.getCurrentAudioDevicesIndex();
      const int idx = currentDevices[static_cast<int>(Audio::Settings::DeviceIndex::OUTPUT)].toInt();
      if (!(idx >= d_ptr->m_lDeviceList.size()))
         d_ptr->m_pSelectionModel->setCurrentIndex(index(idx,0), QItemSelectionModel::ClearAndSelect);
   }
   return d_ptr->m_pSelectionModel;
}

///Set the current output device
void Audio::OutputDeviceModel::setCurrentDevice(const QModelIndex& index)
{
   if (index.isValid()) {
      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      configurationManager.setAudioOutputDevice(index.row());
   }
}

///QCombobox index -> QModelIndex shim
void Audio::OutputDeviceModel::setCurrentDevice(int idx)
{
   setCurrentDevice(index(idx,0));
}

///reload output devices list
void Audio::OutputDeviceModel::reload()
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
   d_ptr->m_lDeviceList = configurationManager.getAudioOutputDeviceList();
   emit layoutChanged();
   emit dataChanged(index(0,0),index(d_ptr->m_lDeviceList.size()-1,0));
}

void Audio::OutputDeviceModel::playDTMF(const QString& str)
{
   Q_NOREPLY DBus::CallManager::instance().playDTMF(str);
}


#include <outputdevicemodel.moc>
