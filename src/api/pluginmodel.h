/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                                  *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// Q_SIGNALS:
private:
    std::unique_ptr<PluginModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
