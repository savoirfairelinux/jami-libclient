/****************************************************************************
 *   Copyright (C) 2013-2014 by Savoir-Faire Linux                          *
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
#include "profilemodel.h"

//Qt
#include <QtCore/QSortFilterProxyModel>
#include <QtCore/QTimer>
#include <QtCore/QObject>
#include <QtCore/QDateTime>
#include <QtCore/QUrl>
#include <QMimeData>
#include <QtCore/QCoreApplication>
#include <QtCore/QPointer>
#include <QtCore/QItemSelectionModel>

//SFLPhone library
#include "accountmodel.h"
#include "collectioninterface.h"
#include "collectioneditor.h"
#include "personmodel.h"
#include "callmodel.h"
#include "contactmethod.h"
#include "person.h"
#include "delegates/profilepersisterdelegate.h"
#include "delegates/pixmapmanipulationdelegate.h"
#include "private/vcardutils.h"
#include "mime.h"

//Qt
class QObject;

//SFLPhone
class Person;
class Account;
struct Node;

class ProfileEditor final : public CollectionEditor<Person>
{
public:
   ProfileEditor(CollectionMediator<Person>* m) : CollectionEditor<Person>(m) {}
   ~ProfileEditor();
   virtual bool save       ( const Person* item ) override;
   virtual bool remove     ( const Person* item ) override;
   virtual bool edit       (       Person* item ) override;
   virtual bool addNew     ( Person*       item ) override;
   virtual bool addExisting( const Person* item ) override;

   Node* getProfileById(const QByteArray& id);
   QList<Account*> getAccountsForProfile(const QString& id);
   QList<Node*> m_lProfiles;
   QHash<QByteArray,Node*> m_hProfileByAccountId;
   QVector<Person*> m_lProfilePersons;

private:
   virtual QVector<Person*> items() const override;
};

///ProfileContentBackend: Implement a backend for Profiles
class ProfileContentBackend final : public QObject, public CollectionInterface {
   Q_OBJECT
public:
   template<typename T>
   explicit ProfileContentBackend(CollectionMediator<T>* mediator);
   virtual ~ProfileContentBackend ();

   //Re-implementation
   virtual QString  name              (             ) const override;
   virtual QString  category          (             ) const override;
   virtual QVariant icon              (             ) const override;
   virtual bool     isEnabled         (             ) const override;
   virtual bool     enable            ( bool enable )       override;
   virtual QByteArray  id             (             ) const override;
   virtual bool     load              (             )       override;
   virtual bool     reload            (             )       override;
   virtual int      size              (             ) const override;
   FlagPack<SupportedFeatures> supportedFeatures(             ) const override;

   //Attributes
   bool m_needSaving;
   bool m_DelayedLoading {false};

   QList<Person*> m_bSaveBuffer;
   bool saveAll();

   Node* m_pDefault;
   ProfileEditor* m_pEditor;

   //Helper
   void  setupDefaultProfile();
   void  addAccount(Node* parent, Account* acc);

public Q_SLOTS:
   void contactChanged();
   void save();
   void loadProfiles();
};


struct Node {
   Node(): account(nullptr),contact(nullptr),type(Node::Type::PROFILE), parent(nullptr),m_Index(0) {}

   enum class Type : bool {
      PROFILE,
      ACCOUNT,
   };

   Node*           parent;
   QVector<Node*>  children;
   Type            type;
   Account*        account;
   Person*         contact;
   int             m_Index;
   uint            m_ParentIndex {(uint)-1}; //INT_MAX
};

bool ProfileEditor::save(const Person* contact)
{
#if QT_VERSION >= 0x050400
    if (!contact->property("delayedSaving").toBool()) {
        const_cast<Person*>(contact)->setProperty("delayedSaving", true);

        QTimer::singleShot(0,[this,contact]() {
                const_cast<Person*>(contact)->setProperty("delayedSaving", false);
#endif
                const auto& profilesDir = ProfilePersisterDelegate::instance()->getProfilesDir();
                const auto& filename = profilesDir.absolutePath() + '/' + contact->uid() + ".vcf";
                qDebug() << "Saving vcf in:" << filename;
                const auto& result = contact->toVCard(getAccountsForProfile(contact->uid()));

                QFile file {filename};
                file.open(QIODevice::WriteOnly);
                file.write(result);
                file.close();
#if QT_VERSION >= 0x050400
            });

    }
#endif
    return true;
}

ProfileEditor::~ProfileEditor()
{
   while (m_lProfiles.size()) {
      Node* c = m_lProfiles[0];
      m_lProfiles.removeAt(0);
      delete c;
   }
}

bool ProfileEditor::remove(const Person* item)
{
   Q_UNUSED(item)
   mediator()->removeItem(item);
   return false;
}

bool ProfileEditor::edit( Person* contact)
{
   qDebug() << "Attempt to edit a profile contact" << contact->uid();
   return false;
}

bool ProfileEditor::addNew( Person* contact)
{
   qDebug() << "Creating new profile" << contact->uid();
   m_lProfilePersons << contact;
   mediator()->addItem(contact);
   save(contact);
//    load(); //FIXME
   return true;
}

bool ProfileEditor::addExisting(const Person* contact)
{
   m_lProfilePersons << const_cast<Person*>(contact);
   mediator()->addItem(contact);
   return true;
}

QVector<Person*> ProfileEditor::items() const
{
   return m_lProfilePersons;
}

int ProfileContentBackend::size() const
{
   return m_pEditor->m_lProfiles.size();
}

ProfileModel* ProfileModel::m_spInstance = nullptr;

template<typename T>
ProfileContentBackend::ProfileContentBackend(CollectionMediator<T>* mediator) :
  CollectionInterface(new ProfileEditor(mediator),nullptr), m_pDefault(nullptr),
  m_needSaving(false)
{
   m_pEditor = static_cast<ProfileEditor*>(editor<Person>());
}

QString ProfileContentBackend::name () const
{
   return tr("Profile backend");
}

QString ProfileContentBackend::category () const
{
   return tr("Profile");
}

QVariant ProfileContentBackend::icon() const
{
   return QVariant();
}

bool ProfileContentBackend::isEnabled() const
{
   return true;
}

bool ProfileContentBackend::enable (bool enable)
{
   Q_UNUSED(enable);
   static bool isLoaded = false;
   if (!isLoaded) {
     load();
     isLoaded = true;
   }
   return true;
}

QByteArray  ProfileContentBackend::id() const
{
   return "Profile_backend";
}

// bool ProfileContentBackend::edit( Person* contact )
// {
//    qDebug() << "Attempt to edit a profile contact" << contact->uid();
//    return false;
// }

// bool ProfileContentBackend::addNew( Person* contact )
// {
//    qDebug() << "Creating new profile" << contact->uid();
//    save(contact);
//    load();
//    return true;
// }

// bool ProfileContentBackend::remove( Person* c )
// {
//    Q_UNUSED(c)
//    return false;
// }

// bool ProfileContentBackend::append(const Person* item)
// {
//    Q_UNUSED(item)
//    return false;
// }

ProfileContentBackend::~ProfileContentBackend( )
{

}

void ProfileContentBackend::setupDefaultProfile()
{

   QHash<Account*,bool> accounts;
   QList<Account*> orphans;

   //Ugly reverse mapping, but we want to keep the profile concept
   //hidden as low as possible for now
   //TODO remove this atrocity when the profile before available in Account::
   //BUG this doesn't work with new accounts
   for (int i=0; i < AccountModel::instance()->size();i++) {
      accounts[(*AccountModel::instance())[i]] = false;
   }

   foreach (Node* node, m_pEditor->m_lProfiles) {
      foreach (Node* acc, node->children) {
         accounts[acc->account] = true;
      }
   }

   for (QHash<Account*,bool>::iterator i = accounts.begin(); i != accounts.end(); ++i) {
      if (i.value() == false) {
         orphans << i.key();
      }
   }
   qDebug() << "ORPHAN" << orphans.size() << m_pEditor << m_pEditor->m_lProfiles.size();

   if (orphans.size() && (!m_pDefault)) {
      qDebug() << "No profile found, creating one";
      Person* profile = new Person(this,QString::number(QDateTime::currentDateTime().currentMSecsSinceEpoch()).toUtf8());
      m_pEditor->addNew(profile);
      profile->setFormattedName(tr("Default"));

      m_pDefault          = new Node           ;
      m_pDefault->type    = Node::Type::PROFILE;
      m_pDefault->contact = profile            ;
      m_pDefault->m_Index = m_pEditor->m_lProfiles.size() ;

      ProfileModel::instance()->beginInsertRows(QModelIndex(), m_pEditor->m_lProfiles.size(), m_pEditor->m_lProfiles.size());
      m_pEditor->m_lProfiles << m_pDefault;
      ProfileModel::instance()->endInsertRows();
   }

   foreach(Account* a, orphans) {
      addAccount(m_pDefault, a);
   }
}

void ProfileContentBackend::addAccount(Node* parent, Account* acc)
{
   Node* account_pro = new Node;
   account_pro->type    = Node::Type::ACCOUNT;
   account_pro->contact = parent->contact;
   account_pro->parent  = parent;
   account_pro->account = acc;
   account_pro->m_Index = parent->children.size();
   account_pro->m_ParentIndex = acc->index().row();

   ProfileModel::instance()->beginInsertRows(ProfileModel::instance()->index(parent->m_Index,0), parent->children.size(), parent->children.size());
   parent->children << account_pro;
   ProfileModel::instance()->endInsertRows();
   m_pEditor->m_hProfileByAccountId[acc->id()] = account_pro;

   if (parent->contact)
      acc->contactMethod()->setPerson(parent->contact);
}

void ProfileContentBackend::loadProfiles()
{
   if (ProfilePersisterDelegate::instance()) {
      m_pEditor->m_lProfiles.clear();

      const QDir profilesDir = ProfilePersisterDelegate::instance()->getProfilesDir();

      qDebug() << "Loading vcf from:" << profilesDir;

      const QStringList entries = profilesDir.entryList({"*.vcf"}, QDir::Files);

      foreach (const QString& item , entries) {

         Person* profile = new Person(this);

         Node* pro    = new Node           ;
         pro->type    = Node::Type::PROFILE;
         pro->contact = profile            ;
         pro->m_Index = m_pEditor->m_lProfiles.size() ;

         QList<Account*> accs;
         qDebug() << "\n\n\nMAPPING!!!";
         VCardUtils::mapToPerson(profile,QUrl(profilesDir.path()+'/'+item),&accs);
         qDebug() << "\n\nEND MAP";
         foreach(Account* a, accs)
            addAccount(pro,a);

         ProfileModel::instance()->beginInsertRows(QModelIndex(), m_pEditor->m_lProfiles.size(), m_pEditor->m_lProfiles.size());
         m_pEditor->m_lProfiles << pro;
         ProfileModel::instance()->endInsertRows();

         connect(profile, SIGNAL(changed()), this, SLOT(contactChanged()));
         PersonModel::instance()->addPerson(profile);
      }

      //Ring need a profile for all account
      setupDefaultProfile();
   }
   else {
      qDebug() << "No ProfilePersistor loaded!";
   }
}

bool ProfileContentBackend::load()
{
    if (!m_DelayedLoading) {
        m_DelayedLoading = true;
        QTimer::singleShot(0, this, SLOT(loadProfiles()));
    }
    return true;
}

bool ProfileContentBackend::reload()
{
   return false;
}

bool ProfileContentBackend::saveAll()
{
   for(Node* pro : m_pEditor->m_lProfiles) {
      editor<Person>()->save(pro->contact);
   }
   return true;
}

FlagPack<ProfileContentBackend::SupportedFeatures> ProfileContentBackend::supportedFeatures() const
{
   return SupportedFeatures::NONE
        | SupportedFeatures::LOAD        //= 0x1 <<  0, /* Load this backend, DO NOT load anything before "load" is called         */
        | SupportedFeatures::EDIT        //= 0x1 <<  2, /* Edit, but **DOT NOT**, save an item)                                    */
        | SupportedFeatures::ADD         //= 0x1 <<  4, /* Add (and save) a new item to the backend                                */
        | SupportedFeatures::SAVE_ALL    //= 0x1 <<  5, /* Save all items at once, this may or may not be faster than "add"        */
        | SupportedFeatures::REMOVE      //= 0x1 <<  7, /* Remove a single item                                                    */
   ;
}

// bool ProfileContentBackend::addContactMethod( Person* contact , ContactMethod* number)
// {
//    Q_UNUSED(contact)
//    Q_UNUSED(number)
//    return false;
// }

// QList<Person*> ProfileContentBackend::items() const
// {
//    QList<Person*> contacts;
//    for (int var = 0; var < m_pEditor->m_lProfiles.size(); ++var) {
//       contacts << m_pEditor->m_lProfiles[var]->contact;
//    }
//    return contacts;
// }

QList<Account*> ProfileEditor::getAccountsForProfile(const QString& id)
{
   QList<Account*> result;
   Node* profile = getProfileById(id.toUtf8());
   if(!profile)
      return result;

   for (Node* child : profile->children) {
      result << child->account;
   }
   return result;
}

Node* ProfileEditor::getProfileById(const QByteArray& id)
{
   for (Node* p : m_lProfiles) {
      if(p->contact->uid() == id)
         return p;
   }
   return nullptr;
}

void ProfileContentBackend::contactChanged()
{
   Person* c = qobject_cast<Person*>(sender());
   qDebug() << c->formattedName();
   qDebug() << "contactChanged!";

   if(m_needSaving) {
      m_bSaveBuffer << c;
      QTimer::singleShot(0,this,SLOT(save()));
   }
   else
      m_needSaving = true;
}

void ProfileContentBackend::save()
{
   for (Person* item : m_bSaveBuffer) {
      qDebug() << "saving:" << item->formattedName();
      editor<Person>()->save(item);
   }

   m_bSaveBuffer.clear();
   m_needSaving = false;
   load();
}

ProfileModel* ProfileModel::instance()
{
   if (!m_spInstance)
      m_spInstance = new ProfileModel(QCoreApplication::instance());
   return m_spInstance;
}

class ProfileModelPrivate final : public QObject {
   Q_OBJECT
public:
   ProfileModelPrivate(ProfileModel* parent);
   ProfileContentBackend*                 m_pProfileBackend;
   ProfilePersisterDelegate*               m_pDelegate   ;
   QStringList m_lMimes;
   QItemSelectionModel* m_pSelectionModel {nullptr};
   QItemSelectionModel* m_pSortedProxySelectionModel {nullptr};
   QSortFilterProxyModel* m_pSortedProxyModel {nullptr};

   //Helpers
   void updateIndexes();
   void regenParentIndexes();

private Q_SLOTS:
   void slotDataChanged(const QModelIndex& tl,const QModelIndex& br);
   void slotLayoutchanged();
   void slotDelayedInit();
   void slotRowsRemoved (const QModelIndex& index, int first, int last);
   void slotRowsInserted(const QModelIndex& index, int first, int last);
   void slotRowsMoved   (const QModelIndex& index, int first, int last, const QModelIndex& newPar, int newIdx);

private:
   ProfileModel* q_ptr;
};

ProfileModelPrivate::ProfileModelPrivate(ProfileModel* parent) : QObject(parent), q_ptr(parent),m_pProfileBackend(nullptr)
{

}

///Avoid creating an initialization loop
void ProfileModelPrivate::slotDelayedInit()
{
   connect(AccountModel::instance(),SIGNAL(dataChanged(QModelIndex,QModelIndex)),this,SLOT(slotDataChanged(QModelIndex,QModelIndex)));
   connect(AccountModel::instance(),&QAbstractItemModel::rowsRemoved, this, &ProfileModelPrivate::slotRowsRemoved);
   connect(AccountModel::instance(),&QAbstractItemModel::rowsInserted, this, &ProfileModelPrivate::slotRowsInserted);
   connect(AccountModel::instance(),&QAbstractItemModel::rowsMoved, this, &ProfileModelPrivate::slotRowsMoved);
   connect(AccountModel::instance(),SIGNAL(layoutChanged()),this,SLOT(slotLayoutchanged()));
}

ProfileModel::ProfileModel(QObject* parent) : QAbstractItemModel(parent), d_ptr(new ProfileModelPrivate(this))
{

   d_ptr->m_lMimes << RingMimes::PLAIN_TEXT << RingMimes::HTML_TEXT << RingMimes::ACCOUNT << RingMimes::PROFILE;

   //Creating the profile contact backend
   d_ptr->m_pProfileBackend = PersonModel::instance()->addCollection<ProfileContentBackend>(LoadOptions::FORCE_ENABLED);

   //Once LibRingClient is ready, start listening
   QTimer::singleShot(0,d_ptr,SLOT(slotDelayedInit()));
}

ProfileModel::~ProfileModel()
{
   delete d_ptr->m_pProfileBackend;
   delete d_ptr;
}

QHash<int,QByteArray> ProfileModel::roleNames() const
{
   static QHash<int, QByteArray> roles = AccountModel::instance()->roleNames();
   /*static bool initRoles = false;
   if (!initRoles) {
      initRoles = true;

   }*/
   return roles;
}

QModelIndex ProfileModel::mapToSource(const QModelIndex& idx) const
{
   if (!idx.isValid() || !idx.parent().isValid() || idx.model() != this)
      return QModelIndex();

   Node* profile = static_cast<Node*>(idx.parent().internalPointer());
   return profile->children[idx.row()]->account->index();
}

#include <unistd.h>
QModelIndex ProfileModel::mapFromSource(const QModelIndex& idx) const
{
   if (!idx.isValid() || idx.model() != AccountModel::instance())
      return QModelIndex();

   Account* acc = AccountModel::instance()->getAccountByModelIndex(idx);
   Node* pro = d_ptr->m_pProfileBackend->m_pEditor->m_hProfileByAccountId[acc->id()];

   //Something is wrong, there is an orphan
   if (!pro) {
      d_ptr->m_pProfileBackend->setupDefaultProfile();
      pro = d_ptr->m_pProfileBackend->m_pEditor->m_hProfileByAccountId[acc->id()];
   }

   return AccountModel::instance()->index(pro->m_Index,0,index(pro->parent->m_Index,0,QModelIndex()));
}

QVariant ProfileModel::data(const QModelIndex& index, int role ) const
{
   if (!index.isValid())
      return QVariant();

   Node* account_node = static_cast<Node*>(index.internalPointer());

   //Accounts
   if (account_node->account) {
      switch(role) {
         case 9999: //TODO add an Account:: role once rebased
            return account_node->m_ParentIndex;
      };
      return account_node->account->roleData(role);
   }
   //Profiles
   else {
      switch (role) {
         case Qt::DisplayRole:
            return d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles[index.row()]->contact->formattedName();
      };
   }
   return QVariant();
}

int ProfileModel::rowCount(const QModelIndex& parent ) const
{
   if (parent.isValid()) {
      Node* account_node = static_cast<Node*>(parent.internalPointer());
      return (account_node->account)?0:account_node->children.size();
   }
   return d_ptr->m_pProfileBackend->size();
}

int ProfileModel::columnCount(const QModelIndex& parent ) const
{
   Q_UNUSED(parent)
   return 1;
}

QModelIndex ProfileModel::parent(const QModelIndex& idx ) const
{
   Node* current = static_cast<Node*>(idx.internalPointer());

   if (!current)
      return QModelIndex();
   switch (current->type) {
      case Node::Type::PROFILE:
         return QModelIndex();
      case Node::Type::ACCOUNT:
         return index(current->parent->m_Index, 0, QModelIndex());
   }
   return QModelIndex();
}

QModelIndex ProfileModel::index( int row, int column, const QModelIndex& parent ) const
{
   Node* current = static_cast<Node*>(parent.internalPointer());
   if(parent.isValid() && current && !column && row < current->children.size()) {
      return createIndex(row, 0, current->children[row]);
   }
   else if(row < d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles.size() && !column) {
      return createIndex(row, 0, d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles[row]);
   }
   return QModelIndex();
}

Qt::ItemFlags ProfileModel::flags(const QModelIndex& index ) const
{
   if (!index.isValid())
      return Qt::ItemIsEnabled;

   Node* current = static_cast<Node*>(index.internalPointer());

   if (current && current->parent)
      return QAbstractItemModel::flags(index)
              | Qt::ItemIsUserCheckable
              | Qt::ItemIsEnabled
              | Qt::ItemIsSelectable
              | Qt::ItemIsDragEnabled
              | Qt::ItemIsDropEnabled;

   return QAbstractItemModel::flags(index)
         | Qt::ItemIsEnabled
         | Qt::ItemIsSelectable
         | Qt::ItemIsDragEnabled
         | Qt::ItemIsDropEnabled;
}

QStringList ProfileModel::mimeTypes() const
{
   return d_ptr->m_lMimes;
}

QMimeData* ProfileModel::mimeData(const QModelIndexList &indexes) const
{
   QMimeData *mMimeData = new QMimeData();

   for (const QModelIndex &index : indexes) {
      Node* current = static_cast<Node*>(index.internalPointer());

      if (current && index.isValid() && current->parent) {
         mMimeData->setData(RingMimes::ACCOUNT , current->account->id());
      }
      else if (index.isValid() && current) {
        mMimeData->setData(RingMimes::PROFILE , current->contact->uid());
      }
      else
         return nullptr;
   }
   return mMimeData;
}

///Return valid payload types
int ProfileModel::acceptedPayloadTypes() const
{
   return CallModel::DropPayloadType::ACCOUNT;
}

QItemSelectionModel* ProfileModel::selectionModel() const
{
   if (!d_ptr->m_pSelectionModel) {
      d_ptr->m_pSelectionModel = new QItemSelectionModel(const_cast<ProfileModel*>(this));

      connect(d_ptr->m_pSelectionModel, &QItemSelectionModel::currentChanged, [this](const QModelIndex& i) {
         const QModelIndex& accIdx = mapToSource(i);
         AccountModel::instance()->selectionModel()->setCurrentIndex(accIdx, QItemSelectionModel::ClearAndSelect);
      });
   }

   return d_ptr->m_pSelectionModel;
}

QItemSelectionModel* ProfileModel::sortedProxySelectionModel() const
{
   if (!d_ptr->m_pSortedProxySelectionModel) {
      d_ptr->m_pSortedProxySelectionModel = new QItemSelectionModel(static_cast<QSortFilterProxyModel*>(sortedProxyModel()));

      connect(d_ptr->m_pSortedProxySelectionModel, &QItemSelectionModel::currentChanged, [this](const QModelIndex& i) {
         const QModelIndex& accIdx = mapToSource(
            static_cast<QSortFilterProxyModel*>(sortedProxyModel())->mapToSource(i)
         );
         AccountModel::instance()->selectionModel()->setCurrentIndex(accIdx, QItemSelectionModel::ClearAndSelect);
      });
   }

   return d_ptr->m_pSortedProxySelectionModel;
}

QAbstractItemModel* ProfileModel::sortedProxyModel() const
{
   if (!d_ptr->m_pSortedProxyModel) {
      d_ptr->m_pSortedProxyModel = new QSortFilterProxyModel(ProfileModel::instance());
      d_ptr->m_pSortedProxyModel->setSourceModel(const_cast<ProfileModel*>(this));
      d_ptr->m_pSortedProxyModel->setSortRole(9999);
      d_ptr->m_pSortedProxyModel->sort(0);
   }

   return d_ptr->m_pSortedProxyModel;
}

void ProfileModelPrivate::updateIndexes()
{
   for (int i = 0; i < m_pProfileBackend->m_pEditor->m_lProfiles.size(); ++i) {
      m_pProfileBackend->m_pEditor->m_lProfiles[i]->m_Index = i;
      for (int j = 0; j < m_pProfileBackend->m_pEditor->m_lProfiles[i]->children.size(); ++j) {
         m_pProfileBackend->m_pEditor->m_lProfiles[i]->children[j]->m_Index = j;
      }
   }
}

bool ProfileModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
   Q_UNUSED(action)
   if((parent.isValid() && row < 0) || column > 0) {
      qDebug() << "row or column invalid";
      return false;
   }

   if (data->hasFormat(RingMimes::ACCOUNT)) {
      qDebug() << "dropping account";

      const QByteArray accountId = data->data(RingMimes::ACCOUNT);
      Node* newProfile = nullptr;
      int destIdx = 0, indexOfAccountToMove = -1; // Where to insert in account list of profile

      if(!parent.isValid() && row < d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles.size()) {
         qDebug() << "Dropping on profile title";
         qDebug() << "row:" << row;
         newProfile = d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles[row];
         destIdx = 0;
      }
      else if (parent.isValid()) {
         newProfile = static_cast<Node*>(parent.internalPointer());
         destIdx = row;
      }

      if (!newProfile)
         return false;

      Node* accountProfile = d_ptr->m_pProfileBackend->m_pEditor->m_hProfileByAccountId[accountId];
      for (Node* acc : accountProfile->children) {
         if(acc->account->id() == accountId) {
            indexOfAccountToMove = acc->m_Index;
            break;
         }
      }

      if(indexOfAccountToMove == -1) {
         qDebug() << "indexOfAccountToMove:" << indexOfAccountToMove;
         return false;
      }

      if(!beginMoveRows(index(accountProfile->m_Index, 0), indexOfAccountToMove, indexOfAccountToMove, parent, destIdx)) {
         return false;
      }

      Node* accountToMove = accountProfile->children.at(indexOfAccountToMove);
      qDebug() << "Moving:" << accountToMove->account->alias();
      accountProfile->children.remove(indexOfAccountToMove);
      accountToMove->parent = newProfile;
      d_ptr->m_pProfileBackend->m_pEditor->m_hProfileByAccountId[accountId] = newProfile;
      newProfile->children.insert(destIdx, accountToMove);
      d_ptr->updateIndexes();
      d_ptr->m_pProfileBackend->saveAll();
      endMoveRows();
   }
   else if (data->hasFormat(RingMimes::PROFILE)) {
      qDebug() << "dropping profile";
      qDebug() << "row:" << row;

      int destinationRow = -1;
      if(row < 0) {
         // drop on bottom of the list
         destinationRow = d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles.size();
      }
      else {
         destinationRow = row;
      }

      Node* moving = d_ptr->m_pProfileBackend->m_pEditor->getProfileById(data->data(RingMimes::PROFILE));

      if(!beginMoveRows(QModelIndex(), moving->m_Index, moving->m_Index, QModelIndex(), destinationRow)) {
         return false;
      }

      d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles.removeAt(moving->m_Index);
      d_ptr->m_pProfileBackend->m_pEditor->m_lProfiles.insert(destinationRow, moving);
      d_ptr->updateIndexes();
      endMoveRows();

      return true;
   }
   return false;
}

bool ProfileModel::setData(const QModelIndex& index, const QVariant &value, int role )
{
   if (!index.isValid())
      return false;

   Node* current = static_cast<Node*>(index.internalPointer());

   if (current->parent) {
      return AccountModel::instance()->setData(mapToSource(index),value,role);
   }
   return false;
}

QVariant ProfileModel::headerData(int section, Qt::Orientation orientation, int role ) const
{
   Q_UNUSED(section)
   Q_UNUSED(orientation)
   if (role == Qt::DisplayRole) return tr("Profiles");
   return QVariant();
}

// bool ProfileModel::addNewProfile(Person* c, CollectionInterface* backend)
// {
//    Q_UNUSED(backend);
//    return d_ptr->m_pProfileBackend->addNew(c);
// }

void ProfileModelPrivate::slotDataChanged(const QModelIndex& tl,const QModelIndex& br)
{
   Q_UNUSED(br)

   const QModelIndex& idx = q_ptr->mapFromSource(tl);

   emit q_ptr->dataChanged(idx,idx);
}

void ProfileModelPrivate::slotLayoutchanged()
{
   m_pProfileBackend->setupDefaultProfile();
   emit q_ptr->layoutChanged();
}

void ProfileModelPrivate::regenParentIndexes()
{
   foreach(Node* n, m_pProfileBackend->m_pEditor->m_lProfiles) {
      foreach(Node* a, n->children) {
         a->m_ParentIndex = a->account->index().row();
      }
      const QModelIndex par = q_ptr->index(n->m_Index,0,QModelIndex());
      emit q_ptr->dataChanged(q_ptr->index(0,0,par),q_ptr->index(n->children.size()-1,0,par));
   }
}

void ProfileModelPrivate::slotRowsRemoved(const QModelIndex& index, int first, int last)
{
   Q_UNUSED(index)
   Q_UNUSED(first)
   Q_UNUSED(last)
   //TODO implement removing
   regenParentIndexes();
}

void ProfileModelPrivate::slotRowsInserted(const QModelIndex& index, int first, int last)
{
   Q_UNUSED(index)
   Q_UNUSED(first)
   Q_UNUSED(last)
   //TODO implement insertion
   regenParentIndexes();
}

void ProfileModelPrivate::slotRowsMoved(const QModelIndex& index, int first, int last, const QModelIndex& newPar, int newIdx)
{
   Q_UNUSED(index)
   Q_UNUSED(first)
   Q_UNUSED(last)
   Q_UNUSED(newPar)
   Q_UNUSED(newIdx)
   regenParentIndexes();
}

Person* ProfileModel::getPerson(const QModelIndex& idx)
{
   if ((!idx.isValid()) || (idx.model() != this))
      return nullptr;

   const Node* account_node = static_cast<Node*>(idx.internalPointer());

   if (account_node->account)
      return nullptr;

   return account_node->contact;
}

#include "profilemodel.moc"
