/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
 *   Author: Mingrui Zhang <mingrui.zhang@savoirfairelinux.com>             *
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
#include <vector>
#include <map>
#include <memory>
#include <string>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"

namespace lrc
{

class CallbacksHandler;
class PeerDiscoveryModelPimpl;

namespace api
{

/**
  *  @brief Class that manages local peer discovery info
  */
class LIB_EXPORT PeerDiscoveryModel : public QObject {
    Q_OBJECT
public:

    PeerDiscoveryModel(const CallbacksHandler& callbackHandler);

    ~PeerDiscoveryModel();
    /**
     * get a map of discovered peers account
     * @return a std::map<std::string, std::string>
     */
    MapStringString getNearbyPeers(const QString &accountID) const;

Q_SIGNALS:
    /**
     * Connect this signal to know when the status of local peer discovery map changed.
     */
    void peerMapStatusChanged(const std::string& accountID, const std::string& contactUri, int state, const std::string& displayname);

private:
    std::unique_ptr<PeerDiscoveryModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
