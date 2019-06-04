/****************************************************************************
 *    Copyright (C) 2017-2019 Savoir-faire Linux Inc.                       *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
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
#pragma once

// std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
#include "typedefs.h"

class DaemonPimpl;

class Daemon : public QObject {
    Q_OBJECT

public:
    Daemon();
    ~Daemon();

    std::vector<std::string> getAccountList() const;

    void addAccount(const std::string& alias, bool isSIP = false);
    std::string getAccountId(const std::string& alias) const;

    void addNewDevice(const std::string& accountId, const std::string& id, const std::string& name); // TODO I am not sure that we can control the generated id here.


private:
    std::unique_ptr<DaemonPimpl> pimpl_;

};
