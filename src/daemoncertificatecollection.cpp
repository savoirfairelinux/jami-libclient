/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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
#include "daemoncertificatecollection.h"

//Ring
#include "certificate.h"

class DaemonCertificateEditor : public CollectionEditor<Certificate>
{
public:
   DaemonCertificateEditor(CollectionMediator<Certificate>* m, const QString& path);
   virtual bool save       ( const Certificate* item ) override;
   virtual bool remove     ( const Certificate* item ) override;
   virtual bool edit       ( Certificate*       item ) override;
   virtual bool addNew     ( const Certificate* item ) override;
   virtual bool addExisting( const Certificate* item ) override;

   QVector<Certificate*>             m_lItems;
   QString                      m_Path  ;
   QHash<const Certificate*,QString> m_hPaths;

private:
   virtual QVector<Certificate*> items() const override;
};

class DaemonCertificateCollectionPrivate
{
public:
};

DaemonCertificateCollection::DaemonCertificateCollection(CollectionMediator<Certificate>* mediator, const QString& path) :
CollectionInterface(new DaemonCertificateEditor(mediator,path),nullptr),d_ptr(new DaemonCertificateCollectionPrivate())
{
   
}

DaemonCertificateCollection::~DaemonCertificateCollection()
{
   delete d_ptr;
}

bool DaemonCertificateCollection::load()
{
   return false;
}

bool DaemonCertificateCollection::reload()
{
   return false;
}

bool DaemonCertificateCollection::clear()
{
   return false;
}

QString DaemonCertificateCollection::name() const
{
   return QObject::tr("Local certificate store");
}

QString DaemonCertificateCollection::category() const
{
   return QObject::tr("Certificate");
}

QVariant DaemonCertificateCollection::icon() const
{
   return QVariant();
}

bool DaemonCertificateCollection::isEnabled() const
{
   return true;
}

QByteArray DaemonCertificateCollection::id() const
{
   return "DaemonCertificateCollection";
}

CollectionInterface::SupportedFeatures DaemonCertificateCollection::supportedFeatures() const
{
   return (CollectionInterface::SupportedFeatures)     (
      CollectionInterface::SupportedFeatures::NONE     |
      CollectionInterface::SupportedFeatures::LOAD     |
      CollectionInterface::SupportedFeatures::CLEAR    |
      CollectionInterface::SupportedFeatures::REMOVE   |
      CollectionInterface::SupportedFeatures::LISTABLE |
      CollectionInterface::SupportedFeatures::ADD      );
}


/*******************************************************************************
 *                                                                             *
 *                                   Editor                                    *
 *                                                                             *
 ******************************************************************************/

DaemonCertificateEditor::DaemonCertificateEditor(CollectionMediator<Certificate>* m, const QString& path) : CollectionEditor<Certificate>(m),m_Path(path)
{

}

bool DaemonCertificateEditor::save( const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool DaemonCertificateEditor::remove( const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool DaemonCertificateEditor::edit( Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool DaemonCertificateEditor::addNew( const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool DaemonCertificateEditor::addExisting( const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

QVector<Certificate*> DaemonCertificateEditor::items() const
{
   return m_lItems;
}
