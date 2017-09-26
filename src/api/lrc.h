/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
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

// Std
#include <memory>

// Lrc
#include "typedefs.h"

namespace lrc
{

class LrcPimpl;

namespace api
{

class NewAccountModel;

class LIB_EXPORT Lrc {
public:
    Lrc();
    ~Lrc();
    /**
     * get a reference on account model.
     * @return a NewAccountModel&.
     */
    const NewAccountModel& getAccountModel() const;

private:
    std::unique_ptr<LrcPimpl> lrcPipmpl_;
};

} // namespace api
} // namespace lrc
