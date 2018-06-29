/****************************************************************************
 *   Copyright (C) 2017-2018 Savoir-faire Linux                                  *
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

class BehaviorController;
class NewAccountModel;
class DataTransferModel;

class LIB_EXPORT Lrc {
public:
    Lrc();
    ~Lrc();
    /**
     * get a reference on account model.
     * @return a NewAccountModel&.
     */
    const NewAccountModel& getAccountModel() const;
    /**
     * get a reference on the behavior controller.
     * @return a BehaviorController&.
     */
    const BehaviorController& getBehaviorController() const;
    /**
     * get a reference on the DataTransfer controller.
     * @return a DataTransferModel&.
     */
    DataTransferModel& getDataTransferModel() const;

    /**
     * Inform the daemon that the connectivity changed
     */
    void connectivityChanged() const;

private:
    std::unique_ptr<LrcPimpl> lrcPimpl_;
};

} // namespace api
} // namespace lrc
