/****************************************************************************
 *    Copyright (C) 2018-2020 Savoir-faire Linux Inc.                       *
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
#include "api/collections/peerDiscovery.h"

#include "api/element.h"

#include "dbus/presencemanager.h"

namespace lrc
{

namespace api
{


class PeerDiscoveryPimpl
{
public:
    PeerDiscoveryPimpl();

public Q_SLOTS:
    /**
     * Emit peerMapStatusChanged.
     * @param accountId
     * @param status
     */
    void slotPeerMapStatusChanged(const QString& accountID, const QString& contactUri, int state, const QString& displayname);
};

PeerDiscoveryPimpl::PeerDiscoveryPimpl()
{
    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::nearbyPeerNotification,
            this,
            &PeerDiscoveryModelPimpl::slotPeerMapStatusChanged,
            Qt::QueuedConnection);
}

void
PeerDiscoveryModelPimpl::slotPeerMapStatusChanged(const QString& accountID, const QString& contactUri, int state, const QString& displayname)
{
    // TODO linked peer Id
}

PeerDiscovery::PeerDiscovery()
{
    
}

PeerDiscovery::~PeerDiscovery()
{
    
}

QVector<Element>
PeerDiscovery::filter(const QString& searchr)
{
    return {};
}

} // namespace api
} // namespace lrc

#include "api/collections/moc_peerDiscovery.cpp"
#include "peerDiscovery.moc"
