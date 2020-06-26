/******************************************************************************
 *    Copyright (C) 2014-2020 Savoir-faire Linux Inc.                         *
 *   Author : Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>   *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#pragma once

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtCore/QTimer>

#include "typedefs.h"
#include <plugin_manager_interface.h>
#include "conversions_wrap.hpp"


/*
 * Proxy class for interface org.ring.Ring.PluginManager
 */
class PluginManagerInterface: public QObject
{
    Q_OBJECT
public:

    PluginManagerInterface() {}
    ~PluginManagerInterface() {}

public Q_SLOTS: // METHODS

    bool loadPlugin(const QString& path)
    {
        return DRing::loadPlugin(path.toStdString());
    }

    bool unloadPlugin(const QString& path)
    {
        return DRing::unloadPlugin(path.toStdString());
    }

    MapStringString getPluginDetails(const QString& path)
    {
        MapStringString temp =
            convertMap(DRing::getPluginDetails(path.toStdString()));
        return temp;
    }

    QStringList listAvailablePlugins()
    {
        return convertStringList(DRing::listAvailablePlugins());
    }

    QStringList listLoadedPlugins()
    {
        return convertStringList(DRing::listLoadedPlugins());
    }

    int installPlugin(const QString& jplPath, bool force)
    {
        return DRing::installPlugin(jplPath.toStdString(), force);
    }

    int uninstallPlugin(const QString& pluginRootPath)
    {
        return DRing::uninstallPlugin(pluginRootPath.toStdString());
    }

    QStringList listCallMediaHandlers()
    {
        return convertStringList(DRing::listCallMediaHandlers());
    }

    void toggleCallMediaHandler(const QString& id, bool toggle)
    {
        DRing::toggleCallMediaHandler(id.toStdString(), toggle);
    }

    MapStringString getCallMediaHandlerStatus()
    {
        return convertMap(DRing::getCallMediaHandlerStatus());
    }

    MapStringString getCallMediaHandlerDetails(const QString& id)
    {
        return convertMap(DRing::getCallMediaHandlerDetails(id.toStdString()));
    }

    void setPluginsEnabled(bool enable)
    {
        DRing::setPluginsEnabled(enable);
    }

    bool getPluginsEnabled()
    {
        return DRing::getPluginsEnabled();
    }

};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::PluginManagerInterface PluginManager;
    }
  }
}
