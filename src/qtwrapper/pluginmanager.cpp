/******************************************************************************
 *    Copyright (C) 2014-2021 Savoir-faire Linux Inc.                         *
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

#include "pluginmanager_wrap.h"

bool
PluginManagerInterface::loadPlugin(const QString& path)
{
    return DRing::loadPlugin(path.toStdString());
}

bool
PluginManagerInterface::unloadPlugin(const QString& path)
{
    return DRing::unloadPlugin(path.toStdString());
}

MapStringString
PluginManagerInterface::getPluginDetails(const QString& path)
{
    return convertMap(DRing::getPluginDetails(path.toStdString()));
}

QStringList
PluginManagerInterface::getInstalledPlugins()
{
    return convertStringList(DRing::getInstalledPlugins());
}

QStringList
PluginManagerInterface::getLoadedPlugins()
{
    return convertStringList(DRing::getLoadedPlugins());
}

int
PluginManagerInterface::installPlugin(const QString& jplPath, bool force)
{
    return DRing::installPlugin(jplPath.toStdString(), force);
}

int
PluginManagerInterface::uninstallPlugin(const QString& pluginRootPath)
{
    return DRing::uninstallPlugin(pluginRootPath.toStdString());
}

QStringList
PluginManagerInterface::getCallMediaHandlers()
{
    return convertStringList(DRing::getCallMediaHandlers());
}

void
PluginManagerInterface::toggleCallMediaHandler(const QString& mediaHandlerId,
                                               const QString& callId,
                                               bool toggle)
{
    DRing::toggleCallMediaHandler(mediaHandlerId.toStdString(), callId.toStdString(), toggle);
}

QStringList
PluginManagerInterface::getChatHandlers()
{
    return convertStringList(DRing::getChatHandlers());
}

void
PluginManagerInterface::toggleChatHandler(const QString& chatHandlerId,
                                          const QString& accountId,
                                          const QString& peerId,
                                          bool toggle)
{
    DRing::toggleChatHandler(chatHandlerId.toStdString(),
                             accountId.toStdString(),
                             peerId.toStdString(),
                             toggle);
}

QStringList
PluginManagerInterface::getCallMediaHandlerStatus(const QString& callId)
{
    return convertStringList(DRing::getCallMediaHandlerStatus(callId.toStdString()));
}

MapStringString
PluginManagerInterface::getCallMediaHandlerDetails(const QString& mediaHandlerId)
{
    return convertMap(DRing::getCallMediaHandlerDetails(mediaHandlerId.toStdString()));
}

QStringList
PluginManagerInterface::getChatHandlerStatus(const QString& accountId, const QString& peerId)
{
    return convertStringList(
        DRing::getChatHandlerStatus(accountId.toStdString(), peerId.toStdString()));
}

MapStringString
PluginManagerInterface::getChatHandlerDetails(const QString& chatHandlerId)
{
    return convertMap(DRing::getChatHandlerDetails(chatHandlerId.toStdString()));
}

void
PluginManagerInterface::setPluginsEnabled(bool enable)
{
    DRing::setPluginsEnabled(enable);
}

bool
PluginManagerInterface::getPluginsEnabled()
{
    return DRing::getPluginsEnabled();
}

VectorMapStringString
PluginManagerInterface::getPluginPreferences(const QString& path)
{
    VectorMapStringString temp;
    for (auto x : DRing::getPluginPreferences(path.toStdString())) {
        temp.push_back(convertMap(x));
    }
    return temp;
}

bool
PluginManagerInterface::setPluginPreference(const QString& path,
                                            const QString& key,
                                            const QString& value)
{
    return DRing::setPluginPreference(path.toStdString(), key.toStdString(), value.toStdString());
}

MapStringString
PluginManagerInterface::getPluginPreferencesValues(const QString& path)
{
    return convertMap(DRing::getPluginPreferencesValues(path.toStdString()));
}

bool
PluginManagerInterface::resetPluginPreferencesValues(const QString& path)
{
    return DRing::resetPluginPreferencesValues(path.toStdString());
}
