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

class PluginModelPimpl;

namespace api
{

namespace plugin
{
    /**
     * This class describes current plugin Details
     */
    struct Details
    {
        QString name = "";
        QString path = "";
        QString iconPath = "";
        bool loaded = false;
    };

    struct MediaHandlerDetails
    {
        QString id = "";
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
    void setPluginsEnabled(bool enable);

    /**
     * Get if plugins are enabled
     * @return plugins enabled
     */
    bool getPluginsEnabled() const;

    /**
     * Get list of installed plugins
     * @return plugins installed
     */
    VectorString listAvailablePlugins() const;
    
    /**
     * Get list of loaded plugins
     * @return plugins loaded
     */
    VectorString listLoadedPlugins() const;

    /**
     * Get details of installed plugin
     * @return plugin Details
     */
    plugin::Details getPluginDetails(const QString& path);

    /**
     * Install plugin
     * @return plugin installed
     */
    bool installPlugin(const QString& jplPath, bool force);

    /**
     * Uninstall plugin
     * @return plugin uninstalled
     */
    bool uninstallPlugin(const QString& rootPath);

    /**
     * Load plugin
     * @return plugin loaded
     */
    bool loadPlugin(const QString& path);

    /**
     * Unload plugin
     * @return plugin unloaded
     */
    bool unloadPlugin(const QString& path);

    /**
     * List available Media Handlers
     * @return List of available Media Handlers
     */
    VectorString listCallMediaHandlers() const;

    /**
     * Toggle media handler
     */
    void toggleCallMediaHandler(const QString& id, bool toggle);

    /**
     * Get details of available media handler
     * @return Media Handler Details
     */
    plugin::MediaHandlerDetails getCallMediaHandlerDetails(const QString& id);

// Q_SIGNALS:
private:
    std::unique_ptr<PluginModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
