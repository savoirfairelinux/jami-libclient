/****************************************************************************
 *    Copyright (C) 2019-2022 Savoir-faire Linux Inc.                       *
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

// Lrc
#include "typedefs.h"

// Qt
#include <QObject>

// std
#include <vector>
#include <map>
#include <memory>

namespace lrc {

class CallbacksHandler;
class PeerDiscoveryModelPimpl;

namespace api {

struct PeerContact
{
    std::string uri;
    std::string displayName;
};

enum class PeerModelChanged { INSERT, REMOVE };

/**
 *  @brief Class that manages local peer discovery info
 */
class LIB_EXPORT PeerDiscoveryModel : public QObject
{
    Q_OBJECT
public:
    PeerDiscoveryModel(const CallbacksHandler& callbackHandler, const QString& accountID);
    ~PeerDiscoveryModel();
    /**
     * get a map of discovered peers account
     * @return a std::vector<PeerContact>
     */
    std::vector<PeerContact> getNearbyPeers() const;

Q_SIGNALS:
    /**
     * Connect this signal to know when the status of local peer discovery map changed.
     */
    void modelChanged(const QString& contactUri, PeerModelChanged state, const QString& displayname);

private:
    std::unique_ptr<PeerDiscoveryModelPimpl> pimpl_;
};
} // namespace api
} // namespace lrc
Q_DECLARE_METATYPE(lrc::api::PeerDiscoveryModel*)
