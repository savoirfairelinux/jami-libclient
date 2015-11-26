/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#include "configurationproxy.h"

//Qt
#include <QtCore/QIdentityProxyModel>
#include <QtCore/QAbstractItemModel>
#include <QtCore/QItemSelectionModel>

//Ring
#include <video/sourcemodel.h>
#include <video/devicemodel.h>
#include <video/channel.h>
#include <video/resolution.h>
#include <video/rate.h>

namespace ConfigurationProxyPrivate {
   static QIdentityProxyModel* m_spDeviceModel    = nullptr;
   static QIdentityProxyModel* m_spChannelModel   = nullptr;
   static QIdentityProxyModel* m_spResolutionModel= nullptr;
   static QIdentityProxyModel* m_spRateModel      = nullptr;

   static QItemSelectionModel* m_spDeviceSelectionModel    = nullptr;
   static QItemSelectionModel* m_spChannelSelectionModel   = nullptr;
   static QItemSelectionModel* m_spResolutionSelectionModel= nullptr;
   static QItemSelectionModel* m_spRateSelectionModel      = nullptr;

   //Helper
   static Video::Device*     currentDevice    ();
   static Video::Channel*    currentChannel   ();
   static Video::Resolution* currentResolution();
//    static Video::Rate*       currentRate      ();

   static void changeDevice    ();
   static void changeChannel   ();
   static void changeResolution();
   static void changeRate      ();

   void updateDeviceSelection    ();
   void updateChannelSelection   ();
   void updateResolutionSelection();
   void updateRateSelection      ();
}

QAbstractItemModel& Video::ConfigurationProxy::deviceModel()
{
   if (!ConfigurationProxyPrivate::m_spDeviceModel) {
      ConfigurationProxyPrivate::m_spDeviceModel = new QIdentityProxyModel(&Video::SourceModel::instance());
      ConfigurationProxyPrivate::m_spDeviceModel->setSourceModel(&Video::DeviceModel::instance());
      ConfigurationProxyPrivate::updateDeviceSelection();
   }
   return *ConfigurationProxyPrivate::m_spDeviceModel;
}

static Video::Device* ConfigurationProxyPrivate::currentDevice()
{
   return Video::DeviceModel::instance().Video::DeviceModel::instance().activeDevice();
}

static Video::Channel* ConfigurationProxyPrivate::currentChannel()
{
   if (Video::DeviceModel::instance().activeDevice()
    && Video::DeviceModel::instance().activeDevice()->activeChannel())
      return Video::DeviceModel::instance().activeDevice()->activeChannel();

   return nullptr;
}

static Video::Resolution* ConfigurationProxyPrivate::currentResolution()
{
   if (Video::DeviceModel::instance().activeDevice()
    && Video::DeviceModel::instance().activeDevice()->activeChannel()
    && Video::DeviceModel::instance().activeDevice()->activeChannel()->activeResolution()
   )
      return Video::DeviceModel::instance().activeDevice()->activeChannel()->activeResolution();
   return nullptr;
}

/*static Video::Rate* ConfigurationProxyPrivate::currentRate()
{
   if (Video::DeviceModel::instance().activeDevice()
    && Video::DeviceModel::instance().activeDevice()->activeChannel()
    && Video::DeviceModel::instance().activeDevice()->activeChannel()->activeResolution()
   )
      return Video::DeviceModel::instance().activeDevice()->activeChannel()->activeResolution()->activeRate();

   return nullptr;
}*/

void ConfigurationProxyPrivate::changeDevice()
{
   Video::DeviceModel::instance().setActive(Video::ConfigurationProxy::deviceSelectionModel().currentIndex());

   reinterpret_cast<QIdentityProxyModel&>(Video::ConfigurationProxy::channelModel()).setSourceModel(ConfigurationProxyPrivate::currentDevice());
   changeChannel();
}

void ConfigurationProxyPrivate::changeChannel()
{
   if (auto dev = ConfigurationProxyPrivate::currentDevice())
     dev->setActiveChannel(Video::ConfigurationProxy::channelSelectionModel().currentIndex().row());

   reinterpret_cast<QIdentityProxyModel&>(Video::ConfigurationProxy::resolutionModel()).setSourceModel(ConfigurationProxyPrivate::currentChannel());

   updateChannelSelection();

   changeResolution();
}

void ConfigurationProxyPrivate::changeResolution()
{
   if (auto chan = ConfigurationProxyPrivate::currentChannel())
     chan->setActiveResolution(Video::ConfigurationProxy::resolutionSelectionModel().currentIndex().row());

   reinterpret_cast<QIdentityProxyModel&>(Video::ConfigurationProxy::rateModel()).setSourceModel(ConfigurationProxyPrivate::currentResolution());

   updateResolutionSelection();

   changeRate();
}

void ConfigurationProxyPrivate::changeRate()
{
   if (auto res = ConfigurationProxyPrivate::currentResolution())
     res->setActiveRate(Video::ConfigurationProxy::rateSelectionModel().currentIndex().row());

   updateRateSelection();
}

void ConfigurationProxyPrivate::updateDeviceSelection()
{
   if (ConfigurationProxyPrivate::m_spDeviceModel) {
      const QModelIndex& idx = ConfigurationProxyPrivate::m_spDeviceModel->index(Video::DeviceModel::instance().activeIndex(),0);
      if (idx.row() != Video::ConfigurationProxy::deviceSelectionModel().currentIndex().row())
         Video::ConfigurationProxy::deviceSelectionModel().setCurrentIndex(idx , QItemSelectionModel::ClearAndSelect);
   }
}

void ConfigurationProxyPrivate::updateChannelSelection()
{
   if (auto dev = ConfigurationProxyPrivate::currentDevice()) {
      if (auto chan = dev->activeChannel()) {
         const QModelIndex& newIdx = dev->index(chan->relativeIndex(),0);
         if (newIdx.row() != Video::ConfigurationProxy::channelSelectionModel().currentIndex().row())
            Video::ConfigurationProxy::channelSelectionModel().setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect );
      }
   }
}

void ConfigurationProxyPrivate::updateResolutionSelection()
{
   if (auto chan = ConfigurationProxyPrivate::currentChannel()) {
      if (auto res = chan->activeResolution()) {
         const QModelIndex& newIdx = chan->index(res->relativeIndex(),0);
         if (newIdx.row() != Video::ConfigurationProxy::resolutionSelectionModel().currentIndex().row())
            Video::ConfigurationProxy::resolutionSelectionModel().setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);
      }
   }
}

void ConfigurationProxyPrivate::updateRateSelection()
{
   if (auto res = ConfigurationProxyPrivate::currentResolution()) {
      if (auto rate = res->activeRate()) {
         const QModelIndex& newIdx = res->index(rate->relativeIndex(),0);
         if (newIdx.row() != Video::ConfigurationProxy::rateSelectionModel().currentIndex().row())
            Video::ConfigurationProxy::rateSelectionModel().setCurrentIndex(newIdx, QItemSelectionModel::ClearAndSelect);
      }
   }
}

QAbstractItemModel& Video::ConfigurationProxy::channelModel()
{
   if (!ConfigurationProxyPrivate::m_spChannelModel) {
      ConfigurationProxyPrivate::m_spChannelModel = new QIdentityProxyModel(&Video::SourceModel::instance());
      if (auto dev = ConfigurationProxyPrivate::currentDevice())
         ConfigurationProxyPrivate::m_spChannelModel->setSourceModel(dev);
   }
   return *ConfigurationProxyPrivate::m_spChannelModel;
}

QAbstractItemModel& Video::ConfigurationProxy::resolutionModel()
{
   if (!ConfigurationProxyPrivate::m_spResolutionModel) {
      ConfigurationProxyPrivate::m_spResolutionModel = new QIdentityProxyModel(&Video::SourceModel::instance());
      if (auto chan = ConfigurationProxyPrivate::currentChannel())
         ConfigurationProxyPrivate::m_spResolutionModel->setSourceModel(chan);
   }
   return *ConfigurationProxyPrivate::m_spResolutionModel;
}

QAbstractItemModel& Video::ConfigurationProxy::rateModel()
{
   if (!ConfigurationProxyPrivate::m_spRateModel) {
      ConfigurationProxyPrivate::m_spRateModel = new QIdentityProxyModel(&Video::SourceModel::instance());
      ConfigurationProxyPrivate::m_spRateModel->setSourceModel(ConfigurationProxyPrivate::currentResolution());
   }
   return *ConfigurationProxyPrivate::m_spRateModel;
}

QItemSelectionModel& Video::ConfigurationProxy::deviceSelectionModel()
{
   if (!ConfigurationProxyPrivate::m_spDeviceSelectionModel) {
      ConfigurationProxyPrivate::m_spDeviceSelectionModel = new QItemSelectionModel(&deviceModel());

      ConfigurationProxyPrivate::updateDeviceSelection();

      //Can happen if a device is removed
      QObject::connect(&Video::DeviceModel::instance(), &Video::DeviceModel::currentIndexChanged,[](int idx) {
         ConfigurationProxyPrivate::m_spDeviceSelectionModel->setCurrentIndex(deviceModel().index(idx,0), QItemSelectionModel::ClearAndSelect );
      });

      QObject::connect(ConfigurationProxyPrivate::m_spDeviceSelectionModel,&QItemSelectionModel::currentChanged, &ConfigurationProxyPrivate::changeDevice);
   }
   return *ConfigurationProxyPrivate::m_spDeviceSelectionModel;
}

QItemSelectionModel& Video::ConfigurationProxy::channelSelectionModel()
{
   if (!ConfigurationProxyPrivate::m_spChannelSelectionModel) {
      ConfigurationProxyPrivate::m_spChannelSelectionModel = new QItemSelectionModel(&channelModel());

      ConfigurationProxyPrivate::updateChannelSelection();

      QObject::connect(ConfigurationProxyPrivate::m_spChannelSelectionModel,&QItemSelectionModel::currentChanged, &ConfigurationProxyPrivate::changeChannel);
   }
   return *ConfigurationProxyPrivate::m_spChannelSelectionModel;
}

QItemSelectionModel& Video::ConfigurationProxy::resolutionSelectionModel()
{
   if (!ConfigurationProxyPrivate::m_spResolutionSelectionModel) {
      ConfigurationProxyPrivate::m_spResolutionSelectionModel = new QItemSelectionModel(&resolutionModel());

      ConfigurationProxyPrivate::updateResolutionSelection();

      QObject::connect(ConfigurationProxyPrivate::m_spResolutionSelectionModel,&QItemSelectionModel::currentChanged, &ConfigurationProxyPrivate::changeResolution);
   }
   return *ConfigurationProxyPrivate::m_spResolutionSelectionModel;
}

QItemSelectionModel& Video::ConfigurationProxy::rateSelectionModel()
{
   if (!ConfigurationProxyPrivate::m_spRateSelectionModel) {
      ConfigurationProxyPrivate::m_spRateSelectionModel = new QItemSelectionModel(&rateModel());

      ConfigurationProxyPrivate::updateRateSelection();

      QObject::connect(ConfigurationProxyPrivate::m_spRateSelectionModel,&QItemSelectionModel::currentChanged, &ConfigurationProxyPrivate::changeRate);
   }
   return *ConfigurationProxyPrivate::m_spRateSelectionModel;
}
