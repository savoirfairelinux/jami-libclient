/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-faire Linux                          *
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
#ifndef ITEMMODELSTATESERIALIZERI_H
#define ITEMMODELSTATESERIALIZERI_H

#include <typedefs.h>

// Ring
class CollectionInterface;
class Account;

namespace Interfaces {

/**
 * Interface for tracking and saving/loading the state (enabled or not) of the backends
 * (CollectionInterfaces)
 */
class ItemModelStateSerializerI {
public:
    virtual ~ItemModelStateSerializerI() = default;

    virtual bool save() = 0;
    virtual bool load() = 0;

    //Getter
    virtual bool isChecked(const CollectionInterface* backend) const = 0;

    //Setter
    virtual bool setChecked(const CollectionInterface* backend, bool enabled) = 0;
};

} // namespace Interface

#endif // ITEMMODELSTATESERIALIZERI_H
