/****************************************************************************
 *   Copyright (C) 2016-2017 Savoir-faire Linux                               *
 *   Author : Alexandre Lision <alexandre.lision@savoirfairelinux.com>      *
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

#include "collectioninterface.h"
#include "collectioneditor.h"

class Profile;

template<typename T> class CollectionMediator;

class LIB_EXPORT LocalProfileCollection : public CollectionInterface
{
public:
    explicit LocalProfileCollection(CollectionMediator<Profile>* mediator);
    virtual ~LocalProfileCollection();

    virtual bool load  () override;
    virtual bool reload() override;
    virtual bool clear () override;

    virtual QString    name     () const override;
    virtual QString    category () const override;
    virtual QVariant   icon     () const override;
    virtual bool       isEnabled() const override;
    virtual QByteArray id       () const override;

    virtual FlagPack<SupportedFeatures> supportedFeatures() const override;

private:
    void setupDefaultProfile();
};
