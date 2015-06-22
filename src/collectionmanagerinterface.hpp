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

template <class T>
class CollectionManagerInterfacePrivate
{
public:
   ///All manager should be QAbstractItemModel, this wont compile (on purpose) if the aren't
   CollectionManagerInterfacePrivate(QAbstractItemModel* p2, CollectionManagerInterface<T>* p) :
   m_pMediator(nullptr),q_ptr(p2),i_ptr(p)
   {}
   ~CollectionManagerInterfacePrivate();

   QVector< CollectionInterface* > m_lCollections;
   QVector< CollectionInterface* > m_lEnabledCollections;
   mutable CollectionMediator<T>*  m_pMediator;
   QAbstractItemModel*             q_ptr;
   CollectionManagerInterface<T>*  i_ptr;

   CollectionMediator<T>* itemMediator() const;
   inline const QVector< CollectionInterface* > filterCollections(QVector< CollectionInterface* > in, FlagPack<CollectionInterface::SupportedFeatures> features) const;
};

template<class T>
CollectionMediator<T>* CollectionManagerInterfacePrivate<T>::itemMediator() const
{
   if (!m_pMediator) {
      m_pMediator = new CollectionMediator<T>(i_ptr,q_ptr);
   }
   return m_pMediator;
}

template<class T>
CollectionManagerInterfacePrivate<T>::~CollectionManagerInterfacePrivate()
{
   if (m_pMediator)
      delete m_pMediator;
}

template<class T>
template <class T2, typename ...Ts>
T2* CollectionManagerInterface<T>::addCollection(Ts... args, const LoadOptions options)
{
   T2* collection = new T2(d_ptr->itemMediator(),args...);

   //This will force the T2 to be a CollectionInterface subclass
   CollectionInterface* b = collection;
   d_ptr->m_lCollections << b;

   //This is the last time we have the class type (T2), so create a lambda
   //to keep track of the configurator type while we still can
   setCollectionConfigurator(collection,[this]() {
      return registerConfigarator<T2>();
   });

   if (options & LoadOptions::FORCE_ENABLED) { //TODO check is the collection is checked

      //Some collections can fail to load directly
      //eventually it will necessary to add an async version of this
      //to load the collection only when it is loaded
      if (collection->load())
         d_ptr->m_lEnabledCollections << collection;
   }

   registerToModel(collection);

   return collection;
}

template<class T>
template <class T2>
CollectionCreationInterface* CollectionManagerInterface<T>::registerCreator(CollectionCreationInterface* creator)
{
   static CollectionCreationInterface* cfg = nullptr;
   if (creator) {
      cfg = creator;
      addCreatorToList(creator);
   }
   return cfg;
}

template<class T>
template <class T2>
CollectionConfigurationInterface* CollectionManagerInterface<T>::registerConfigarator(CollectionConfigurationInterface* configurator)
{
   static CollectionConfigurationInterface* cfg = nullptr;
   if (configurator) {
      cfg = configurator;
      addConfiguratorToList(configurator);
   }
   return cfg;
}

template<class T>
CollectionManagerInterface<T>::CollectionManagerInterface(QAbstractItemModel* self) : d_ptr(new CollectionManagerInterfacePrivate<T>(self,this)),
m_InsertionMutex(QMutex::Recursive)
{

}

template<class T>
const QVector< CollectionInterface* > CollectionManagerInterfacePrivate<T>::filterCollections(QVector< CollectionInterface* > in, FlagPack<CollectionInterface::SupportedFeatures> features) const
{
   QVector< CollectionInterface* > out;
   for (CollectionInterface* col : in) {
      if ((col->supportedFeatures() & features) == features)
         out << col;
   }
   return out;
}

/**
 * Return the list of all collections associated with this manager
 * @param features An optional set of required features
 * @note Please note that this method complexity is O(N) when "features" is set
 */
template<class T>
const QVector< CollectionInterface* > CollectionManagerInterface<T>::collections(FlagPack<CollectionInterface::SupportedFeatures> features) const
{
   if (features != CollectionInterface::SupportedFeatures::NONE)
      return d_ptr->filterCollections(d_ptr->m_lCollections, features);
   else
      return d_ptr->m_lCollections;
}

/**
 * Return the list of all enabled collections associated with this manager
 * @param features An optional set of required features
 * @note Please note that this method complexity is O(N) when "features" is set
 */
template<class T>
const QVector< CollectionInterface* > CollectionManagerInterface<T>::enabledCollections(FlagPack<CollectionInterface::SupportedFeatures> features) const
{
   if (features != CollectionInterface::SupportedFeatures::NONE)
      return d_ptr->filterCollections(d_ptr->m_lEnabledCollections, features);
   else
      return d_ptr->m_lEnabledCollections;
}

/**
 * Return if the manager has enabled collections
 * @param features An optional set of required features
 * @note Please note that this method complexity is O(N) when "features" is set
 */
template<class T>
bool CollectionManagerInterface<T>::hasEnabledCollections(FlagPack<CollectionInterface::SupportedFeatures> features) const
{
   if (features != CollectionInterface::SupportedFeatures::NONE)
      return d_ptr->filterCollections(d_ptr->m_lEnabledCollections, features).size();
   else
      return d_ptr->m_lEnabledCollections.size();
}

/**
 * Return the list of all collections associated with this manager
 * @param features An optional set of required features
 * @note Please note that this method complexity is O(N) when "features" is set
 */
template<class T>
bool CollectionManagerInterface<T>::hasCollections(FlagPack<CollectionInterface::SupportedFeatures> features) const
{
   if (features != CollectionInterface::SupportedFeatures::NONE)
      return d_ptr->filterCollections(d_ptr->m_lCollections, features).size();
   else
      return d_ptr->m_lCollections.size();
}

template<class T>
bool CollectionManagerInterface<T>::clearAllCollections() const
{
   return false;
}

template<class T>
void CollectionManagerInterface<T>::collectionAddedCallback(CollectionInterface* collection)
{
   Q_UNUSED(collection)
}

template<class T>
bool CollectionManagerInterface<T>::deleteItem(T* item)
{
   if (item->collection()) {
      if (item->collection()->model() != reinterpret_cast<QAbstractItemModel*>(this))
         qWarning() << "Item not deleted by its parent model";
      if (item->collection()->supportedFeatures() & CollectionInterface::SupportedFeatures::REMOVE) {
         item->collection()->remove(item);
         return true;
      }
      else
         qDebug() << item << "cannot be deleted, the collection doesn't support removing items";
   }
   else
      qDebug() << item << "cannot be deleted, it has no collection";
   return false;
}

template<class T>
bool CollectionManagerInterface<T>::enableCollection( CollectionInterface*  collection, bool enabled)
{
   Q_UNUSED(enabled) //TODO implement it
   collection->load();
   return true;
}
