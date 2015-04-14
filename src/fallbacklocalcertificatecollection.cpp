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
#include "fallbacklocalcertificatecollection.h"

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

/*

QUrl KDECertificateSerializationDelegate::saveCertificate(const QByteArray& id, const QByteArray& content)
{
   
}

bool KDECertificateSerializationDelegate::deleteCertificate(const QByteArray& id)
{
   Q_UNUSED(id)
   QMutexLocker(&this->m_Mutex);
   return false;
}

QList<QByteArray> KDECertificateSerializationDelegate::listCertificates()
{
   
}
 * */

class FallbackLocalCertificateEditor : public CollectionEditor<Certificate>
{
public:
   FallbackLocalCertificateEditor(CollectionMediator<Certificate>* m, const QString& path);
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

class BackgroundLoader : public QThread
{
   Q_OBJECT
public:
   BackgroundLoader(FallbackLocalCertificateCollection* parent);

   //Attributes
   QList<QByteArray> m_lIdList    ;
   QMutex            m_LoaderMutex;
   QList<QByteArray> m_lQueue     ;
   FallbackLocalCertificateCollection* m_pFallbackCollection;

   //Helpers
   QByteArray        loadCertificate(const QByteArray& id);

protected:
   virtual void run() override;

Q_SIGNALS:
   void listLoaded(const QList<QByteArray>& list);
};

class FallbackLocalCertificateCollectionPrivate
{
public:

   //Helper
   QList<CollectionInterface::Element> getCertificateList();
};

FallbackLocalCertificateCollection::FallbackLocalCertificateCollection(CollectionMediator<Certificate>* mediator, const QString& path) :
CollectionInterface(new FallbackLocalCertificateEditor(mediator,path)),d_ptr(new FallbackLocalCertificateCollectionPrivate())
{

}

FallbackLocalCertificateCollection::~FallbackLocalCertificateCollection()
{
   delete d_ptr;
}

QByteArray BackgroundLoader::loadCertificate(const QByteArray& id)
{
//    QMutexLocker(&this->m_Mutex);
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/certs/" + id + ".pem");
   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      qDebug() << "Error opening certificate: " << id;
      return QByteArray();
   }

   return file.readAll();
}

bool FallbackLocalCertificateCollection::load()
{
   //Load the stored certificates
   BackgroundLoader* loader = new BackgroundLoader(this);
   QObject::connect(loader, SIGNAL(finished()), loader, SLOT(deleteLater()));

   /*if (!loader->isFinished())
      connect(loader,&BackgroundLoader::finished,[loader](){
         delete loader;
      });*/

   loader->start();
   return true;
   return false;
}

bool FallbackLocalCertificateCollection::reload()
{
   return false;
}

QList<CollectionInterface::Element> FallbackLocalCertificateCollection::listId() const
{
   return d_ptr->getCertificateList();
}

bool FallbackLocalCertificateCollection::clear()
{
   return false;
}

QString FallbackLocalCertificateCollection::name() const
{
   return QObject::tr("Local certificate store");
}

QString FallbackLocalCertificateCollection::category() const
{
   return QObject::tr("Certificate");
}

QVariant FallbackLocalCertificateCollection::icon() const
{
   return QVariant();
}

bool FallbackLocalCertificateCollection::isEnabled() const
{
   return true;
}

QByteArray FallbackLocalCertificateCollection::id() const
{
   return "FallbackLocalCertificateCollection";
}

CollectionInterface::SupportedFeatures FallbackLocalCertificateCollection::supportedFeatures() const
{
   return (CollectionInterface::SupportedFeatures)     (
      CollectionInterface::SupportedFeatures::NONE     |
      CollectionInterface::SupportedFeatures::LISTABLE |
      CollectionInterface::SupportedFeatures::LOAD     |
      CollectionInterface::SupportedFeatures::CLEAR    |
      CollectionInterface::SupportedFeatures::REMOVE   |
      CollectionInterface::SupportedFeatures::ADD      );
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
   QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation));

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

BackgroundLoader::BackgroundLoader(FallbackLocalCertificateCollection* parent) : QThread(nullptr),
m_pFallbackCollection(parent)
{

}

QList<CollectionInterface::Element> FallbackLocalCertificateCollectionPrivate::getCertificateList()
{
//    QMutexLocker(&this->m_Mutex);
   QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/certs/");

   if (!dir.exists())
      return QList<QByteArray>();

   QList<QByteArray> ret;
   for (const QString& str : dir.entryList({"*.pem"}) ) {
      ret << str.left(str.size()-4).toLatin1();
   }

   return ret;
}

void BackgroundLoader::run()
{
   if (m_lIdList.isEmpty()) //WARNING potential race
      m_lIdList = m_pFallbackCollection->listId();

   QMutexLocker(&this->m_LoaderMutex);
   for(const QByteArray& id : m_lIdList) {
      Certificate* cert = CertificateModel::instance()->getCertificateFromContent(loadCertificate(id),nullptr,false);
      m_pFallbackCollection->editor<Certificate>()->addExisting(cert);
   }
   QThread::exit(0);
}

#include "fallbacklocalcertificatecollection.moc"
