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
#ifdef ENABLE_PLUGIN
#include <plugin_manager_interface.h>
#endif
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
#ifdef ENABLE_PLUGIN
        return DRing::loadPlugin(path.toStdString());
#else
        return false;
#endif
    }

    bool unloadPlugin(const QString& path)
    {
#ifdef ENABLE_PLUGIN
        return DRing::unloadPlugin(path.toStdString());
#else
        return false;
#endif
    }

    MapStringString getPluginDetails(const QString& path)
    {
#ifdef ENABLE_PLUGIN
        return convertMap(DRing::getPluginDetails(path.toStdString()));
#else
        MapStringString temp;
        return temp;
#endif
    }

    QStringList listAvailablePlugins()
    {
#ifdef ENABLE_PLUGIN
        return convertStringList(DRing::listAvailablePlugins());
#else
        QStringList temp;
        return temp;
#endif
    }

    QStringList listLoadedPlugins()
    {
#ifdef ENABLE_PLUGIN
        return convertStringList(DRing::listLoadedPlugins());
#else
        QStringList temp;
        return temp;
#endif
    }

    int installPlugin(const QString& jplPath, bool force)
    {
#ifdef ENABLE_PLUGIN
        return DRing::installPlugin(jplPath.toStdString(), force);
#else
        return 0;
#endif
    }

    int uninstallPlugin(const QString& pluginRootPath)
    {
#ifdef ENABLE_PLUGIN
        return DRing::uninstallPlugin(pluginRootPath.toStdString());
#else
        return 0;
#endif
    }

    QStringList listCallMediaHandlers()
    {
#ifdef ENABLE_PLUGIN
        return convertStringList(DRing::listCallMediaHandlers());
#else
        QStringList temp;
        return temp;
#endif
    }

    void toggleCallMediaHandler(const QString& id, bool toggle)
    {
#ifdef ENABLE_PLUGIN
        DRing::toggleCallMediaHandler(id.toStdString(), toggle);
#endif
    }

    MapStringString getCallMediaHandlerStatus()
    {
#ifdef ENABLE_PLUGIN
        return convertMap(DRing::getCallMediaHandlerStatus());
#else
        MapStringString temp;
        return temp;
#endif
    }

    MapStringString getCallMediaHandlerDetails(const QString& id)
    {
#ifdef ENABLE_PLUGIN
        return convertMap(DRing::getCallMediaHandlerDetails(id.toStdString()));
#else
        MapStringString temp;
        return temp;
#endif
    }

    void setPluginsEnabled(bool enable)
    {
#ifdef ENABLE_PLUGIN
        DRing::setPluginsEnabled(enable);
#endif
    }

    bool getPluginsEnabled()
    {
#ifdef ENABLE_PLUGIN
        return DRing::getPluginsEnabled();
#else
        return false;
#endif
    }

    VectorMapStringString getPluginPreferences(const QString& path)
    {
        VectorMapStringString temp;
#ifdef ENABLE_PLUGIN
        for (auto x : DRing::getPluginPreferences(path.toStdString())) {
            temp.push_back(convertMap(x));
        }
#endif
        return temp;
    }

    bool setPluginPreference(const QString& path, const QString& key, const QString& value)
    {
#ifdef ENABLE_PLUGIN
        return DRing::setPluginPreference(path.toStdString(), key.toStdString(), value.toStdString());
#else
        return false;
#endif
    }

    MapStringString getPluginPreferencesValues(const QString& path)
    {
#ifdef ENABLE_PLUGIN
        return convertMap(DRing::getPluginPreferencesValues(path.toStdString()));
#else
        MapStringString temp;
        return temp;
#endif
    }

    bool resetPluginPreferencesValues(const QString& path)
    {
#ifdef ENABLE_PLUGIN
        return DRing::resetPluginPreferencesValues(path.toStdString());
#else
        return false;
#endif
    }
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::PluginManagerInterface PluginManager;
    }
  }
}
