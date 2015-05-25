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
#ifndef RINGTONE_H
#define RINGTONE_H

#include <typedefs.h>
#include <itembase.h>

class RingtonePrivate;

class LIB_EXPORT Ringtone : public ItemBase<QObject>
{
   Q_OBJECT
public:
   Ringtone(QObject* parent = nullptr);
   virtual ~Ringtone();

   //Getter
   QUrl    path     () const;
   QString name     () const;
   bool    isPlaying() const;

   //Setter
   void setPath     (const QUrl&    url  );
   void setName     (const QString& name );
   void setIsPlaying(const bool     value);

private:
   RingtonePrivate* d_ptr;
   Q_DECLARE_PRIVATE(Ringtone)
};

#endif