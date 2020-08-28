/**
 *    Copyright (C) 2020 Savoir-faire Linux Inc.
 *   Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "api/pluginmodel.h"

// Std
#include <algorithm> // std::sort
#include <chrono>
#include <csignal>
#include <iomanip> // for std::put_time
#include <fstream>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>
#include <QUrl>

// Ring daemon

// LRC
#include "dbus/pluginmanager.h"

namespace lrc {

using namespace api;

PluginModel::PluginModel()
    : QObject()
{}

PluginModel::~PluginModel() {}

void
PluginModel::setPluginsEnabled(bool enable)
{
    PluginManager::instance().setPluginsEnabled(enable);
}

bool
PluginModel::getPluginsEnabled() const
{
    return PluginManager::instance().getPluginsEnabled();
}

VectorString
PluginModel::listAvailablePlugins() const
{
    return VectorString::fromList(PluginManager::instance().listAvailablePlugins());
}

VectorString
PluginModel::listLoadedPlugins() const
{
    return VectorString::fromList(PluginManager::instance().listLoadedPlugins());
}

plugin::PluginDetails
PluginModel::getPluginDetails(const QString& path)
{
    if (path.isEmpty()) {
        return plugin::PluginDetails();
    }
    MapStringString details = PluginManager::instance().getPluginDetails(path);
    plugin::PluginDetails result;
    if (!details.empty()) {
        result.name = details["name"];
        result.path = path;
        result.iconPath = details["iconPath"];
    }

    VectorString loadedPlugins = listLoadedPlugins();
    if (std::find(loadedPlugins.begin(), loadedPlugins.end(), result.path) != loadedPlugins.end()) {
        result.loaded = true;
    }

    return result;
}

bool
PluginModel::installPlugin(const QString& jplPath, bool force)
{
    if (getPluginsEnabled()) {
        return PluginManager::instance().installPlugin(jplPath, force);
    }
    return false;
}

bool
PluginModel::uninstallPlugin(const QString& rootPath)
{
    return PluginManager::instance().uninstallPlugin(rootPath);
}

bool
PluginModel::loadPlugin(const QString& path)
{
    bool status = PluginManager::instance().loadPlugin(path);
    return status;
}

bool
PluginModel::unloadPlugin(const QString& path)
{
    return PluginManager::instance().unloadPlugin(path);
}

VectorString
PluginModel::listCallMediaHandlers() const
{
    return VectorString::fromList(PluginManager::instance().listCallMediaHandlers());
}

void
PluginModel::toggleCallMediaHandler(const QString& id)
{
    MapStringString toggleInfo = PluginManager::instance().getCallMediaHandlerStatus();
    if (toggleInfo["name"] == id) {
        PluginManager::instance().toggleCallMediaHandler(id, false);
    } else {
        PluginManager::instance().toggleCallMediaHandler(id, true);
    }
}

MapStringString
PluginModel::getCallMediaHandlerStatus()
{
    return PluginManager::instance().getCallMediaHandlerStatus();
}

plugin::MediaHandlerDetails
PluginModel::getCallMediaHandlerDetails(const QString& id)
{
    if (id.isEmpty()) {
        return plugin::MediaHandlerDetails();
    }
    MapStringString mediaHandlerDetails = PluginManager::instance().getCallMediaHandlerDetails(id);
    plugin::MediaHandlerDetails result;
    if (!mediaHandlerDetails.empty()) {
        result.id = id;
        result.iconPath = mediaHandlerDetails["iconPath"];
        result.name = mediaHandlerDetails["name"];
        result.pluginId = mediaHandlerDetails["pluginId"];
    }

    return result;
}

VectorMapStringString
PluginModel::getPluginPreferences(const QString& path)
{
    return PluginManager::instance().getPluginPreferences(path);
}

bool
PluginModel::setPluginPreference(const QString& path, const QString& key, const QString& value)
{
    return PluginManager::instance().setPluginPreference(path, key, value);
}

MapStringString
PluginModel::getPluginPreferencesValues(const QString& path)
{
    return PluginManager::instance().getPluginPreferencesValues(path);
}

bool
PluginModel::resetPluginPreferencesValues(const QString& path)
{
    return PluginManager::instance().resetPluginPreferencesValues(path);
}

bool
PluginModel::addValueToPreference(const QString& pluginId,
                                  const QString& preferenceKey,
                                  const QString& value)
{
    return PluginManager::instance().addValueToPreference(pluginId, preferenceKey, value);
}

} // namespace lrc

#include "api/moc_pluginmodel.cpp"
#include "pluginmodel.moc"
