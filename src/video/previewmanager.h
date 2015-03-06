/******************************************************************************
 *   Copyright (C) 2012-2015 by Savoir-Faire Linux                            *
 *   Author : Emmanuel Lepage Vallee <emmanuel.lepage@savoirfairelinux.com>   *
 *                                                                            *
 *   This library is free software; you can redistribute it and/or            *
 *   modify it under the terms of the GNU Lesser General Public               *
 *   License as published by the Free Software Foundation; either             *
 *   version 2.1 of the License, or (at your option) any later version.       *
 *                                                                            *
 *   This library is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        *
 *   Lesser General Public License for more details.                          *
 *                                                                            *
 *   You should have received a copy of the Lesser GNU General Public License *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *****************************************************************************/
#ifndef PREVIEWMANGER_H
#define PREVIEWMANGER_H

#include <QtCore/QObject>

#include <typedefs.h>

namespace Video {

   class Renderer;

/**
 * This class is used to enable, disable and use the video preview.
 *
 * The preview can be embedded into video widgets or used for configuration.
 */
class LIB_EXPORT PreviewManager : public QObject
{
   Q_OBJECT

public:
   //Singleton
   static PreviewManager* instance();

   //Getters
   bool             isPreviewing   ();
   Video::Renderer* previewRenderer();

private:
   //Constructor
   explicit PreviewManager();
   virtual ~PreviewManager();

   //Static attributes
   static PreviewManager* m_spInstance;

public Q_SLOTS:
   void stopPreview ();
   void startPreview();

Q_SIGNALS:
   ///The preview started/stopped
   void previewStateChanged(bool startStop);
   void previewStarted(Video::Renderer* renderer);
   void previewStopped(Video::Renderer* renderer);
};

}

#endif