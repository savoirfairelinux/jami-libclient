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
#include <string>

// Qt
#include <qobject.h>

// Data
#include "smartlistitem.h"

typedef std::vector<std::shared_ptr<SmartListItem>> SmartListItems;

class SmartListModel : public QObject {
    Q_OBJECT // on changera ça dans le futur // utilisé pour binder les signaux qt
    public:
    ~SmartListModel();

    //Singleton
    static SmartListModel& instance();

    SmartListItems getItems();
    std::shared_ptr<SmartListItem> getItem(int row);

    // factories (pour l'instant en public, mais il faudra think a little, elles devront sans doute est utilisées au travers
    // des signaux rendant le public caduc).
    // - la factory des contacts provenant du daemon est dans le constructeur dans la lambda associée au changment de compte


    // signals
    Q_SIGNALS:
    void modelUpdated();
    void showConversationView(std::vector<std::string> messages);

    private:
    explicit SmartListModel(QObject* parent = nullptr);
    SmartListItems items;

};
