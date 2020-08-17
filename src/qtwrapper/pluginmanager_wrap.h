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

    bool loadPlugin(const QString& path);

    bool unloadPlugin(const QString& path);

    MapStringString getPluginDetails(const QString& path);

    QStringList listAvailablePlugins();

    QStringList listLoadedPlugins();

    int installPlugin(const QString& jplPath, bool force);

    int uninstallPlugin(const QString& pluginRootPath);

    QStringList listCallMediaHandlers();

    void toggleCallMediaHandler(const QString& callID, const QString& id, bool toggle);

    MapStringString getCallMediaHandlerStatus(const QString& callID);

    MapStringString getCallMediaHandlerDetails(const QString& id);

    void setPluginsEnabled(bool enable);

    bool getPluginsEnabled();

    VectorMapStringString getPluginPreferences(const QString& path);

    bool setPluginPreference(const QString& path, const QString& key, const QString& value);

    MapStringString getPluginPreferencesValues(const QString& path);

    bool resetPluginPreferencesValues(const QString& path);
};

namespace org {
  namespace ring {
    namespace Ring {
      typedef ::PluginManagerInterface PluginManager;
    }
  }
}
