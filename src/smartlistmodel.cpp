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
#include <phonedirectorymodel.h>
#include "database.h"

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
    connect(&PhoneDirectoryModel::instance(), &PhoneDirectoryModel::contactSearched, this, &SmartListModel::slotContactSearched);

    // initialise the list
    fillsWithContacts(AvailableAccountModel::instance().currentDefaultAccount());


    // test only
    DataBase::instance();

}

SmartListModel::~SmartListModel()
{
}

SmartListItems
SmartListModel::getItems()
{
    return items;
}

SmartListModel&
SmartListModel::instance()
{
    static auto instance = new SmartListModel(QCoreApplication::instance());
    return *instance;
}

std::shared_ptr<SmartListItem>
SmartListModel::getItem(int row)
{
    return items[row];
}

void
SmartListModel::slotContactSearched(const ContactMethod* cm)
{
    auto row = find(cm->uri().toStdString());
    if (row != -1) {
        //TODO set this item at first pos and emit modelUpdated (replace action())
        items[row]->action();
    } else {
        // TODO change temporary item
    }
}

int
SmartListModel::find(const std::string& uid) const
{
    for (auto i = 0; i < items.size(); ++i) {
        auto item = items[i];
        if (item->getTitle() == uid) return i;
    }
    return -1;
}

#include <smartlistmodel.moc>
