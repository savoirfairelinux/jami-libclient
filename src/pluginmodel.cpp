/**
 *    Copyright (C) 2020-2022 Savoir-faire Linux Inc.
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
    if (!enable)
        Q_EMIT chatHandlerStatusUpdated(false);
    else
        Q_EMIT chatHandlerStatusUpdated(getChatHandlers().size() > 0);

    Q_EMIT modelUpdated();
}

bool
PluginModel::getPluginsEnabled() const
{
    return PluginManager::instance().getPluginsEnabled();
}

VectorString
PluginModel::getInstalledPlugins() const
{
    return VectorString::fromList(PluginManager::instance().getInstalledPlugins());
}

VectorString
PluginModel::getLoadedPlugins() const
{
    return VectorString::fromList(PluginManager::instance().getLoadedPlugins());
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

    VectorString loadedPlugins = getLoadedPlugins();
    if (std::find(loadedPlugins.begin(), loadedPlugins.end(), result.path) != loadedPlugins.end()) {
        result.loaded = true;
    }

    return result;
}

bool
PluginModel::installPlugin(const QString& jplPath, bool force)
{
    if (getPluginsEnabled()) {
        auto result = PluginManager::instance().installPlugin(jplPath, force);
        Q_EMIT modelUpdated();
        return result;
    }
    return false;
}

bool
PluginModel::uninstallPlugin(const QString& rootPath)
{
    auto result = PluginManager::instance().uninstallPlugin(rootPath);
    Q_EMIT modelUpdated();
    return result;
}

bool
PluginModel::loadPlugin(const QString& path)
{
    bool status = PluginManager::instance().loadPlugin(path);
    Q_EMIT modelUpdated();
    if (getChatHandlers().size() > 0)
        emit chatHandlerStatusUpdated(getPluginsEnabled());
    return status;
}

bool
PluginModel::unloadPlugin(const QString& path)
{
    bool status = PluginManager::instance().unloadPlugin(path);
    Q_EMIT modelUpdated();
    if (getChatHandlers().size() <= 0)
        emit chatHandlerStatusUpdated(false);
    return status;
}

VectorString
PluginModel::getCallMediaHandlers() const
{
    return VectorString::fromList(PluginManager::instance().getCallMediaHandlers());
}

void
PluginModel::toggleCallMediaHandler(const QString& mediaHandlerId,
                                    const QString& callId,
                                    bool toggle)
{
    PluginManager::instance().toggleCallMediaHandler(mediaHandlerId, callId, toggle);
    Q_EMIT modelUpdated();
}

VectorString
PluginModel::getChatHandlers() const
{
    return VectorString::fromList(PluginManager::instance().getChatHandlers());
}

void
PluginModel::toggleChatHandler(const QString& chatHandlerId,
                               const QString& accountId,
                               const QString& peerId,
                               bool toggle)
{
    PluginManager::instance().toggleChatHandler(chatHandlerId, accountId, peerId, toggle);
    Q_EMIT modelUpdated();
}

VectorString
PluginModel::getCallMediaHandlerStatus(const QString& callId)
{
    return VectorString::fromList(PluginManager::instance().getCallMediaHandlerStatus(callId));
}

plugin::PluginHandlerDetails
PluginModel::getCallMediaHandlerDetails(const QString& mediaHandlerId)
{
    if (mediaHandlerId.isEmpty()) {
        return plugin::PluginHandlerDetails();
    }
    MapStringString mediaHandlerDetails = PluginManager::instance().getCallMediaHandlerDetails(
        mediaHandlerId);
    plugin::PluginHandlerDetails result;
    if (!mediaHandlerDetails.empty()) {
        result.id = mediaHandlerId;
        result.iconPath = mediaHandlerDetails["iconPath"];
        result.name = mediaHandlerDetails["name"];
        result.pluginId = mediaHandlerDetails["pluginId"];
    }

    return result;
}

VectorString
PluginModel::getChatHandlerStatus(const QString& accountId, const QString& peerId)
{
    return VectorString::fromList(PluginManager::instance().getChatHandlerStatus(accountId, peerId));
}

plugin::PluginHandlerDetails
PluginModel::getChatHandlerDetails(const QString& chatHandlerId)
{
    if (chatHandlerId.isEmpty()) {
        return plugin::PluginHandlerDetails();
    }
    MapStringString chatHandlerDetails = PluginManager::instance().getChatHandlerDetails(
        chatHandlerId);
    plugin::PluginHandlerDetails result;
    if (!chatHandlerDetails.empty()) {
        result.id = chatHandlerId;
        result.iconPath = chatHandlerDetails["iconPath"];
        result.name = chatHandlerDetails["name"];
        result.pluginId = chatHandlerDetails["pluginId"];
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
    auto result = PluginManager::instance().setPluginPreference(path, key, value);
    Q_EMIT modelUpdated();
    return result;
}

MapStringString
PluginModel::getPluginPreferencesValues(const QString& path)
{
    return PluginManager::instance().getPluginPreferencesValues(path);
}

bool
PluginModel::resetPluginPreferencesValues(const QString& path)
{
    auto result = PluginManager::instance().resetPluginPreferencesValues(path);
    Q_EMIT modelUpdated();
    return result;
}

} // namespace lrc

#include "api/moc_pluginmodel.cpp"
#include "pluginmodel.moc"
