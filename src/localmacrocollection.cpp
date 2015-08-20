/****************************************************************************
 *   Copyright (C) 2013-2015 by Savoir-Faire Linux                          *
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
#include "localmacrocollection.h"

//Qt
#include <QtCore/QFile>
#include <QtCore/QHash>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtCore/QStandardPaths>

//Ring
#include <macro.h>
#include <collectioneditor.h>
#include <macromodel.h>
#include <interfaces/instances.h>
#include <interfaces/pixmapmanipulatori.h>

namespace Serializable {
   class MacroNode
   {
   public:
      Macro* macro;

      void read (const QJsonObject &json);
      void write(QJsonObject       &json);
   };
}

class LocalMacroEditor final : public CollectionEditor<Macro>
{
public:
   LocalMacroEditor(CollectionMediator<Macro>* m) : CollectionEditor<Macro>(m),m_Tracked(false) {}
   virtual bool save       ( const Macro* item ) override;
   virtual bool remove     ( const Macro* item ) override;
   virtual bool addNew     ( Macro*       item ) override;
   virtual bool addExisting( const Macro* item ) override;

   //Attributes
   QVector<Macro*> m_lNumbers;
   QList<Serializable::MacroNode> m_Nodes;
   bool m_Tracked;

private:
   virtual QVector<Macro*> items() const override;
};

class LocalMacroCollectionPrivate
{
public:

   //Attributes
   constexpr static const char FILENAME[] = "macro.json";
};

constexpr const char LocalMacroCollectionPrivate::FILENAME[];

LocalMacroCollection::LocalMacroCollection(CollectionMediator<Macro>* mediator) :
   CollectionInterface(new LocalMacroEditor(mediator))
{
   load();
}


LocalMacroCollection::~LocalMacroCollection()
{
   delete d_ptr;
}

bool LocalMacroCollection::load()
{
   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') +LocalMacroCollectionPrivate::FILENAME);
   if ( file.open(QIODevice::ReadOnly | QIODevice::Text) ) {
      LocalMacroEditor* e = static_cast<LocalMacroEditor*>(editor<Macro>());
      const QByteArray content = file.readAll();
      QJsonDocument loadDoc = QJsonDocument::fromJson(content);
      QJsonArray a = loadDoc.array();

      for (int i = 0; i < a.size(); ++i) {
         QJsonObject o = a[i].toObject();
         Serializable::MacroNode n;
         n.read(o);

         n.macro->setCollection(this);
         e->addExisting(n.macro);

         e->m_Nodes << n;
      }

      return true;
   }
   else
      qWarning() << "Macros doesn't exist or is not readable";
   return false;
}

bool LocalMacroEditor::save(const Macro* macro)
{
   Q_UNUSED(macro)

   QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/')+LocalMacroCollectionPrivate::FILENAME);
   if ( file.open(QIODevice::WriteOnly | QIODevice::Text) ) {

      QJsonArray a;
      for (Serializable::MacroNode& g : m_Nodes) {
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
      qWarning() << "Unable to save macros";

   return false;
}

bool LocalMacroEditor::remove(const Macro* item)
{
   Q_UNUSED(item)

   if (m_lNumbers.indexOf(const_cast<Macro*>(item)) != -1) {
      m_lNumbers.removeAt(m_lNumbers.indexOf(const_cast<Macro*>(item)));
      mediator()->removeItem(item);

      for (int i =0;i<m_Nodes.size();i++) {
         if (m_Nodes[i].macro == item) {
            m_Nodes.removeAt(i);
            break;
         }
      }

      return save(nullptr);
   }
   return false;
}

bool LocalMacroEditor::addNew( Macro* macro)
{
   Serializable::MacroNode n;

   n.macro = macro;
   m_Nodes << n;

   if (!save(macro))
      qWarning() << "Unable to save macros";

   addExisting(macro);
   return save(macro);
}

bool LocalMacroEditor::addExisting(const Macro* item)
{
   m_lNumbers << const_cast<Macro*>(item);
   mediator()->addItem(item);
   return false;
}

QVector<Macro*> LocalMacroEditor::items() const
{
   return m_lNumbers;
}

QString LocalMacroCollection::name () const
{
   return QObject::tr("Local macros");
}

QString LocalMacroCollection::category () const
{
   return QObject::tr("Macro");
}

QVariant LocalMacroCollection::icon() const
{
   return Interfaces::pixmapManipulator().collectionIcon(this,Interfaces::PixmapManipulatorI::CollectionIconHint::MACRO);
}

bool LocalMacroCollection::isEnabled() const
{
   return true;
}

bool LocalMacroCollection::reload()
{
   return false;
}

FlagPack<CollectionInterface::SupportedFeatures> LocalMacroCollection::supportedFeatures() const
{
   return
      CollectionInterface::SupportedFeatures::NONE      |
      CollectionInterface::SupportedFeatures::LOAD      |
      CollectionInterface::SupportedFeatures::CLEAR     |
      CollectionInterface::SupportedFeatures::ADD       |
      CollectionInterface::SupportedFeatures::MANAGEABLE|
      CollectionInterface::SupportedFeatures::REMOVE    ;
}

bool LocalMacroCollection::clear()
{
   return QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + QLatin1Char('/') + LocalMacroCollectionPrivate::FILENAME);
}

QByteArray LocalMacroCollection::id() const
{
   return "localmacro";
}


void LocalMacroCollection::setPresenceTracked(bool tracked)
{
   static_cast<LocalMacroEditor*>(editor<Macro>())->m_Tracked = tracked;
}

bool LocalMacroCollection::isPresenceTracked() const
{
   return static_cast<LocalMacroEditor*>(editor<Macro>())->m_Tracked;
}

void Serializable::MacroNode::read(const QJsonObject &json)
{
   macro = MacroModel::instance()->newMacro(json["ID"].toString());
   macro->setName        ( json[ "Name"   ].toString ());
   macro->setSequence    ( json[ "Seq"    ].toString ());
   macro->setCategory    ( json[ "Cat"    ].toString ());
   macro->setDelay       ( json[ "Delay"  ].toInt    ());
   macro->setDescription ( json[ "Desc"   ].toString ());
}

void Serializable::MacroNode::write(QJsonObject& json)
{
   json[ "Name"  ] = macro->name        ();
   json[ "Seq"   ] = macro->sequence    ();
   json[ "Cat"   ] = macro->category    ();
   json[ "Delay" ] = macro->delay       ();
   json[ "Desc"  ] = macro->description ();
   json[ "ID"    ] = macro->id          ();
}
