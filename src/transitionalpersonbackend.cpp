/****************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                          *
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
#include "transitionalpersonbackend.h"
#include <personmodel.h>

#include <collectioneditor.h>

#include "globalinstances.h"
#include "interfaces/pixmapmanipulatori.h"

class TransitionalPersonBackendPrivate
{

};

class TransitionalPersonEditor final : public CollectionEditor<Person>
{
public:
   TransitionalPersonEditor(CollectionMediator<Person>* m) : CollectionEditor<Person>(m) {}
   virtual bool save       ( const Person* item ) override;
   virtual bool remove     ( const Person* item ) override;
   virtual bool edit       ( Person*       item ) override;
   virtual bool addNew     ( Person*       item ) override;
   virtual bool addExisting( const Person* item ) override;

private:
   virtual QVector<Person*> items() const override;
};

bool TransitionalPersonEditor::save(const Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalPersonEditor::remove(const Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalPersonEditor::edit( Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalPersonEditor::addNew( Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalPersonEditor::addExisting(const  Person* item)
{
   Q_UNUSED(item)
   return false;
}


QVector<Person*> TransitionalPersonEditor::items() const
{
   return QVector<Person*>();
}

CollectionInterface* TransitionalPersonBackend::m_spInstance = nullptr;

CollectionInterface* TransitionalPersonBackend::instance()
{
   if (!m_spInstance) {
      m_spInstance = PersonModel::instance()->addCollection<TransitionalPersonBackend>();
   }
   return m_spInstance;
}

TransitionalPersonBackend::~TransitionalPersonBackend()
{
}

template<typename T>
TransitionalPersonBackend::TransitionalPersonBackend(CollectionMediator<T>* mediator) :
CollectionInterface(new TransitionalPersonEditor(mediator), nullptr)
{
}

bool TransitionalPersonBackend::load()
{
   return false;
}

bool TransitionalPersonBackend::reload()
{
   return false;
}

// bool TransitionalPersonBackend::append(const Person* item)
// {
//    Q_UNUSED(item)
//    return false;
// }

// bool TransitionalPersonBackend::save(const Person* contact)
// {
//    Q_UNUSED(contact)
//    return false;
// }

///Edit 'contact', the implementation may be a GUI or somehting else
// bool TransitionalPersonBackend::edit( Person* contact)
// {
//    Q_UNUSED(contact)
//    return false;
// }

///Add a new contact to the backend
// bool TransitionalPersonBackend::addNew( Person* contact)
// {
//    Q_UNUSED(contact)
//    return false;
// }

bool TransitionalPersonBackend::isEnabled() const
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> TransitionalPersonBackend::supportedFeatures() const
{
   return CollectionInterface::SupportedFeatures::NONE;
}

QString TransitionalPersonBackend::name () const
{
   return QObject::tr("Contact placeholders");
}

QString TransitionalPersonBackend::category () const
{
   return QObject::tr("Contact");
}

QVariant TransitionalPersonBackend::icon() const
{
   return GlobalInstances::pixmapManipulator().collectionIcon(this,Interfaces::PixmapManipulatorI::CollectionIconHint::CONTACT);
}

QByteArray TransitionalPersonBackend::id() const
{
   return  "trcb";
}

// QList<Person*> TransitionalPersonBackend::items() const
// {
//    return QList<Person*>();
// }
