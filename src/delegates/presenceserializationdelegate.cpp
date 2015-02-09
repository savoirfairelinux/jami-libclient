/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                           *
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
#include "presenceserializationdelegate.h"

class LIB_EXPORT DummyPresenceSerializationDelegate : public PresenceSerializationDelegate {
public:
   virtual void serialize() override;
   virtual void load     () override;
   virtual bool isTracked(CollectionInterface* backend) override;
   virtual void setTracked(CollectionInterface* backend, bool tracked) override;
   virtual ~DummyPresenceSerializationDelegate();

};

PresenceSerializationDelegate* PresenceSerializationDelegate::m_spInstance = new DummyPresenceSerializationDelegate();

void DummyPresenceSerializationDelegate::serialize()
{
   
}
void DummyPresenceSerializationDelegate::load()
{
   
}

bool DummyPresenceSerializationDelegate::isTracked(CollectionInterface* backend)
{
   Q_UNUSED(backend)
   return false;
}

void DummyPresenceSerializationDelegate::setTracked(CollectionInterface* backend, bool tracked)
{
   Q_UNUSED(backend)
   Q_UNUSED(tracked)
}

DummyPresenceSerializationDelegate::~DummyPresenceSerializationDelegate()
{}

PresenceSerializationDelegate* PresenceSerializationDelegate::instance()
{
   return m_spInstance;
}

void PresenceSerializationDelegate::setInstance(PresenceSerializationDelegate* ins)
{
   m_spInstance = ins;
   ins->load();
}
