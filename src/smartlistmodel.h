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
#pragma once

// Std
#include <list>
#include <deque>
#include <string>

// Qt
#include <qobject.h>

// Data
#include "smartlistitem.h"

class NewConversationItem;
class Account;
class ContactItem;

typedef std::deque<std::shared_ptr<SmartListItem>> SmartListItems;

class LIB_EXPORT SmartListModel : public QObject {
    Q_OBJECT // on changera ça dans le futur // utilisé pour binder les signaux qt
    public:
    ~SmartListModel();

    //Singleton
    static SmartListModel& instance();

    SmartListItems getItems() const;
    std::shared_ptr<SmartListItem> getItem(int row);

    // factories (pour l'instant en public, mais il faudra think a little, elles devront sans doute est utilisées au travers
    // des signaux rendant le public caduc).
    // - la factory des contacts provenant du daemon est dans le constructeur dans la lambda associée au changment de compte

    int find(const std::string& uid) const;
    int findFiltered(const std::string& uid) const;
    void openConversation(const std::string& uid) const;
    void removeConversation(const std::string& title);

    void setFilter(const std::string& newFilter);
    std::string getFilter() const;

    // signals
    Q_SIGNALS:
    void itemChanged(unsigned int row);
    void modelUpdated();
    void conversationItemUpdated(const unsigned int row);
    void newContactAdded(const std::string& uid);
    void incomingCallFromItem(const unsigned int row);

    // NOTE for njager: I need the history when the chatview is loaded. Not for creating the chatview.
    // I think it's more logical to get the history when we want
    // But I need to know which item is activated for creating the chatview. So I add SmartListItem* item
    void showConversationView(SmartListItem* item);
    void newConversationItemActivated(NewConversationItem* item); // TODO replace by showNewConversation
    void ShowIncomingCallView(ContactItem* item); // adding const is good, but requires some work in gnome-ring

    public Q_SLOTS:
    void contactFound(const std::string& uid);
    void contactAdded(const std::string& uid);
    void contactAddedAndSend(const std::string& uid, std::string message);
    void slotItemChanged(SmartListItem*);
    void contactAddedAndCall(const std::string& uid);

    private:
    explicit SmartListModel(QObject* parent = nullptr);
    Account* fillsWithContacts(Account* account);

    void removeNewConversationItem();
    std::shared_ptr<NewConversationItem> createNewConversationItem();

    SmartListItems items_;
    mutable SmartListItems filteredItems_;
    std::string m_sFilter;

};
