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
#include "inputdevicemodel.h"

//Qt
#include <QtCore/QItemSelectionModel>

//Ring
#include "dbus/configurationmanager.h"
#include "settings.h"

class InputDeviceModelPrivate final : public QObject
{
   Q_OBJECT
public:
   InputDeviceModelPrivate(Audio::InputDeviceModel* parent);
   QStringList m_lDeviceList;
   mutable QItemSelectionModel* m_pSelectionModel;
   QModelIndex currentDevice() const;

private:
   Audio::InputDeviceModel* q_ptr;

public Q_SLOTS:
   void setCurrentDevice(const QModelIndex& index);
};

InputDeviceModelPrivate::InputDeviceModelPrivate(Audio::InputDeviceModel* parent) : q_ptr(parent),
    m_pSelectionModel(nullptr)
{

}

QHash<int,QByteArray> Audio::InputDeviceModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Constructor
Audio::InputDeviceModel::InputDeviceModel(const QObject* parent) : QAbstractListModel(const_cast<QObject*>(parent)),
d_ptr(new InputDeviceModelPrivate(this))
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   d_ptr->m_lDeviceList = configurationManager.getAudioInputDeviceList  ();
   connect(&configurationManager, SIGNAL(audioDeviceEvent()), this, SLOT(reload()));
}

///Destructor
Audio::InputDeviceModel::~InputDeviceModel()
{

}

///Re-implement QAbstractListModel data
QVariant Audio::InputDeviceModel::data( const QModelIndex& index, int role) const
{
   if (!index.isValid())
      return QVariant();
   switch(role) {
      case Qt::DisplayRole:
         if (index.row() < d_ptr->m_lDeviceList.size())
            return d_ptr->m_lDeviceList[index.row()];
   };
   return QVariant();
}

///Re-implement QAbstractListModel rowCount
int Audio::InputDeviceModel::rowCount( const QModelIndex& parent ) const
{
   if (parent.isValid())
      return 0;
   return d_ptr->m_lDeviceList.size();
}

///Re-implement QAbstractListModel flags
Qt::ItemFlags Audio::InputDeviceModel::flags( const QModelIndex& index ) const
{
   Q_UNUSED(index)
   return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///This model does not support setting data
bool Audio::InputDeviceModel::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

QItemSelectionModel* Audio::InputDeviceModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<Audio::InputDeviceModel*>(this));

      ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
      const QStringList currentDevices = configurationManager.getCurrentAudioDevicesIndex();
      const auto input_idx = static_cast<int>(Settings::DeviceIndex::INPUT);
      if (input_idx < currentDevices.size()) {
         const int idx = currentDevices[input_idx].toInt();
         if (idx < d_ptr->m_lDeviceList.size())
            d_ptr->m_pSelectionModel->setCurrentIndex(index(idx,0), QItemSelectionModel::ClearAndSelect);
      }

      connect(d_ptr->m_pSelectionModel, &QItemSelectionModel::currentChanged, d_ptr.data(), &InputDeviceModelPrivate::setCurrentDevice);
   }

   return d_ptr->m_pSelectionModel;
}

///Return the current ringtone device
QModelIndex InputDeviceModelPrivate::currentDevice() const
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   const QStringList currentDevices = configurationManager.getCurrentAudioDevicesIndex();
   const auto input_idx = static_cast<int>(Audio::Settings::DeviceIndex::INPUT);
   if (input_idx < currentDevices.size()) {
      const int idx = currentDevices[input_idx].toInt();
      if (idx < m_lDeviceList.size())
         return q_ptr->index(idx,0);
   }
   return {};
}

///Set the current input device
void InputDeviceModelPrivate::setCurrentDevice(const QModelIndex& index)
{
   if (index.isValid() and index != currentDevice()) {
      ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
      configurationManager.setAudioInputDevice(index.row());
   }
}

///Reload input device list
void Audio::InputDeviceModel::reload()
{
   ConfigurationManagerInterface& configurationManager = ConfigurationManager::instance();
   beginResetModel();
   d_ptr->m_lDeviceList = configurationManager.getAudioInputDeviceList  ();
   endResetModel();

   // Restore the selection
   selectionModel()->setCurrentIndex(d_ptr->currentDevice(), QItemSelectionModel::ClearAndSelect);
}

#include <inputdevicemodel.moc>
