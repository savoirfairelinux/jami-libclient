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
#include "api/peerdiscoverymodel.h"

// new LRC
#include "api/lrc.h"
#include "callbackshandler.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc
{

using namespace api;

class PeerDiscoveryModelPimpl: public QObject
{
    Q_OBJECT
public:
    PeerDiscoveryModelPimpl(PeerDiscoveryModel& linked,
                            Lrc& lrc,
                            const CallbacksHandler& callbackHandler,
                            const QString &accountID);
    ~PeerDiscoveryModelPimpl();

    PeerDiscoveryModel& linked;
    Lrc& lrc;
    const CallbacksHandler& callbacksHandler;
    const QString &accountID;

public Q_SLOTS:

    /**
     * Emit peerMapStatusChanged.
     * @param accountId
     * @param status
     */
    void slotPeerMapStatusChanged(const std::string& accountID, const std::string& contactUri, int state, const std::string& displayname);
};

PeerDiscoveryModel::PeerDiscoveryModel(Lrc& lrc,
                                       const CallbacksHandler& callbacksHandler,
                                       const QString &accountID)
: QObject()
, pimpl_(std::make_unique<PeerDiscoveryModelPimpl>(*this, lrc, callbacksHandler, accountID))
{
}

PeerDiscoveryModel::~PeerDiscoveryModel()
{
}

PeerDiscoveryModelPimpl::PeerDiscoveryModelPimpl(PeerDiscoveryModel& linked,
                                                 Lrc& lrc,
                                                 const CallbacksHandler& callbacksHandler,
                                                 const QString &accountID)
: linked(linked)
, lrc {lrc}
, callbacksHandler(callbacksHandler)
, accountID(accountID)
{
    connect(&callbacksHandler, &CallbacksHandler::newPeerSubscription, this, &PeerDiscoveryModelPimpl::slotPeerMapStatusChanged);
}

PeerDiscoveryModelPimpl::~PeerDiscoveryModelPimpl()
{
}

void
PeerDiscoveryModelPimpl::slotPeerMapStatusChanged(const std::string& accountID, const std::string& contactUri, int state, const std::string& displayname)
{
    emit linked.peerMapStatusChanged(accountID,contactUri,state,displayname);
}

MapStringString
PeerDiscoveryModel::getNearbyPeers() const
{
    return ConfigurationManager::instance().getNearbyPeers(pimpl_->accountID);
}

} // namespace lrc

#include "api/moc_peerdiscoverymodel.cpp"
#include "peerdiscoverymodel.moc"
