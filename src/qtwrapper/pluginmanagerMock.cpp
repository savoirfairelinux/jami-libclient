/******************************************************************************
 *    Copyright (C) 2014-2022 Savoir-faire Linux Inc.                         *
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

#include "pluginmanager_wrap.h"

bool
PluginManagerInterface::loadPlugin(const QString& path)
{
    return false;
}

bool
PluginManagerInterface::unloadPlugin(const QString& path)
{
    return false;
}

MapStringString
PluginManagerInterface::getPluginDetails(const QString& path)
{
    return {};
}

QStringList
PluginManagerInterface::getInstalledPlugins()
{
    return {};
}

QStringList
PluginManagerInterface::getLoadedPlugins()
{
    return {};
}

int
PluginManagerInterface::installPlugin(const QString& jplPath, bool force)
{
    return 0;
}

int
PluginManagerInterface::uninstallPlugin(const QString& pluginRootPath)
{
    return 0;
}

QStringList
PluginManagerInterface::getCallMediaHandlers()
{
    return {};
}

void
PluginManagerInterface::toggleCallMediaHandler(const QString& mediaHandlerId,
                                               const QString& callId,
                                               bool toggle)
{}

QStringList
PluginManagerInterface::getChatHandlers()
{
    return {};
}

void
PluginManagerInterface::toggleChatHandler(const QString& chatHandlerId,
                                          const QString& accountId,
                                          const QString& peerId,
                                          bool toggle)
{}

QStringList
PluginManagerInterface::getCallMediaHandlerStatus(const QString& callId)
{
    return {};
}

MapStringString
PluginManagerInterface::getCallMediaHandlerDetails(const QString& mediaHandlerId)
{
    return {};
}

QStringList
PluginManagerInterface::getChatHandlerStatus(const QString& accountId, const QString& peerId)
{
    return {};
}

MapStringString
PluginManagerInterface::getChatHandlerDetails(const QString& chatHandlerId)
{
    return {};
}

void
PluginManagerInterface::setPluginsEnabled(bool enable)
{}

bool
PluginManagerInterface::getPluginsEnabled()
{
    return false;
}

VectorMapStringString
PluginManagerInterface::getPluginPreferences(const QString& path)
{
    return {};
}

bool
PluginManagerInterface::setPluginPreference(const QString& path,
                                            const QString& key,
                                            const QString& value)
{
    return false;
}

MapStringString
PluginManagerInterface::getPluginPreferencesValues(const QString& path)
{
    return {};
}

bool
PluginManagerInterface::resetPluginPreferencesValues(const QString& path)
{
    return false;
}
