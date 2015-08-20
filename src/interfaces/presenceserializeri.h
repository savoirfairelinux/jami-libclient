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
#ifndef PRESENCESERIALIZERI_H
#define PRESENCESERIALIZERI_H

#include <typedefs.h>

class CollectionInterface;

namespace Interfaces {

class PresenceSerializerI {
public:
    virtual ~PresenceSerializerI() = default;
    virtual void save     () = 0;
    virtual void load     () = 0;
    virtual bool isTracked(CollectionInterface* backend) const = 0;
    virtual void setTracked(CollectionInterface* backend, bool tracked) = 0;
};

} // namespace Interfaces

#endif // PRESENCESERIALIZERI_H
