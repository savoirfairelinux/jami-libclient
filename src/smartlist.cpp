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
#include "api/collections/temporaryItem.h"
#include "api/collections/peerDiscovery.h"
#include "api/element.h"

namespace lrc
{

namespace api
{

class SmartListPimpl
{
public:
    SmartListPimpl();

    QMap<QString, QMap<QString, std::shared_ptr<Collection>>> collections_;
};

SmartListPimpl::SmartListPimpl()
{
    QMap<QString, std::shared_ptr<Collection>> contactsCollections;
    contactsCollections.insert("Search result", std::make_shared<TemporaryItem>()); 
    contactsCollections.insert("Local contacts", std::make_shared<PeerDiscovery>()); 
    collections_.insert("Contacts", contactsCollections);
    QMap<QString, std::shared_ptr<Collection>> conversationsCollections;
    collections_.insert("Conversations", conversationsCollections);
}

SmartList::SmartList()
: QObject()
, pimpl_(std::make_unique<SmartListPimpl>())
{
    
}

SmartList::~SmartList()
{
    
}

QMap<QString, QMap<QString, std::shared_ptr<Collection>>>
SmartList::list() const
{
    return {};   
}

FilteredCollections
SmartList::filter(const QString& category, const QString& filter)
{
    return {};
}

} // namespace api
} // namespace lrc

#include "api/moc_smartlist.cpp"
#include "smartlist.moc"
