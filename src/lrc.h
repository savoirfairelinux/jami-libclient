/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author : Nicolas Jäger <nicolas.jager@savoirfairelinux.com>            *
 *   Author : Sébastien Blin <sebastien.blin@savoirfairelinux.com>          *
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
#include <memory>

// Qt
#include <qobject.h>

namespace lrc
{

class NewAccountModel;
class Database;

class Lrc : public QObject {
    Q_OBJECT
public:
    explicit Lrc();
    ~Lrc();
    NewAccountModel& getAccountModel() {return *accountModel_.get();};

private Q_SLOTS:
    void slotNewAccountMessage(const QString& accountId, const QString& from, const QMap<QString,QString>& payloads);
    void slotNewBuddySubscription(const QString& accountId, const QString& uri, bool status, const QString& message);

private:
    std::unique_ptr<Database> database_;
    std::unique_ptr<NewAccountModel> accountModel_;
};

} // namespace lrc
