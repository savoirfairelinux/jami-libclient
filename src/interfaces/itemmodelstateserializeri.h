/****************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                          *
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

#include <typedefs.h>

// Ring
#include <collectioninterface.h>
class Account;
class CollectionManagerInterfaceBase;

namespace Interfaces {

/**
 * Interface for tracking and saving/loading the state (enabled or not) of the backends
 * (CollectionInterfaces)
 */
class ItemModelStateSerializerI {
public:
   /**
    * Hints for the preferredCollection implementation about how to select
    * the right collection.
    */
   enum class Hints {
      NONE            = 0x0 << 0,
      ASK_USER        = 0x1 << 0,
      DO_NOT_ASK_USER = 0x1 << 1,
   };

    virtual ~ItemModelStateSerializerI() = default;

    /**
     * Called when the new enabled collection list need to be serialized
     */
    virtual bool save() = 0;

    /**
     * Called when the enabled collection list is first needed
     *
     * @note You can call this manually if your collections depend on it
     */
    virtual bool load() = 0;

    //Getter
    virtual bool isChecked(const CollectionInterface* backend) const = 0;

    /**
     * Allow to overload the default algoritm used to look for a default
     * collection to store something new.
     *
     * @param manager The likes of PersonModel::instance() or CategorizedHistoryModel::instance()
     * @param features A list of must have collection features. If NONE is set \
     * the implementation is free to return whatever it want. If it is set, a \
     * collection must return a compliant collection or the result will be \
     * ignored.
     */
    virtual CollectionInterface* preferredCollection(CollectionManagerInterfaceBase* manager,
      FlagPack<CollectionInterface::SupportedFeatures> features
         = FlagPack<CollectionInterface::SupportedFeatures>(CollectionInterface::SupportedFeatures::NONE),
      FlagPack<Hints> hints = FlagPack<Hints>(Hints::NONE)
    ) = 0;

    //Setter
    virtual bool setChecked(const CollectionInterface* backend, bool enabled) = 0;
};

} // namespace Interface
