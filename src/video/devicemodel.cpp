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
#include "devicemodel.h"
#include "device.h"
#include <call.h>
#include <account.h>
#include "../dbus/videomanager.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>

Video::DeviceModel* Video::DeviceModel::m_spInstance = nullptr;

namespace Video {
class DeviceModelPrivate : public QObject
{
   Q_OBJECT
public:
   DeviceModelPrivate();

   //Attrbutes
   QHash<QString,Video::Device*> m_hDevices     ;
   QList<Video::Device*>         m_lDevices     ;
   Video::Device*                m_pDummyDevice ;
   Video::Device*                m_pActiveDevice;

private Q_SLOTS:
   void idleReload();
};
}

Video::DeviceModelPrivate::DeviceModelPrivate() : m_pDummyDevice(nullptr),m_pActiveDevice(nullptr)
{
}

///
void Video::DeviceModelPrivate::idleReload()
{
   DeviceModel::instance()->setActive(DeviceModel::instance()->activeDevice());
}

///Constructor
Video::DeviceModel::DeviceModel() : QAbstractListModel(QCoreApplication::instance()),
d_ptr(new Video::DeviceModelPrivate())
{
   m_spInstance = this;
   reload();
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   connect(&interface, SIGNAL(deviceEvent()), this, SLOT(reload()));
}

Video::DeviceModel* Video::DeviceModel::instance()
{
   if (!m_spInstance)
      m_spInstance = new Video::DeviceModel();
   return m_spInstance;
}

QHash<int,QByteArray> Video::DeviceModel::roleNames() const
{
   static QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

///Get data from the model
QVariant Video::DeviceModel::data( const QModelIndex& idx, int role) const
{
   if(idx.isValid() && idx.column() == 0 && role == Qt::DisplayRole && d_ptr->m_lDevices.size() > idx.row())
      return QVariant(d_ptr->m_lDevices[idx.row()]->id());
   return QVariant();
}

///The number of codec
int Video::DeviceModel::rowCount( const QModelIndex& par ) const
{
   Q_UNUSED(par)
   return d_ptr->m_lDevices.size();
}

///Items flag
Qt::ItemFlags Video::DeviceModel::flags( const QModelIndex& idx ) const
{
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   return QAbstractItemModel::flags(idx);
}

///Set the codec data (codecs can't be added or removed that way)
bool Video::DeviceModel::setData(const QModelIndex& idx, const QVariant &value, int role)
{
   Q_UNUSED(idx)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

///Destructor
Video::DeviceModel::~DeviceModel()
{
   while (d_ptr->m_lDevices.size()) {
      Video::Device* c = d_ptr->m_lDevices[0];
      d_ptr->m_lDevices.removeAt(0);
      delete c;
   }
//    delete d_ptr;
}

///Save the current model over dbus
void Video::DeviceModel::setActive(const QModelIndex& idx)
{
   if (idx.isValid() && d_ptr->m_lDevices.size() > idx.row()) {
      VideoManagerInterface& interface = DBus::VideoManager::instance();
      interface.setDefaultDevice(d_ptr->m_lDevices[idx.row()]->id());
      d_ptr->m_pActiveDevice = d_ptr->m_lDevices[idx.row()];
      emit changed();
      emit currentIndexChanged(idx.row());
   }
}

///Convenience
void Video::DeviceModel::setActive(const int idx)
{
   setActive(index(idx,0,QModelIndex()));
}


void Video::DeviceModel::setActive(const Video::Device* device)
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();

   interface.setDefaultDevice(device?device->id():Video::Device::NONE);
   d_ptr->m_pActiveDevice = const_cast<Video::Device*>(device);
   emit changed();
   const int idx = d_ptr->m_lDevices.indexOf((Video::Device*)device);
   emit currentIndexChanged(idx);
}

void Video::DeviceModel::reload()
{
   QHash<QString,Video::Device*> devicesHash;
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   const QStringList deviceList = interface.getDeviceList();
   if (deviceList.size() == d_ptr->m_hDevices.size()) {
      d_ptr->m_lDevices = d_ptr->m_hDevices.values();
   }

   foreach(const QString& deviceName,deviceList) {
      if (!d_ptr->m_hDevices[deviceName]) {
         devicesHash[deviceName] = new Video::Device(deviceName);
      }
      else {
         devicesHash[deviceName] = d_ptr->m_hDevices[deviceName];
      }
   }
   foreach(Video::Device* dev, d_ptr->m_hDevices) {
      if (dev && devicesHash.key(dev).isEmpty()) {
         dev->deleteLater();
      }
   }
   d_ptr->m_pActiveDevice = nullptr;
   d_ptr->m_hDevices.clear();
   d_ptr->m_hDevices = devicesHash;
   beginResetModel();
   d_ptr->m_lDevices = d_ptr->m_hDevices.values();
   endResetModel();

   emit layoutChanged();
//    channelModel   ()->reload();

   //Avoid a possible infinite loop by using a reload event
   QTimer::singleShot(0,d_ptr.data(),SLOT(idleReload()));

}


Video::Device* Video::DeviceModel::activeDevice() const
{
   if (!d_ptr->m_pActiveDevice) {
      VideoManagerInterface& interface = DBus::VideoManager::instance();
      const QString deId = interface.getDefaultDevice();
      if (!d_ptr->m_lDevices.size())
         const_cast<Video::DeviceModel*>(this)->reload();
      Video::Device* dev =  d_ptr->m_hDevices[deId];

      //Handling null everywhere is too long, better create a dummy device and
      //log the event
      if (!dev) {
         if (!deId.isEmpty())
            qWarning() << "Requested unknown device" << deId;
         if (!d_ptr->m_pDummyDevice)
            d_ptr->m_pDummyDevice = new Video::Device("None");
         return d_ptr->m_pDummyDevice;
      }
      d_ptr->m_pActiveDevice = dev;
   }
   return d_ptr->m_pActiveDevice;
}

int Video::DeviceModel::activeIndex() const
{
   return d_ptr->m_lDevices.indexOf(activeDevice());
}


Video::Device* Video::DeviceModel::getDevice(const QString& devId) const
{
   return d_ptr->m_hDevices[devId];
}

QList<Video::Device*> Video::DeviceModel::devices() const
{
   return d_ptr->m_lDevices;
}

#include <devicemodel.moc>
