/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
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

// Parent
#include "smartlistmodel.h"

// Debug
#include <qdebug.h>

// Lrc mettre des ""
#include <contactitem.h>
#include <accountmodel.h>
#include <contactmethod.h>
#include <availableaccountmodel.h>
#include "database.h"
#include "dbus/callmanager.h"

// Std
#include <iterator>
#include <algorithm>
#include <regex>

SmartListModel::SmartListModel(QObject* parent)
{
    auto fillsWithContacts = [&] (Account* a) {
        if (not a) {
            qDebug() << "no available account selected";
            return a;
        }

        auto contacts = a->getContacts();

        // clear the list
        items.clear();

        // add contacts to the list
        for (auto c : contacts) {
            auto contact = std::shared_ptr<ContactItem>(new ContactItem(c));
            contact->setTitle(c->uri().toUtf8().constData());
            items.push_back(contact);
        }

        return a;
    };

    connect(&AvailableAccountModel::instance(), &AvailableAccountModel::currentDefaultAccountChanged,
    [this, fillsWithContacts](Account* a)
    {
        if (fillsWithContacts(a))
            emit modelUpdated();
    });


    connect(&CallManager::instance(), &CallManagerInterface::incomingCall,
    [this](const QString &accountID, const QString &callID, const QString &fromQString)
    {
        auto from = fromQString.toStdString();

        // during a call we receiving something like :
        // "gargouille <6f42876966f3eb12c5ad33c33398e0fb22c6cea4@ring.dht>"
        // we trim to get only the ringid
        from.erase(0, from.find('<')+1);
        from.erase(from.find('@'));

        unsigned int row = 0;

        for (auto iter = items.begin(); iter != items.end(); iter++) {
            auto conversation = std::dynamic_pointer_cast<ContactItem>(*iter);

            if(not conversation) {
                qDebug() << "incomingCall, but no conversation found";
                continue;
            }

            if (conversation->getUri() == from) {
                conversation->setCallId(callID.toInt());
                emit conversationItemUpdated(std::distance(items.begin(), iter));
            }
        }
    });

    // initialise the list
    fillsWithContacts(AvailableAccountModel::instance().currentDefaultAccount());


    // test only
    DataBase::instance();
}

SmartListModel::~SmartListModel()
{
}

SmartListItems
SmartListModel::getItems() const
{
    if (m_sFilter.length() == 0) return items;

    SmartListItems filteredItems(items.size());
    auto filter = m_sFilter;
    auto it = std::copy_if(items.begin(), items.end(), filteredItems.begin(),
    [&filter] (std::shared_ptr<SmartListItem> item) {
        try {
            // TODO filter by UID and not by title?
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(item->getTitle(), regexFilter)
            | std::regex_search(item->getAlias(), regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring of the title or the alias.
            return item->getTitle().find(filter) != std::string::npos
            && item->getAlias().find(filter) != std::string::npos;
        }
    });
    filteredItems.resize(std::distance(filteredItems.begin(), it));
    return filteredItems;
}

SmartListModel&
SmartListModel::instance()
{
    static auto instance = new SmartListModel(QCoreApplication::instance());
    return *instance;
}

std::shared_ptr<SmartListItem>
SmartListModel::getItem(int row){
    return items[row];
}

int
SmartListModel::find(const std::string& uid) const
{
    for (unsigned int i = 0 ; i < items.size() ; ++i) {
        if (items[i]->getTitle() == uid) { // TODO get UID
            return i;
        }
    }
    return -1;
}

void
SmartListModel::openConversation(const std::string& uid) const
{
    auto i = find(uid);
    if (i != -1) items[i]->activate();
    else {
        // TODO open temporary item
    }
}

void
SmartListModel::setFilter(const std::string& newFilter)
{
    m_sFilter = newFilter;
    emit modelUpdated();
}


std::string
SmartListModel::getFilter() const
{
    return m_sFilter;
}


#include <smartlistmodel.moc>
