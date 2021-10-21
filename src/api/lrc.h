/****************************************************************************
 *    Copyright (C) 2017-2021 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
 *   Author : Aline Gondim Santos <aline.gondimsantos@savoirfairelinux.com> *
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

#include "typedefs.h"

#include <memory>
#include <vector>
#include <atomic>

namespace lrc {

class LrcPimpl;

namespace api {

class BehaviorController;
class NewAccountModel;
class DataTransferModel;
class AVModel;
class PluginModel;

class LIB_EXPORT Lrc
{
public:
    /**
     * Construct an Lrc object and optionally invoke callbacks
     * to control ui informing the user of a possibly lengthy
     * migration process.
     * @param willMigrateCb
     * @param didMigrateCb
     */
    Lrc(MigrationCb willMigrateCb = {}, MigrationCb didMigrateCb = {}, bool muteDring = false);
    ~Lrc();
    /**
     * get a reference on account model.
     * @return a NewAccountModel&.
     */
    NewAccountModel& getAccountModel() const;
    /**
     * get a reference on the behavior controller.
     * @return a BehaviorController&.
     */
    BehaviorController& getBehaviorController() const;
    /**
     * get a reference on the audio-video controller.
     * @return a AVModel&.
     */
    AVModel& getAVModel() const;

    /**
     * get a reference on the PLUGIN controller.
     * @return a PluginModel&.
     */
    PluginModel& getPluginModel() const;

    /**
     * Inform the daemon that the connectivity changed
     */
    void connectivityChanged() const;

    /**
     * Test connection with daemon
     */
    static bool isConnected();
    /**
     * Can communicate with the daemon via dbus
     */
    static bool dbusIsValid();
    /**
     * Connect to debugMessageReceived signal
     */
    void subscribeToDebugReceived();

    /**
     * Helper: get active call list from daemon
     */
    static VectorString activeCalls();

    /**
     * Close all active calls and conferences
     */
    void hangupCallsAndConferences();

    /**
     * Helper: get call list from daemon
     */
    static VectorString getCalls();

    /**
     * Helper: get conference list from daemon
     */
    static VectorString getConferences(const QString& accountId = "");

    /**
     * Preference
     */
    static std::atomic_bool holdConferences;

    /**
     * Make monitor continous or discrete
     */
    static void monitor(bool continous);

private:
    std::unique_ptr<LrcPimpl> lrcPimpl_;
};

} // namespace api
} // namespace lrc
