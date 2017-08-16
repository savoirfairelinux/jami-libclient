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
#include <string>

// Lrc
#include "contactitem.h"
#include "contact.h"
#include "namedirectory.h"

class LIB_EXPORT NewConversationItem : public ContactItem {
    Q_OBJECT
    public:
    explicit NewConversationItem();
    ~NewConversationItem();

    void setTitle(const std::string) override;
    const std::string getTitle() const override;
    const std::string getAlias() const override { return m_sAlias; };
    virtual void activate() override;
    const bool isPresent() const override;

    void search(const std::string& query);

Q_SIGNALS:
    void changed();

public Q_SLOTS:
    void registeredNameFound(const Account* account, NameDirectory::LookupStatus status, const QString& address, const QString& name);

private:
    std::string m_sAlias;
};
