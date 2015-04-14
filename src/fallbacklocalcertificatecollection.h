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

#include <collectioninterface.h>
#include <typedefs.h>

class FallbackLocalCertificateCollectionPrivate;
class Certificate;

template<typename T> class CollectionMediator;

/**
 * This class provide a fallback storage container for certificates used
 * locally by LibRingClient. If the platform have a certificate hosting facility,
 * it is strongly encourages to use it rather than this.
 */
class LIB_EXPORT FallbackLocalCertificateCollection : public CollectionInterface
{
public:
   explicit FallbackLocalCertificateCollection(CollectionMediator<Certificate>* mediator, const QString& path = QString());
   virtual ~FallbackLocalCertificateCollection();

   virtual bool load  () override;
   virtual bool reload() override;
   virtual bool clear () override;

   virtual QString        name     () const override;
   virtual QString        category () const override;
   virtual QVariant       icon     () const override;
   virtual bool           isEnabled() const override;
   virtual QByteArray     id       () const override;
   virtual QList<Element> listId   () const override;

   virtual SupportedFeatures  supportedFeatures() const override;

private:
   FallbackLocalCertificateCollectionPrivate* d_ptr;
   Q_DECLARE_PRIVATE(FallbackLocalCertificateCollection)
};
Q_DECLARE_METATYPE(FallbackLocalCertificateCollection*)