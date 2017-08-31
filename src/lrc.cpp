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
#include "database.h"
#include "api/newaccountmodel.h"
#include "callbackshandler.h"

namespace lrc
{

namespace api
{

class LrcPimpl
{

public:
    LrcPimpl();

    std::unique_ptr<Lrc> parent;
    std::unique_ptr<Database> database;
    std::unique_ptr<NewAccountModel> accountModel;
    std::unique_ptr<CallbacksHandler> callbackHandler;
};

Lrc::Lrc()
: lrcPipmpl_(std::make_unique<LrcPimpl>())
{
}

Lrc::~Lrc()
{
}

NewAccountModel&
Lrc::getAccountModel()
{
    return *lrcPipmpl_->accountModel;
}

LrcPimpl::LrcPimpl()
{
    // create the database.
    database = std::make_unique<Database>();

    // create account model.
    accountModel = std::unique_ptr<NewAccountModel>(new NewAccountModel(*database));
}

} // namespace api
} // namespace lrc
