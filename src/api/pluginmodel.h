/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                       *
 *   Author: Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com>  *
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
#pragma once

// std
#include <memory>
#include <string>
#include <vector>

// Qt
#include <qobject.h>

// LRC
#include "typedefs.h"

namespace lrc
{

namespace api
{

namespace plugin
{
    /**
     * This class describes current plugin Details
     */
    struct PluginDetails
    {
        QString name = "";
        QString path = "";
        QString iconPath = "";
        bool loaded = false;
    };

    struct MediaHandlerDetails
    {
        QString id = "";
        QString name = "";
        QString iconPath = "";
    };
}

class LIB_EXPORT PluginModel : public QObject {
    Q_OBJECT
public:
    PluginModel();
    ~PluginModel();

    /**
     * Enable/disable plugins
     * @param if plugin enabled
     */
    Q_INVOKABLE void setPluginsEnabled(bool enable);

    /**
     * Get if plugins are enabled
     * @return plugins enabled
     */
    Q_INVOKABLE bool getPluginsEnabled() const;

    /**
     * Get list of installed plugins
     * @return plugins installed
     */
    Q_INVOKABLE VectorString listAvailablePlugins() const;

    /**
     * Get list of loaded plugins
     * @return plugins loaded
     */
    Q_INVOKABLE VectorString listLoadedPlugins() const;

    /**
     * Get details of installed plugin
     * @return plugin Details
     */
    Q_INVOKABLE plugin::PluginDetails getPluginDetails(const QString& path);

    /**
     * Install plugin
     * @return plugin installed
     */
    Q_INVOKABLE bool installPlugin(const QString& jplPath, bool force);

    /**
     * Uninstall plugin
     * @return plugin uninstalled
     */
    Q_INVOKABLE bool uninstallPlugin(const QString& rootPath);

    /**
     * Load plugin
     * @return plugin loaded
     */
    Q_INVOKABLE bool loadPlugin(const QString& path);

    /**
     * Unload plugin
     * @return plugin unloaded
     */
    Q_INVOKABLE bool unloadPlugin(const QString& path);

    /**
     * List available Media Handlers
     * @return List of available Media Handlers
     */
    Q_INVOKABLE VectorString listCallMediaHandlers() const;

    /**
     * Toggle media handler
     */
    Q_INVOKABLE void toggleCallMediaHandler(const QString& id);

    /**
     * Verify if there is an active plugin media handler
     * @return Map with name and status
     */
    Q_INVOKABLE MapStringString getCallMediaHandlerStatus();

    /**
     * Get details of available media handler
     * @return Media Handler Details
     */
    Q_INVOKABLE plugin::MediaHandlerDetails getCallMediaHandlerDetails(const QString& id);
};

} // namespace api
} // namespace lrc
