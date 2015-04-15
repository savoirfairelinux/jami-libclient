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
#ifndef FOLDERCERTIFICATECOLLECTION_H
#define FOLDERCERTIFICATECOLLECTION_H

#include <collectioninterface.h>
#include <typedefs.h>

class FolderCertificateCollectionPrivate;
class Certificate;

template<typename T> class CollectionMediator;

/**
 * This class provide a fallback storage container for certificates used
 * locally by LibRingClient. If the platform have a certificate hosting facility,
 * it is strongly encourages to use it rather than this.
 */
class LIB_EXPORT FolderCertificateCollection : public CollectionInterface
{
public:
   ///@enum Options load and behavior parameters for a FolderCertificateCollection
   enum class Options {
      READ_WRITE = 0 << 0, /*!< Try to add certificates to that store (default)   */
      READ_ONLY  = 1 << 0, /*!< Do not try to add certificate to that store       */
      RECURSIVE  = 1 << 1, /*!< Read all sub-folders recursively                  */
      TOP_LEVEL  = 1 << 2, /*!< Consider those certificates as top-level entities */
   };

   explicit FolderCertificateCollection(CollectionMediator<Certificate>* mediator,
                                          const QString& path              = QString(),
                                          const FlagPack<Options>& options = Options::READ_WRITE,
                                          const QString& name              = QString()
                                       );
   virtual ~FolderCertificateCollection();

   virtual bool load  () override;
   virtual bool reload() override;
   virtual bool clear () override;

   virtual QString        name     () const override;
   virtual QString        category () const override;
   virtual QVariant       icon     () const override;
   virtual bool           isEnabled() const override;
   virtual QByteArray     id       () const override;
   virtual QList<Element> listId   () const override;

   virtual FlagPack<SupportedFeatures> supportedFeatures() const override;

private:
   FolderCertificateCollectionPrivate* d_ptr;
   Q_DECLARE_PRIVATE(FolderCertificateCollection)
};
Q_DECLARE_METATYPE(FolderCertificateCollection*)
DECLARE_ENUM_FLAGS(FolderCertificateCollection::Options)

#endif
