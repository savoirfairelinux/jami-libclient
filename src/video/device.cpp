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
#include "device.h"

//Qt
#include <QtCore/QTimer>

//Ring
#include "../dbus/videomanager.h"
#include "devicemodel.h"
#include "resolution.h"
#include "rate.h"
#include "channel.h"
#include "renderer.h"
#include "previewmanager.h"

//Ring private
#include "../private/videochannel_p.h"
#include "../private/videodevice_p.h"
#include "../private/videorate_p.h"
#include "../private/videoresolution_p.h"
#include "../private/videorenderermanager.h"

VideoDevicePrivate::VideoDevicePrivate(Video::Device* parent) : QObject(parent),m_pCurrentChannel(nullptr),m_RequireSave(false),q_ptr(parent)
{
}

///Constructor
Video::Device::Device(const QString &id) : QAbstractListModel(nullptr),
d_ptr(new VideoDevicePrivate(this))
{
   d_ptr->m_DeviceId = id;
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   MapStringMapStringVectorString cap = interface.getCapabilities(id);
   QMapIterator<QString, MapStringVectorString> channels(cap);
   while (channels.hasNext()) {
      channels.next();

      Video::Channel* chan = new Video::Channel(this,channels.key());
      d_ptr->m_lChannels << chan;

      QMapIterator<QString, VectorString> resolutions(channels.value());
      while (resolutions.hasNext()) {
         resolutions.next();

         Video::Resolution* res = new Video::Resolution(resolutions.key(),chan);
         chan->d_ptr->m_lValidResolutions << res;

         foreach(const QString& rate, resolutions.value()) {
            Video::Rate* r = new Video::Rate(res,rate);
            res->d_ptr->m_lValidRates << r;
         }
      }
   }
   Q_UNUSED(cap)
}

///Destructor
Video::Device::~Device()
{
//    delete d_ptr;
}

QVariant Video::Device::data( const QModelIndex& index, int role) const
{
   if (index.isValid() && role == Qt::DisplayRole && d_ptr->m_lChannels.size() > index.row()) {
      return d_ptr->m_lChannels[index.row()]->name();
   }
   return QVariant();
}

int Video::Device::rowCount( const QModelIndex& parent) const
{
   return (parent.isValid())?0:d_ptr->m_lChannels.size();
}

Qt::ItemFlags Video::Device::flags( const QModelIndex& idx) const
{
   if (idx.column() == 0)
      return QAbstractItemModel::flags(idx) | Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
   return QAbstractItemModel::flags(idx);
}

bool Video::Device::setData( const QModelIndex& index, const QVariant &value, int role)
{
   Q_UNUSED(index)
   Q_UNUSED(value)
   Q_UNUSED(role)
   return false;
}

// int Video::Device::relativeIndex() {
//    return m_pDevice->channelList().indexOf(this);
// }

///Get the valid channel list
QList<Video::Channel*> Video::Device::channelList() const
{
   return d_ptr->m_lChannels;
}

///Save the current settings
void Video::Device::save()
{
   if (!d_ptr->m_RequireSave) {
      d_ptr->m_RequireSave = true;
      //A little delay wont hurt
      QTimer::singleShot(100,d_ptr.data(),SLOT(saveIdle()));
   }
}

///Get the device id
const QString Video::Device::id() const
{
   return d_ptr->m_DeviceId;
}

///Get the device name
const QString Video::Device::name() const
{
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   return QMap<QString,QString>(interface.getSettings(d_ptr->m_DeviceId))[VideoDevicePrivate::PreferenceNames::NAME];;
}

///Is this device the default one
bool Video::Device::isActive() const
{
   return Video::DeviceModel::instance()->activeDevice() == this;
}

bool Video::Device::setActiveChannel(Video::Channel* chan)
{
   if (!chan || !d_ptr->m_lChannels.indexOf(chan)) {
      qWarning() << "Trying to set an invalid channel" << (chan?chan->name():"NULL") << "for" << id();
      return false;
   }

   if (d_ptr->m_pCurrentChannel == chan)
      return false;

   d_ptr->m_pCurrentChannel = chan;
   save();
   return true;
}

bool Video::Device::setActiveChannel(int idx)
{
   if (idx < 0 || idx >= d_ptr->m_lChannels.size()) return false;
   return setActiveChannel(d_ptr->m_lChannels[idx]);
}

Video::Channel* Video::Device::activeChannel() const
{
   if (!d_ptr->m_pCurrentChannel) {
      VideoManagerInterface& interface = DBus::VideoManager::instance();
      const QString chan = QMap<QString,QString>(interface.getSettings(d_ptr->m_DeviceId))[VideoDevicePrivate::PreferenceNames::CHANNEL];
      foreach(Video::Channel* c, d_ptr->m_lChannels) {
         if (c->name() == chan) {
            d_ptr->m_pCurrentChannel = c;
            break;
         }
      }
   }
   if (!d_ptr->m_pCurrentChannel && d_ptr->m_lChannels.size()) {
      d_ptr->m_pCurrentChannel = d_ptr->m_lChannels[0];
   }
   return d_ptr->m_pCurrentChannel;
}


void VideoDevicePrivate::saveIdle()
{
   m_RequireSave = false;

   //In case new (unsupported) fields are added, merge with existing
   VideoManagerInterface& interface = DBus::VideoManager::instance();
   MapStringString pref = interface.getSettings(m_DeviceId);

   Video::Channel* chan = q_ptr->activeChannel();

   if (!chan) {
      qWarning() << "Saving video failed: Invalid channel";
      return;
   }

   Video::Resolution* res = chan->activeResolution();

   if (!res) {
      qWarning() << "Saving video failed: Invalid resolution";
      return;
   }

   Video::Rate* rate = res->activeRate();

   if (!rate) {
      qWarning() << "Saving video failed: Invalid rate";
      return;
   }

   pref[VideoDevicePrivate::PreferenceNames::CHANNEL] = chan->name ();
   pref[VideoDevicePrivate::PreferenceNames::SIZE   ] = res ->name ();
   pref[VideoDevicePrivate::PreferenceNames::RATE   ] = rate->name ();
   interface.applySettings(m_DeviceId,pref);

   //If the preview is running, reload it
   //doing this during a call will cause re-invite, this is unwanted
   if (Video::PreviewManager::instance()->isPreviewing() && VideoRendererManager::instance()->size() == 1) {
      Video::PreviewManager::instance()->stopPreview();
      Video::PreviewManager::instance()->startPreview();
   }
}
