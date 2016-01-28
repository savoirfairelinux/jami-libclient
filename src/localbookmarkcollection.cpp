/************************************************************************************
 *   Copyright (C) 2014 by Savoir-Faire Linux                                       *
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
#include "localbookmarkcollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>

//Ring
#include <call.h>
#include <account.h>
#include <contactmethod.h>
#include <accountmodel.h>
#include <personmodel.h>
#include <phonedirectorymodel.h>
#include <collectioneditor.h>
#include <globalinstances.h>
#include <interfaces/pixmapmanipulatori.h>

namespace Serializable {
   class BookmarkNode
   {
   public:
      Account*       account  ;
      ContactMethod* cm       ;
      Person*        contact  ;

      void read (const QJsonObject &json);
      void write(QJsonObject       &json);
   };
}

class LocalBookmarkEditor final : public CollectionEditor<ContactMethod>
{
public:
   LocalBookmarkEditor(CollectionMediator<ContactMethod>* m)
     : CollectionEditor<ContactMethod>(m),m_Tracked(false) {}
   virtual bool save       ( const ContactMethod* item ) override;
   virtual bool remove     ( const ContactMethod* item ) override;
   virtual bool addNew     ( ContactMethod*       item ) override;
   virtual bool addExisting( const ContactMethod* item ) override;

   //Attributes
   QVector<ContactMethod*> m_lNumbers;
   QList<Serializable::BookmarkNode> m_Nodes;
   bool m_Tracked;

private:
   virtual QVector<ContactMethod*> items() const override;
};

class LocalBookmarkCollectionPrivate
{
public:

   //Attributes
   constexpr static const char FILENAME[] = "bookmark.json";
};

constexpr const char LocalBookmarkCollectionPrivate::FILENAME[];

LocalBookmarkCollection::LocalBookmarkCollection(CollectionMediator<ContactMethod>* mediator) :
   CollectionInterface(new LocalBookmarkEditor(mediator))
{
   load();
}


LocalBookmarkCollection::~LocalBookmarkCollection()
{
   delete d_ptr;
}

bool LocalBookmarkCollection::load()
{
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
              + QLatin1Char('/')
              + LocalBookmarkCollectionPrivate::FILENAME);
   if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
      LocalBookmarkEditor* e = static_cast<LocalBookmarkEditor*>(editor<ContactMethod>());
      const QByteArray content = file.readAll();
      QJsonDocument loadDoc = QJsonDocument::fromJson(content);
      QJsonArray a = loadDoc.array();

      for (int i = 0; i < a.size(); ++i) {
         QJsonObject o = a[i].toObject();
         Serializable::BookmarkNode n;
         n.read(o);

         e->addExisting(n.cm);

         n.cm->setTracked   (e->m_Tracked);
         n.cm->setBookmarked(true        );

         e->m_Nodes << n;
      }

      return true;
   }
   else
      qWarning() << "Bookmarks doesn't exist or is not readable";
   return false;
}

bool LocalBookmarkEditor::save(const ContactMethod* number)
{
   Q_UNUSED(number)

   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
              + QLatin1Char('/')
              + LocalBookmarkCollectionPrivate::FILENAME);
   if ( file.open(QIODevice::WriteOnly | QIODevice::Text) ) {

      QJsonArray a;
      for (Serializable::BookmarkNode& g : m_Nodes) {
         QJsonObject o;
         g.write(o);
         a.append(o);
      }

      QJsonDocument doc(a);

      QTextStream streamFileOut(&file);
      streamFileOut << doc.toJson();
      streamFileOut.flush();
      file.close();

      return true;
   }
   else
      qWarning() << "Unable to save bookmarks";

   return false;
}

bool LocalBookmarkEditor::remove(const ContactMethod* item)
{
   Q_UNUSED(item)

   if (m_lNumbers.indexOf(const_cast<ContactMethod*>(item)) != -1) {
      m_lNumbers.removeAt(m_lNumbers.indexOf(const_cast<ContactMethod*>(item)));
      mediator()->removeItem(item);

      for (int i =0;i<m_Nodes.size();i++) {
         if (m_Nodes[i].cm == item) {
            m_Nodes.removeAt(i);
            break;
         }
      }

      return save(nullptr);
   }
   return false;
}

bool LocalBookmarkEditor::addNew( ContactMethod* number)
{
   if (!number->isBookmarked()) {
      number->setTracked(m_Tracked);
      number->setBookmarked(true);
      Serializable::BookmarkNode n;

      n.cm = number;
      n.account = number->account();
      n.contact = number->contact();
      m_Nodes << n;

      if (!save(number))
         qWarning() << "Unable to save bookmarks";
   }
   else
      qDebug() << number->uri() << "is already bookmarked";

   addExisting(number);
   return save(number);
}

bool LocalBookmarkEditor::addExisting(const ContactMethod* item)
{
   m_lNumbers << const_cast<ContactMethod*>(item);
   mediator()->addItem(item);
   return false;
}

QVector<ContactMethod*> LocalBookmarkEditor::items() const
{
   return m_lNumbers;
}

QString LocalBookmarkCollection::name () const
{
   return QObject::tr("Local bookmarks");
}

QString LocalBookmarkCollection::category () const
{
   return QObject::tr("Bookmark");
}

QVariant LocalBookmarkCollection::icon() const
{
   return GlobalInstances::pixmapManipulator().collectionIcon(this,Interfaces::PixmapManipulatorI::CollectionIconHint::BOOKMARK);
}

bool LocalBookmarkCollection::isEnabled() const
{
   return true;
}

bool LocalBookmarkCollection::reload()
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalBookmarkCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE      |
      CollectionInterface::SupportedFeatures::LOAD      |
      CollectionInterface::SupportedFeatures::CLEAR     |
      CollectionInterface::SupportedFeatures::ADD       |
      CollectionInterface::SupportedFeatures::MANAGEABLE|
      CollectionInterface::SupportedFeatures::REMOVE    ;
}

bool LocalBookmarkCollection::clear()
{
   return QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DataLocation)
                        + QLatin1Char('/')
                        + LocalBookmarkCollectionPrivate::FILENAME);
}

QByteArray LocalBookmarkCollection::id() const
{
   return "localbookmark";
}


void LocalBookmarkCollection::setPresenceTracked(bool tracked)
{
   static_cast<LocalBookmarkEditor*>(editor<ContactMethod>())->m_Tracked = tracked;
}

bool LocalBookmarkCollection::isPresenceTracked() const
{
   return static_cast<LocalBookmarkEditor*>(editor<ContactMethod>())->m_Tracked;
}

void Serializable::BookmarkNode::read(const QJsonObject &json)
{
   const QString&    uri       = json[ "uri"       ].toString()           ;
   const QByteArray& accountId = json[ "accountId" ].toString().toLatin1();
   const QByteArray& contactId = json[ "contactId" ].toString().toLatin1();

   account = accountId.isEmpty()?nullptr:AccountModel::instance ()->getById       ( accountId            );
   contact = contactId.isEmpty()?nullptr:PersonModel::instance  ()->getPersonByUid( contactId            );
   cm      = uri.isEmpty()?nullptr:PhoneDirectoryModel::instance()->getNumber     ( uri, contact, account);
}

void Serializable::BookmarkNode::write(QJsonObject& json)
{
   if (!account)
      account = cm->account();

   json[ "uri"       ] = cm->uri()                      ;
   json[ "accountId" ] = account?account->id():QString();
   json[ "contactId" ] = contact?contact->uid ():QString();
}
