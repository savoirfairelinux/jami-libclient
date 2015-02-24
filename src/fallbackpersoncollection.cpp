/************************************************************************************
 *   Copyright (C) 2014-2015 by Savoir-Faire Linux                                       *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>         *
 *                                                                                  *
 *   This library is free software; you can redistribute it and/or                  *
 *   modify it under the terms of the GNU Lesser General Public                     *
 *   License as published by the Free Software Foundation; either                   *
 *   version 2.1 of the License, or (at your option) any later version.             *
 *                                                                                  *
 *   This library is distributed in the hope that it will be useful,                *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of                 *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU              *
 *   Lesser General Public License for more details.                                *
 *                                                                                  *
 *   You should have received a copy of the GNU Lesser General Public               *
 *   License along with this library; if not, write to the Free Software            *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA *
 ***********************************************************************************/
#include "fallbackpersoncollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QHash>
#include <QtCore/QUrl>
#include <QtWidgets/QApplication>
#include <QtCore/QStandardPaths>

//Ring
#include "person.h"
#include "vcardutils.h"
#include "contactmethod.h"
#include "collectioneditor.h"


class FallbackPersonBackendEditor : public CollectionEditor<Person>
{
public:
   FallbackPersonBackendEditor(CollectionMediator<Person>* m) : CollectionEditor<Person>(m) {}
   virtual bool save       ( const Person* item ) override;
   virtual bool append     ( const Person* item ) override;
   virtual bool remove     ( Person*       item ) override;
   virtual bool edit       ( Person*       item ) override;
   virtual bool addNew     ( Person*       item ) override;
   virtual bool addExisting( Person*       item ) override;

   QVector<Person*> m_lItems;

private:
   virtual QVector<Person*> items() const override;
};

class FallbackPersonCollectionPrivate
{
public:
   FallbackPersonCollectionPrivate(CollectionMediator<Person>* mediator);
   CollectionMediator<Person>*  m_pMediator;
};

FallbackPersonCollectionPrivate::FallbackPersonCollectionPrivate(CollectionMediator<Person>* mediator) : m_pMediator(mediator)
{

}

FallbackPersonCollection::FallbackPersonCollection(CollectionMediator<Person>* mediator) :
CollectionInterface(new FallbackPersonBackendEditor(mediator)),d_ptr(new FallbackPersonCollectionPrivate(mediator))
{
}

FallbackPersonCollection::~FallbackPersonCollection()
{

}

bool FallbackPersonBackendEditor::save(const Person* item)
{
   QFile file("/tmp/vcard/"+item->uid()+".vcf");
   file.open(QIODevice::WriteOnly);
   file.write(item->toVCard({}));
   file.close();
   return true;
}

bool FallbackPersonBackendEditor::append(const Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackPersonBackendEditor::remove(Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackPersonBackendEditor::edit( Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackPersonBackendEditor::addNew( Person* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackPersonBackendEditor::addExisting( Person* item)
{
   Q_UNUSED(item)

   m_lItems << item;
   mediator()->addItem(item);
   return true;
}

QVector<Person*> FallbackPersonBackendEditor::items() const
{
   return m_lItems;
}

QString FallbackPersonCollection::name () const
{
   return QObject::tr("vCard backend");
}

QString FallbackPersonCollection::category () const
{
   return QObject::tr("Contacts");
}

QVariant FallbackPersonCollection::icon() const
{
   return QVariant();
}

bool FallbackPersonCollection::isEnabled() const
{
   return true;
}

bool FallbackPersonCollection::load()
{
   bool ok;
   QList< Person* > ret =  VCardUtils::loadDir(QUrl("/tmp/vcard"),ok);
   for(Person* p : ret) {
      editor<Person>()->addExisting(p);
   }
   return true;
}

bool FallbackPersonCollection::reload()
{
   return false;
}

CollectionInterface::SupportedFeatures FallbackPersonCollection::supportedFeatures() const
{
   return (CollectionInterface::SupportedFeatures) (
      CollectionInterface::SupportedFeatures::NONE  |
      CollectionInterface::SupportedFeatures::LOAD  |
      CollectionInterface::SupportedFeatures::CLEAR |
//       CollectionInterface::SupportedFeatures::REMOVE|
      CollectionInterface::SupportedFeatures::ADD   );
}

bool FallbackPersonCollection::clear()
{
   QDir dir("/tmp/vcard");
   for (const QString& file : dir.entryList({"*.vcf"},QDir::Files))
      dir.remove(file);
   return true;
}

QByteArray FallbackPersonCollection::id() const
{
   return "fpc2";
}
