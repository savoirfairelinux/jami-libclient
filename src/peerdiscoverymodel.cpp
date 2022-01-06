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
#include "api/peerdiscoverymodel.h"

// new LRC
#include "callbackshandler.h"

// Dbus
#include "dbus/configurationmanager.h"

namespace lrc {

using namespace api;

class PeerDiscoveryModelPimpl : public QObject
{
    Q_OBJECT
public:
    PeerDiscoveryModelPimpl(PeerDiscoveryModel& linked,
                            const CallbacksHandler& callbackHandler,
                            const QString& accountID);
    ~PeerDiscoveryModelPimpl();

    PeerDiscoveryModel& linked_;
    const CallbacksHandler& callbacksHandler_;
    const QString accountID_;

public Q_SLOTS:

    /**
     * Emit peerMapStatusChanged.
     * @param accountId
     * @param status
     */
    void slotPeerMapStatusChanged(const QString& accountID,
                                  const QString& contactUri,
                                  int state,
                                  const QString& displayname);
};

PeerDiscoveryModel::PeerDiscoveryModel(const CallbacksHandler& callbacksHandler,
                                       const QString& accountID)
    : QObject()
    , pimpl_(std::make_unique<PeerDiscoveryModelPimpl>(*this, callbacksHandler, accountID))
{}

PeerDiscoveryModel::~PeerDiscoveryModel() {}

PeerDiscoveryModelPimpl::PeerDiscoveryModelPimpl(PeerDiscoveryModel& linked,
                                                 const CallbacksHandler& callbacksHandler,
                                                 const QString& accountID)
    : linked_(linked)
    , callbacksHandler_(callbacksHandler)
    , accountID_(accountID)
{
    connect(&callbacksHandler_,
            &CallbacksHandler::newPeerSubscription,
            this,
            &PeerDiscoveryModelPimpl::slotPeerMapStatusChanged);
}

PeerDiscoveryModelPimpl::~PeerDiscoveryModelPimpl()
{
    disconnect(&callbacksHandler_,
               &CallbacksHandler::newPeerSubscription,
               this,
               &PeerDiscoveryModelPimpl::slotPeerMapStatusChanged);
}

void
PeerDiscoveryModelPimpl::slotPeerMapStatusChanged(const QString& accountID,
                                                  const QString& contactUri,
                                                  int state,
                                                  const QString& displayname)
{
    if (accountID != accountID_) {
        return;
    }
    emit linked_.modelChanged(contactUri,
                              state == 0 ? PeerModelChanged::INSERT : PeerModelChanged::REMOVE,
                              displayname);
}

std::vector<PeerContact>
PeerDiscoveryModel::getNearbyPeers() const
{
    std::vector<PeerContact> result;
    const MapStringString nearbyPeers = ConfigurationManager::instance().getNearbyPeers(
        pimpl_->accountID_);
    result.reserve(nearbyPeers.size());

    QMap<QString, QString>::const_iterator i = nearbyPeers.constBegin();
    while (i != nearbyPeers.constEnd()) {
        result.emplace_back(PeerContact {i.key().toStdString(), i.value().toStdString()});
        ++i;
    }
    return result;
}

} // namespace lrc

#include "api/moc_peerdiscoverymodel.cpp"
#include "peerdiscoverymodel.moc"
