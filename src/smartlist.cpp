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
#include "api/smartlist.h"
#include "api/collection.h"
#include "api/collections/searchResultList.h"
#include "api/collections/peerDiscovery.h"
#include "api/collections/trustRequestList.h"
#include "api/collections/conversationList.h"
#include "api/element.h"

namespace lrc
{

namespace api
{

class SmartListPimpl
{
public:
    SmartListPimpl(const QString& owner);

    QMap<QString, QMap<QString, std::shared_ptr<Collection>>> collections_;
    QString accountId_;
};

SmartListPimpl::SmartListPimpl(const QString& owner)
: accountId_(owner)
{
    qWarning() << "Create smartlist for " << owner;
    // TODO move to account
    QMap<QString, std::shared_ptr<Collection>> contactsCollections;
    contactsCollections.insert("Contact requests", std::make_shared<TrustRequestList>(owner));
    contactsCollections.insert("Local peers", std::make_shared<PeerDiscovery>(owner));
    collections_.insert("Contacts", contactsCollections);
    QMap<QString, std::shared_ptr<Collection>> conversationsCollections;
    conversationsCollections.insert("Search result", std::make_shared<SearchResultList>(owner));
    conversationsCollections.insert("Conversations", std::make_shared<ConversationList>(owner));
    collections_.insert("Conversations", conversationsCollections);
}

SmartList::SmartList(const QString& owner)
: QObject()
, pimpl_(std::make_unique<SmartListPimpl>(owner))
{

}

SmartList::~SmartList()
{

}

QMap<QString, QMap<QString, std::shared_ptr<Collection>>>
SmartList::list() const
{
    return pimpl_->collections_;
}

void
SmartList::filter(const QString& category, const QString& filter)
{
    // TODO lock
    QMapIterator<QString, QMap<QString, std::shared_ptr<Collection>>> it(pimpl_->collections_);
    while (it.hasNext()) {
        if (category.isEmpty() or category == it.key()) {
            QMapIterator<QString, std::shared_ptr<Collection>> itCollection(it.value());
            while (itCollection.hasNext()) {
                if (itCollection.value())
                    itCollection.value()->filter(filter);
            }
        }
    }
}

} // namespace api
} // namespace lrc

#include "api/moc_smartlist.cpp"
#include "smartlist.moc"
