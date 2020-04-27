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

#include "api/element.h"

#include "dbus/presencemanager.h"

namespace lrc
{

namespace api
{


class PeerDiscoveryPimpl : public QObject
{
    Q_OBJECT
public:
    PeerDiscoveryPimpl(const QString& owner);
    ~PeerDiscoveryPimpl();

public Q_SLOTS:
    /**
     * Emit peerMapStatusChanged.
     * @param accountId
     * @param status
     */
    void slotPeerMapStatusChanged(const QString& accountID, const QString& contactUri, int state, const QString& displayname);

public:
    std::mutex elementsMutex_;
    QMap<QString, Element> elements;
    QString accountId_;
};

PeerDiscoveryPimpl::PeerDiscoveryPimpl(const QString& owner) : accountId_(owner)
{
    connect(&PresenceManager::instance(),
            &PresenceManagerInterface::nearbyPeerNotification,
            this,
            &PeerDiscoveryPimpl::slotPeerMapStatusChanged,
            Qt::QueuedConnection);
}

PeerDiscoveryPimpl::~PeerDiscoveryPimpl()
{
    disconnect(&PresenceManager::instance(),
               &PresenceManagerInterface::nearbyPeerNotification,
               this,
               &PeerDiscoveryPimpl::slotPeerMapStatusChanged);
}

void
PeerDiscoveryPimpl::slotPeerMapStatusChanged(const QString& accountId, const QString& contactUri, int state, const QString& displayname)
{
    if (accountId != accountId_) return;
    qWarning() << "@@@ slotPeerMapStatusChanged: " << contactUri << " " << state << " " << displayname;
    std::lock_guard<std::mutex> lk(elementsMutex_);
    if (state == 0) {
        elements.insert(contactUri, {
            contactUri,
            "" /* avatar */,
            displayname /* title */,
            contactUri /* subtitle */,
            "",
            0
        });
    } else {
        elements.remove(contactUri);
    }
    // TODO emit parent collectionChanged
}

PeerDiscovery::PeerDiscovery(const QString& owner)
: Collection(owner), pimpl_ {std::make_unique<PeerDiscoveryPimpl>(owner)}
{

}

PeerDiscovery::~PeerDiscovery()
{

}

QVector<Element>
PeerDiscovery::filter(const QString& search)
{
    std::lock_guard<std::mutex> lk(pimpl_->elementsMutex_);
    QVector<Element> res;
    for (auto k: pimpl_->elements.keys()) {
        auto elem = pimpl_->elements.value(k);
        if (elem.uri.indexOf(search) != -1
        || elem.title.indexOf(search) != -1
        || elem.subtitle.indexOf(search) != -1) {
            res.push_back(elem);
        }
    }
    return res;
}

} // namespace api
} // namespace lrc

#include "api/collections/moc_peerDiscovery.cpp"
#include "peerDiscovery.moc"
