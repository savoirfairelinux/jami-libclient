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
#include "newconversationitem.h"
#include "dbus/configurationmanager.h"
#include "globals.h"

// Std
#include <iterator>
#include <algorithm>
#include <regex>

SmartListModel::SmartListModel(QObject* parent)
{
    connect(&AvailableAccountModel::instance(), &AvailableAccountModel::currentDefaultAccountChanged,
    [this](Account* a)
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

        for (auto iter = items_.begin(); iter != items_.end(); iter++) {
            auto conversation = std::dynamic_pointer_cast<ContactItem>(*iter);

            if(not conversation) {
                qDebug() << "incomingCall, but no conversation found";
                continue;
            }
            if (conversation->getUri() == from) {
                conversation->setCallId(callID.toStdString());
                conversation->setCallStatus(CallStatus::INCOMING_RINGING);
                emit incomingCallFromItem(std::distance(items_.begin(), iter));
            }
        }
    });


    connect(&CallManager::instance(), &CallManagerInterface::callStateChanged, // A DEPLACER DANS UNE FONCTION CAR TROP GROS
    [this](const QString& callID, const QString& stateName, int code)
    {
        auto iter = std::find_if (items_.begin(), items_.end(),
        [&callID] (const std::shared_ptr<SmartListItem>& item)
        {
            // for now we use the dynamic cast...
            auto toto = std::dynamic_pointer_cast<ContactItem>(item);
            if (not toto)
                return false;
            return (toto->getCallId() == callID.toStdString());
        });

        if (iter == items_.end())
            return;

        auto contactItem = std::dynamic_pointer_cast<ContactItem>(*iter);

        CallStatus state = CallStatus::NONE;
        if (stateName == "INCOMING")
            state = CallStatus::INCOMING_RINGING;

        if (stateName == "CURRENT")
            state = CallStatus::IN_PROGRESS;

        if (stateName == "CONNECTING")
            state = CallStatus::CONNECTING;

        if (stateName == "INACTIVE")
            state = CallStatus::INACTIVE;

        if (stateName == "OVER")
            state = CallStatus::ENDED;

        if (stateName == "RINGING")
            state = CallStatus::OUTGOING_RINGING;

        if (stateName == "CONNECTING")
            state = CallStatus::SEARCHING;

        if (stateName == "HOLD") {
            emit callPauseModeChangedFor(contactItem.get());
            state = CallStatus::PAUSED;
        }

        if (stateName == "PEER_PAUSED")
            state = CallStatus::PEER_PAUSED;



        contactItem->setCallStatus(state);
    });

    // initialise the list
    fillsWithContacts(AvailableAccountModel::instance().currentDefaultAccount());


    // test only
    DataBase::instance();
    QObject::connect(&DataBase::instance(), &DataBase::messageAdded, this, &SmartListModel::slotNewMessageInDatabase);
}


SmartListModel::~SmartListModel()
{
}

SmartListItems
SmartListModel::getItems() const
{
    filteredItems_ = items_;

    if (filter_.length() == 0) return filteredItems_;

    auto filter = filter_;
    auto it = std::copy_if(items_.begin(), items_.end(), filteredItems_.begin(),
    [&filter, this] (const std::shared_ptr<SmartListItem>& item) {
        auto isTemporary = std::dynamic_pointer_cast<NewConversationItem>(item); //TODO wait for enum LRC side to get type
        if (isTemporary) return true;
        try {
            auto regexFilter = std::regex(filter, std::regex_constants::icase);
            bool result = std::regex_search(item->getTitle(), regexFilter)
            | std::regex_search(item->getUID(), regexFilter);
            return result;
        } catch(std::regex_error&) {
            // If the regex is incorrect, just test if filter is a substring of the title or the alias.
            return item->getTitle().find(filter) != std::string::npos
            && item->getUID().find(filter) != std::string::npos;
        }
    });
    filteredItems_.resize(std::distance(filteredItems_.begin(), it));
    return filteredItems_;
}

SmartListModel&
SmartListModel::instance()
{
    static auto instance = new SmartListModel(QCoreApplication::instance());
    return *instance;
}

std::shared_ptr<SmartListItem>
SmartListModel::getItem(int row){
    return filteredItems_[row];
}


int
SmartListModel::find(const std::string& uid) const
{
    for (unsigned int i = 0 ; i < items_.size() ; ++i) {
        if (items_[i]->getUID() == uid) { // TODO get UID
            return i;
        }
    }
    return -1;
}

int
SmartListModel::findFiltered(const std::string& uid) const
{
    for (unsigned int i = 0 ; i < filteredItems_.size() ; ++i) {
        if (filteredItems_[i]->getUID() == uid) { // TODO get UID
            return i;
        }
    }
    return -1;
}

void
SmartListModel::openConversation(const std::string& uid) const
{
    auto i = find(uid);
    if (i != -1) items_[i]->activate();
}

void
SmartListModel::removeConversation(const std::string& title)
{
    // Find item to remove
    auto idx = find(title);
    if (idx < 0) return;
    auto account = AvailableAccountModel::instance().currentDefaultAccount();
    if (!account) return;

    // Clear history with this contact
    DataBase::instance().removeHistory(QString(title.c_str()), QString(account->id()));

    // TODO define behavior for group chats.
    // Remove contact from Daemon
    ConfigurationManager::instance().removeContact(account->id(), QString(title.c_str()), false);

    // Remove item
    auto it = items_.begin();
    auto item = items_.at(idx);
    std::advance(it, idx);
    items_.erase(it);

    // The model has changed
    emit modelUpdated();
}


void
SmartListModel::setFilter(const std::string& newFilter)
{
    filter_ = newFilter;
    std::shared_ptr<NewConversationItem> newConversationItem;
    if (!newFilter.empty()) {
        // add the first item, wich is the NewConversationItem
        if (!items_.empty()) {
            auto isTemporary = std::dynamic_pointer_cast<NewConversationItem>(items_.front()); //TODO wait for enum LRC side to get type
            if (!isTemporary) {
                // No newConversationItem, create one
                newConversationItem = createNewConversationItem();
            } else {
                // The item already exists
                newConversationItem = std::dynamic_pointer_cast<NewConversationItem>(items_.front());
            }
        } else {
            newConversationItem = createNewConversationItem();
        }
        newConversationItem->search(newFilter);
    } else {
        // No filter, so we can remove the newConversationItem
        if (!items_.empty()) {
            auto isTemporary = std::dynamic_pointer_cast<NewConversationItem>(items_.front()); //TODO wait for enum LRC side to get type
            if (isTemporary) {
                removeNewConversationItem();
            }
        }
    }
    emit modelUpdated();
}

void
SmartListModel::contactFound(const std::string& id)
{
    auto idx = findFiltered(id);
    if (idx != -1) {
        removeNewConversationItem();
    }
    emit modelUpdated();
}

std::shared_ptr<NewConversationItem>
SmartListModel::createNewConversationItem()
{
    auto newConversationItem = std::shared_ptr<NewConversationItem>(new NewConversationItem());
    items_.emplace_front(newConversationItem);
    connect(newConversationItem.get(), &NewConversationItem::changed, this, &SmartListModel::slotItemChanged);
    connect(newConversationItem.get(), &NewConversationItem::contactFound, this, &SmartListModel::contactFound);
    connect(newConversationItem.get(), &NewConversationItem::contactAdded, this, &SmartListModel::contactAdded);
    connect(newConversationItem.get(), &NewConversationItem::contactAddedAndCall, this, &SmartListModel::contactAddedAndCall);
    connect(newConversationItem.get(), &NewConversationItem::contactAddedAndSend, this, &SmartListModel::contactAddedAndSend);
    return newConversationItem;
}


void
SmartListModel::removeNewConversationItem()
{
    auto newConversationItem = std::dynamic_pointer_cast<NewConversationItem>(items_.front());
    disconnect(newConversationItem.get(), &NewConversationItem::changed, this, &SmartListModel::slotItemChanged);
    disconnect(newConversationItem.get(), &NewConversationItem::contactFound, this, &SmartListModel::contactFound);
    disconnect(newConversationItem.get(), &NewConversationItem::contactAdded, this, &SmartListModel::contactAdded);
    disconnect(newConversationItem.get(), &NewConversationItem::contactAddedAndCall, this, &SmartListModel::contactAddedAndCall);
    disconnect(newConversationItem.get(), &NewConversationItem::contactAddedAndSend, this, &SmartListModel::contactAddedAndSend);
    items_.pop_front();
}


std::string
SmartListModel::getFilter() const
{
    return filter_;
}

Account*
SmartListModel::fillsWithContacts(Account* account)
{
    if (not account) {
        qDebug() << "no available account selected";
        return account;
    }

    auto contacts = account->getContacts();

    // clear the list
    items_.clear();
    filteredItems_.clear();

    // add contacts to the list
    for (auto c : contacts) {
        auto contact = std::shared_ptr<ContactItem>(new ContactItem(c));
        connect(contact.get(), &ContactItem::changed, this, &SmartListModel::slotItemChanged);
        connect(contact.get(), &ContactItem::lastInteractionChanged, this, &SmartListModel::slotLastInteractionChanged);
        contact->setUID(c->uri().toUtf8().constData());
        items_.emplace_back(contact);
    }
    sortItems();
    filteredItems_ = items_;

    return account;
}


void
SmartListModel::contactAdded(const std::string& id)
{
    fillsWithContacts(AvailableAccountModel::instance().currentDefaultAccount());
    emit modelUpdated();
    emit newContactAdded(id);
}

void
SmartListModel::contactAddedAndCall(const std::string& id)
{
    contactAdded(id);
    // Call the new item
    auto idx = find(id);
    if (idx != -1) {
        auto conversation = std::dynamic_pointer_cast<ContactItem>(items_.at(idx));
        if (conversation) {
            conversation->placeCall();
        }
    }
}

void
SmartListModel::slotItemChanged(SmartListItem* item)
{
    auto idx = find(item->getUID());
    if (idx != -1) {
        emit itemChanged(static_cast<unsigned int>(idx));
    }
}


void
SmartListModel::contactAddedAndSend(const std::string& id, std::string message)
{
    contactAdded(id);
    // Send a message to the new item
    auto idx = find(id);
    if (idx != -1) {
        auto conversation = std::dynamic_pointer_cast<ContactItem>(items_.at(idx));
        if (conversation) {
            conversation->sendMessage(message);
        }
    }
}

void
SmartListModel::slotLastInteractionChanged(SmartListItem* item)
{
    if (items_.empty() && !filter_.empty()) return;
    sortItems();
    emit modelUpdated();
}


void
SmartListModel::sortItems()
{
    std::sort(items_.begin(), items_.end(),
    [](const std::shared_ptr<SmartListItem>& itemA, const std::shared_ptr<SmartListItem>& itemB)
    {
        return itemA->getLastInteractionTimeStamp() > itemB->getLastInteractionTimeStamp();
    });
}

void
SmartListModel::slotNewMessageInDatabase(const std::string& contact, const std::string& account, Message msg)
{
    auto currentAccount = AvailableAccountModel::instance().currentDefaultAccount();
    if (QString(currentAccount->id()).toStdString() != account) return;

    auto idx = find(contact);
    if (idx == -1) return;
    auto conversation = std::dynamic_pointer_cast<ContactItem>(items_.at(idx));
    if (conversation) {
        conversation->newMessageAdded(msg);
    }
}


#include <smartlistmodel.moc>
