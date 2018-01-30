/****************************************************************************
 *   Copyright (C) 2014-2018 Savoir-faire Linux                          *
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
#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QSize>
#include <typedefs.h>

namespace Video {
   class Rate;
   class Channel;
   class Device;
}

class VideoResolutionPrivate;

namespace Video {

///@struct Resolution Equivalent of "640x480"
class LIB_EXPORT Resolution : public QAbstractListModel {
   Q_OBJECT
   //Only Video::Device can add validated rates
   friend class Video::Device;
public:
   //Constructor
   Resolution(const QString& size, Video::Channel* chan);
   explicit Resolution();
   virtual ~Resolution();

   //Getter
   const QString             name          () const;
   int                       height        () const;
   const QList<Video::Rate*> validRates    () const;
   int                       relativeIndex () const;
   Video::Rate*              activeRate    () const;
   int                       width         () const;
   QSize                     size          () const;

   //Setters
   bool setActiveRate( Video::Rate* rate );
   bool setActiveRate( int index         );

   //Setters
   void setWidth(int width);
   void setHeight(int height);

   //Model
   virtual QVariant      data     ( const QModelIndex& index, int role = Qt::DisplayRole     ) const override;
   virtual int           rowCount ( const QModelIndex& parent = QModelIndex()                ) const override;
   virtual Qt::ItemFlags flags    ( const QModelIndex& index                                 ) const override;
   virtual bool          setData  ( const QModelIndex& index, const QVariant &value, int role)       override;


private:
   VideoResolutionPrivate* d_ptr;

};

}
