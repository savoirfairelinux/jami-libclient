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
#include "transitionalcontactbackend.h"
#include <contactmodel.h>

#include <collectioneditor.h>

class TransitionalContactBackendPrivate
{

};

class TransitionalContactEditor : public CollectionEditor<Contact>
{
public:
   TransitionalContactEditor(CollectionMediator<Contact>* m) : CollectionEditor<Contact>(m) {}
   virtual bool save       ( const Contact* item ) override;
   virtual bool append     ( const Contact* item ) override;
   virtual bool remove     ( Contact*       item ) override;
   virtual bool edit       ( Contact*       item ) override;
   virtual bool addNew     ( Contact*       item ) override;

private:
   virtual QVector<Contact*> items() const override;
};

bool TransitionalContactEditor::save(const Contact* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalContactEditor::append(const Contact* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalContactEditor::remove(Contact* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalContactEditor::edit( Contact* item)
{
   Q_UNUSED(item)
   return false;
}

bool TransitionalContactEditor::addNew( Contact* item)
{
   Q_UNUSED(item)
   return false;
}

QVector<Contact*> TransitionalContactEditor::items() const
{
   return QVector<Contact*>();
}

CollectionInterface* TransitionalContactBackend::m_spInstance = nullptr;

CollectionInterface* TransitionalContactBackend::instance()
{
   if (!m_spInstance) {
      m_spInstance = ContactModel::instance()->addBackend<TransitionalContactBackend>();
   }
   return m_spInstance;
}

TransitionalContactBackend::~TransitionalContactBackend()
{
}

template<typename T>
TransitionalContactBackend::TransitionalContactBackend(CollectionMediator<T>* mediator) :
CollectionInterface(new TransitionalContactEditor(mediator), nullptr)
{
}

bool TransitionalContactBackend::load()
{
   return false;
}

bool TransitionalContactBackend::reload()
{
   return false;
}

// bool TransitionalContactBackend::append(const Contact* item)
// {
//    Q_UNUSED(item)
//    return false;
// }

// bool TransitionalContactBackend::save(const Contact* contact)
// {
//    Q_UNUSED(contact)
//    return false;
// }

///Edit 'contact', the implementation may be a GUI or somehting else
// bool TransitionalContactBackend::edit( Contact* contact)
// {
//    Q_UNUSED(contact)
//    return false;
// }

///Add a new contact to the backend
// bool TransitionalContactBackend::addNew( Contact* contact)
// {
//    Q_UNUSED(contact)
//    return false;
// }

bool TransitionalContactBackend::isEnabled() const
{
   return false;
}

CollectionInterface::SupportedFeatures TransitionalContactBackend::supportedFeatures() const
{
   return CollectionInterface::SupportedFeatures::NONE;
}

QString TransitionalContactBackend::name () const
{
   return QObject::tr("Transitional contact backend");
}

QString TransitionalContactBackend::category () const
{
   return QObject::tr("Contact");
}

QVariant TransitionalContactBackend::icon() const
{
   return QVariant();
}

QByteArray TransitionalContactBackend::id() const
{
   return  "trcb";
}

// QList<Contact*> TransitionalContactBackend::items() const
// {
//    return QList<Contact*>();
// }
