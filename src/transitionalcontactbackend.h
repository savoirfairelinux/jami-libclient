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
#ifndef TRANSITIONAL_CONTACT_BACKEND
#define TRANSITIONAL_CONTACT_BACKEND

#include "abstractitembackend.h"

#include "typedefs.h"

class TransitionalContactBackendPrivate;

/**
 * A temporary contact backend until concrete ones are loaded
 * 
 * Some contacts are created early during the initialization process.
 * This cause race issues between initialization of remote sources,
 * such as GMail, and local one, such as history. As both contain some
 * properties related to contact, if an incomplete one is loaded before
 * the "real" one, then a temporary placeholder object have to be used.
 * 
 * This object will then be silently replaced by the "real" copy.
 * 
 * The old pointers will stay valid.
 * 
 * This backend is the default one when such scenarios happen. It can also
 * be used when contacts are created locally, but a "real" backend have
 * yet to be selected.
 */
class LIB_EXPORT TransitionalContactBackend : public AbstractContactBackend {
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
   Q_OBJECT
   #pragma GCC diagnostic pop
public:

   virtual ~TransitionalContactBackend();

   //Getters
   virtual QByteArray      id       () const override;
   virtual bool            isEnabled() const override;
   virtual QString         name     () const override;
   virtual QVariant        icon     () const override;
   virtual QList<Contact*> items    () const override;
   virtual SupportedFeatures supportedFeatures() const override;

   //Mutators
   virtual bool load       (                            ) override;
   virtual bool reload     (                            ) override;
   virtual bool append     ( const Contact* item        ) override;
   virtual bool save       ( const Contact* contact     ) override;
   virtual bool edit       ( Contact*       contact     ) override;
   virtual bool addNew     ( Contact*       contact     ) override;

   //Singleton
   static AbstractContactBackend* instance();

private:
   explicit TransitionalContactBackend(QObject* parent = nullptr);
   static AbstractContactBackend* m_spInstance;

   const QScopedPointer<TransitionalContactBackendPrivate> d_ptr;
};

#endif
