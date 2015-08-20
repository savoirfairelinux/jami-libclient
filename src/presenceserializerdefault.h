/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-faire Linux                          *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com> *
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

#include "interfaces/presenceserializeri.h"

class CollectionInterface;

namespace Interfaces {

///Default implementation of the PresenceSerializer interface; does not actually preform any serialization
class LIB_EXPORT PresenceSerializerDefault : public PresenceSerializerI {
public:
    void save       () override {}
    void load       () override {}

    ///Always returns 'false'
    bool isTracked  (CollectionInterface* backend) const override;
    void setTracked (CollectionInterface* backend, bool tracked) override;
};

} // namespace Interfaces
