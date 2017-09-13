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
#include "api/lrc.h"

// Models and database
#include "api/newaccountmodel.h"
#include "database.h"
#include "callbackshandler.h"
#include "dbus/instancemanager.h"

namespace lrc
{

using namespace api;

class LrcPimpl
{

public:
    LrcPimpl(const Lrc& linked);

    const Lrc& linked;
    std::unique_ptr<CallbacksHandler> callbackHandler;
    std::unique_ptr<Database> database;
    std::unique_ptr<NewAccountModel> accountModel;
};

Lrc::Lrc()
{
    // Ensure Daemon is running/loaded (especially on non-DBus platforms)
    // before instantiating LRC and its members
    InstanceManager::instance();
    lrcPipmpl_ = std::make_unique<LrcPimpl>(*this);
}

Lrc::~Lrc()
{
}

const NewAccountModel&
Lrc::getAccountModel() const
{
    return *lrcPipmpl_->accountModel;
}

LrcPimpl::LrcPimpl(const Lrc& linked)
: linked(linked)
, callbackHandler(std::make_unique<CallbacksHandler>(linked))
, database(std::make_unique<Database>())
, accountModel(std::make_unique<NewAccountModel>(*database, *callbackHandler))
{
}

} // namespace lrc
