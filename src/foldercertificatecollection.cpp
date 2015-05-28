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
#include "foldercertificatecollection.h"

//Qt
#include <QtCore/QThread>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDebug>
#include <QtCore/QMutex>
#include <QtCore/QStandardPaths>

//Ring
#include "certificate.h"
#include "certificatemodel.h"
#include "delegates/pixmapmanipulationdelegate.h"

//Dring
#include "dbus/configurationmanager.h"

class FallbackLocalCertificateEditor final : public CollectionEditor<Certificate>
{
public:
   FallbackLocalCertificateEditor(CollectionMediator<Certificate>* m, const QString& path);
   virtual bool save       ( const Certificate* item ) override;
   virtual bool remove     ( const Certificate* item ) override;
   virtual bool edit       ( Certificate*       item ) override;
   virtual bool addNew     ( const Certificate* item ) override;
   virtual bool addExisting( const Certificate* item ) override;

   QVector<Certificate*>             m_lItems;
   QString                           m_Path  ;
   QHash<const Certificate*,QString> m_hPaths;

private:
   virtual QVector<Certificate*> items() const override;
};

class BackgroundLoader final : public QThread
{
   Q_OBJECT
public:
   BackgroundLoader(FolderCertificateCollection* parent);

   //Attributes
   QMutex            m_LoaderMutex;
   FolderCertificateCollection* m_pCurrentFolder;
   QList<FolderCertificateCollection*> m_lFolderQueue;

   //Helpers
   QByteArray        loadCertificate(const QByteArray& id);

protected:
   virtual void run() override;

Q_SIGNALS:
   void listLoaded(const QList<QByteArray>& list);
};

class FolderCertificateCollectionPrivate
{
public:

   //Attributes
   FlagPack<FolderCertificateCollection::Options> m_Flags;
   QString                      m_Path             ;
   QString                      m_Name             ;
   bool                         m_IsValid          ;
   FolderCertificateCollection* m_pParent          ;
   static bool                  m_sHasFallbackStore;
   FolderCertificateCollection* q_ptr              ;
   static BackgroundLoader*     m_spLoader         ;

   //Helper
   QList<CollectionInterface::Element> getCertificateList();
};

bool FolderCertificateCollectionPrivate::m_sHasFallbackStore = false;
BackgroundLoader* FolderCertificateCollectionPrivate::m_spLoader = nullptr;

FolderCertificateCollection::FolderCertificateCollection(CollectionMediator<Certificate>* mediator,
  const QString& path               ,
  const FlagPack<Options>& options  ,
  const QString& name               ,
  FolderCertificateCollection* p
 ) :
CollectionInterface(new FallbackLocalCertificateEditor(mediator,path),p),d_ptr(new FolderCertificateCollectionPrivate())
{
   d_ptr->q_ptr     = this   ;
   d_ptr->m_Flags   = options;
   d_ptr->m_Path    = path   ;
   d_ptr->m_Name    = name   ;
   d_ptr->m_pParent = p      ;
   d_ptr->m_IsValid = true   ;

   if (path.isEmpty()) {
      d_ptr->m_Path = QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/certs/";

      d_ptr->m_IsValid = !FolderCertificateCollectionPrivate::m_sHasFallbackStore;

      if (!d_ptr->m_IsValid) {
         qWarning() << "A fallback certificat store already exist, doing nothing";
      }

      FolderCertificateCollectionPrivate::m_sHasFallbackStore = true;
   }

   if (name.isEmpty()) {
      d_ptr->m_Name  = d_ptr->m_Path;
   }
}

FolderCertificateCollection::~FolderCertificateCollection()
{
   delete d_ptr;
}

QByteArray BackgroundLoader::loadCertificate(const QByteArray& id)
{
//    QMutexLocker(&this->m_Mutex);
   QFile file(id);
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "Error opening certificate: " << id;
      return QByteArray();
   }

   return file.readAll();
}

bool FolderCertificateCollection::load()
{
   if (d_ptr->m_IsValid) {
      //Load the stored certificates
      if (!d_ptr->m_spLoader) {
         d_ptr->m_spLoader = new BackgroundLoader(this);
         QObject::connect(d_ptr->m_spLoader, SIGNAL(finished()), d_ptr->m_spLoader, SLOT(deleteLater()));
      }

      /*if (!loader->isFinished())
         connect(loader,&BackgroundLoader::finished,[loader](){
            delete loader;
         });*/

      d_ptr->m_spLoader->m_lFolderQueue << this;

      if (!d_ptr->m_spLoader->isRunning())
         d_ptr->m_spLoader->start();
      return true;
   }
   return false;
}

bool FolderCertificateCollection::reload()
{
   return false;
}

QList<CollectionInterface::Element> FolderCertificateCollection::listId() const
{
   return d_ptr->getCertificateList();
}

bool FolderCertificateCollection::clear()
{
   return false;
}

QString FolderCertificateCollection::name() const
{
   return d_ptr->m_Name;
}

QString FolderCertificateCollection::category() const
{
   return QObject::tr("Certificate");
}

QVariant FolderCertificateCollection::icon() const
{
   return PixmapManipulationDelegate::instance()->collectionIcon(this,PixmapManipulationDelegate::CollectionIconHint::CERTIFICATE);
}

bool FolderCertificateCollection::isEnabled() const
{
   return true;
}

QByteArray FolderCertificateCollection::id() const
{
   return "FolderCertificateCollection";
}

FlagPack<CollectionInterface::SupportedFeatures> FolderCertificateCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE     |
      CollectionInterface::SupportedFeatures::LISTABLE |
      CollectionInterface::SupportedFeatures::LOAD     |
      CollectionInterface::SupportedFeatures::CLEAR    |
      CollectionInterface::SupportedFeatures::REMOVE   |
      CollectionInterface::SupportedFeatures::ADD      ;
}


QUrl FolderCertificateCollection::path() const
{
   return QUrl(d_ptr->m_Path);
}


/*******************************************************************************
 *                                                                             *
 *                                   Editor                                    *
 *                                                                             *
 ******************************************************************************/

FallbackLocalCertificateEditor::FallbackLocalCertificateEditor(CollectionMediator<Certificate>* m, const QString& path) : CollectionEditor<Certificate>(m),m_Path(path)
{

}

bool FallbackLocalCertificateEditor::save( const Certificate* item)
{
   Q_UNUSED(item)
   /*QMutexLocker(&this->m_Mutex);
   QDir dir(m_Path);

   dir.mkdir("certs/");

   const QString path = "/certs/" + id + ".pem";

   //There could be a race condition if the loader load while this is saving
   if (dir.exists("certs/"+path))
      return QUrl(dir.path() + path);

   QFile file(dir.path() + path);

   if ( file.open(QIODevice::WriteOnly | QIODevice::Text) ) {
      QTextStream streamFileOut(&file);
      streamFileOut << content;
      streamFileOut.flush();
      file.close();
      return QUrl(dir.path() + path);
   }

   return QUrl();*/
   return false;
}

bool FallbackLocalCertificateEditor::remove( const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackLocalCertificateEditor::edit( Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackLocalCertificateEditor::addNew( const Certificate* item)
{
   Q_UNUSED(item)
   return false;
}

bool FallbackLocalCertificateEditor::addExisting( const Certificate* item)
{
   Q_UNUSED(item)
   m_lItems << const_cast<Certificate*>(item);
   return false;
}

QVector<Certificate*> FallbackLocalCertificateEditor::items() const
{
   return m_lItems;
}


/*******************************************************************************
 *                                                                             *
 *                                Async loader                                 *
 *                                                                             *
 ******************************************************************************/

BackgroundLoader::BackgroundLoader(FolderCertificateCollection* parent) : QThread(nullptr),
m_pCurrentFolder(parent)
{

}

QList<CollectionInterface::Element> FolderCertificateCollectionPrivate::getCertificateList()
{
//    QMutexLocker(&this->m_Mutex);
   QDir dir(m_Path);

   if (!dir.exists())
      return QList<QByteArray>();

   QList<QByteArray> ret;
   for (const QString& str : dir.entryList({"*.pem","*.crt"}) ) {
      ret << (m_Path + "/" + str).toLatin1();
   }

   if (m_Flags & FolderCertificateCollection::Options::RECURSIVE) {
      for (const QString& d : dir.entryList(QDir::AllDirs)) {
         if (d != QString('.') && d != "..") {
            CertificateModel::instance()->addCollection<FolderCertificateCollection,QString,FlagPack<FolderCertificateCollection::Options>,QString,FolderCertificateCollection*>(
               m_Path+'/'+d              ,
               m_Flags                   ,
               d                         ,
               q_ptr                     ,
               LoadOptions::FORCE_ENABLED
            );
         }
      }
   }

   return ret;
}

void BackgroundLoader::run()
{
   while (m_lFolderQueue.size()) {
      m_pCurrentFolder = m_lFolderQueue.takeFirst();

      QMutexLocker(&this->m_LoaderMutex);
      for(const QByteArray& id : m_pCurrentFolder->listId()) {
         Certificate* cert = CertificateModel::instance()->getCertificateFromContent(loadCertificate(id),m_pCurrentFolder->name(),false);
         m_pCurrentFolder->editor<Certificate>()->addExisting(cert);
      }

      ConfigurationManagerInterface& configurationManager = DBus::ConfigurationManager::instance();
      //qDebug() << "\n\nPINING PATH TODO remove extra /" << m_pCurrentFolder->path();
      configurationManager.pinCertificatePath(m_pCurrentFolder->path().path()+'/');
   }
   FolderCertificateCollectionPrivate::m_spLoader = nullptr;
   QThread::exit(0);
}

#include "foldercertificatecollection.moc"
