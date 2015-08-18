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
#include "account.h"
#include "certificatemodel.h"
#include "delegates/pixmapmanipulationdelegate.h"

//Dring
#include "dbus/configurationmanager.h"
#include "security_const.h"

class DaemonCertificateEditor final : public CollectionEditor<Certificate>
{
public:
   DaemonCertificateEditor(CollectionMediator<Certificate>* m, const QString& path);
   virtual bool save       ( const Certificate* item ) override;
   virtual bool remove     ( const Certificate* item ) override;
   virtual bool edit       ( Certificate*       item ) override;
   virtual bool addNew     ( Certificate*       item ) override;
   virtual bool addExisting( const Certificate* item ) override;

   QVector<Certificate*>             m_lItems;
   QString                           m_Path  ;
   QHash<const Certificate*,QString> m_hPaths;

private:
   virtual QVector<Certificate*> items() const override;
};

class DaemonCertificateCollectionPrivate : public QObject
{
   Q_OBJECT
public:
   DaemonCertificateCollectionPrivate(DaemonCertificateCollection* parent, Account* a, DaemonCertificateCollection::Mode mode);

   //Attributes
   DaemonCertificateCollection* q_ptr;
   Account* m_pAccount;
   DaemonCertificateCollection::Mode m_Mode;

public Q_SLOTS:
   void slotCertificatePinned(const QString& id);
   void slotCertificateExpired(const QString& id);
   void slotCertificatePathPinned(const QString& path, const QStringList& certIds);
};

DaemonCertificateCollectionPrivate::DaemonCertificateCollectionPrivate(DaemonCertificateCollection* parent, Account* a, DaemonCertificateCollection::Mode mode) : QObject(), q_ptr(parent),
m_pAccount(a), m_Mode(mode)
{
   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   connect(&configurationManager, &ConfigurationManagerInterface::certificatePinned     , this, &DaemonCertificateCollectionPrivate::slotCertificatePinned    );

   connect(&configurationManager, &ConfigurationManagerInterface::certificateExpired    , this, &DaemonCertificateCollectionPrivate::slotCertificateExpired   );

   connect(&configurationManager, &ConfigurationManagerInterface::certificatePathPinned , this, &DaemonCertificateCollectionPrivate::slotCertificatePathPinned);

   //    connect(&configurationManager, &ConfigurationManagerInterface::incomingTrustRequest  , this, &DaemonCertificateCollectionPrivate::);
}

DaemonCertificateCollection::DaemonCertificateCollection(CollectionMediator<Certificate>* mediator, Account* a, DaemonCertificateCollection::Mode mode) :
CollectionInterface(new DaemonCertificateEditor(mediator,QString()),nullptr),d_ptr(new DaemonCertificateCollectionPrivate(this,a,mode))
{

}

DaemonCertificateCollection::~DaemonCertificateCollection()
{
   delete d_ptr;
}

void DaemonCertificateCollectionPrivate::slotCertificatePinned(const QString& id)
{
   //qDebug() << "\n\nCERTIFICATE ADDED" << id;
   Certificate* cert = CertificateModel::instance()->getCertificateFromId(id);

   if (!cert->collection())
      cert->setCollection(q_ptr);

   q_ptr->editor<Certificate>()->addExisting(cert);
}

void DaemonCertificateCollectionPrivate::slotCertificateExpired(const QString& id)
{
   Q_UNUSED(id);
   //qDebug() << "\n\nCERTIFICATE EXPIRED" << id;
}

void DaemonCertificateCollectionPrivate::slotCertificatePathPinned(const QString& path, const QStringList& certIds)
{
   Q_UNUSED(path);
   Q_UNUSED(certIds);
   //Create a new collection if it is a directory or size > 1
   //qDebug() << "\n\nSIGNAL CERTIFICATE PATH PINNING" << path << certIds;
}

bool DaemonCertificateCollection::load()
{
   if (!d_ptr->m_pAccount)
      return false;

   const QString mode = d_ptr->m_Mode == DaemonCertificateCollection::Mode::ALLOWED ?
      DRing::Certificate::Status::ALLOWED : DRing::Certificate::Status::BANNED;

   ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();

   const QStringList certs = configurationManager.getCertificatesByStatus(
      d_ptr->m_pAccount->id(), mode
   );
   //qDebug() << "\n\nLOADING CERTS" << d_ptr->m_pAccount << certs;

   for (const QString& id : certs)
      CertificateModel::instance()->getCertificateFromId(id,d_ptr->m_pAccount,d_ptr->m_pAccount->id()+"_"+mode);

   return true;
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
   return QObject::tr("%1 %2 list")
      .arg(d_ptr->m_pAccount ? d_ptr->m_pAccount->alias() : QObject::tr("Daemon certificate store"))
      .arg(d_ptr->m_Mode == Mode::BANNED ?
         QObject::tr( "banned"  ) :
         QObject::tr( "allowed" )
      );
}

QString DaemonCertificateCollection::category() const
{
   return QObject::tr("Certificate");
}

QVariant DaemonCertificateCollection::icon() const
{
   return PixmapManipulationDelegate::instance()->collectionIcon(this,PixmapManipulationDelegate::CollectionIconHint::CERTIFICATE);
}

bool DaemonCertificateCollection::isEnabled() const
{
   return true;
}

QByteArray DaemonCertificateCollection::id() const
{
   return "DaemonCertificateCollection";
}

FlagPack<CollectionInterface::SupportedFeatures> DaemonCertificateCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE     |
      CollectionInterface::SupportedFeatures::LOAD     |
      CollectionInterface::SupportedFeatures::REMOVE   |
      CollectionInterface::SupportedFeatures::LISTABLE ;
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
   //TODO
   Q_UNUSED(item);
   return false;
}

bool DaemonCertificateEditor::edit( Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool DaemonCertificateEditor::addNew( Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool DaemonCertificateEditor::addExisting( const Certificate* item)
{
   Q_UNUSED(item)
   m_lItems << const_cast<Certificate*>(item);
   return false;
}

QVector<Certificate*> DaemonCertificateEditor::items() const
{
   return m_lItems;
}

#include <daemoncertificatecollection.moc>
