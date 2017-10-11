/****************************************************************************
 *   Copyright (C) 2017 Savoir-faire Linux                                  *
 *   Author: Nicolas Jäger <nicolas.jager@savoirfairelinux.com>             *
 *   Author: Sébastien Blin <sebastien.blin@savoirfairelinux.com>           *
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

// Std
#include <memory>

// Qt
#include <qobject.h>

// Lrc
#include "renderers.h"

namespace lrc
{

namespace api
{
class Lrc;
}

class BaseRender : public QObject {
    Q_OBJECT

public:
    BaseRender(int width, int height);
    ~BaseRender();

   virtual bool isRendering() const;
   virtual lrc::api::video::Frame currentFrame() const;
   virtual lrc::api::video::ColorSpace colorSpace() const;
   
   virtual void stopRendering();
   virtual bool startRendering();
   int getWidth();
   int getHeight();

Q_SIGNALS:
   void frameUpdated();
   void stopped();
   void started();

//~ private Q_SLOTS:

protected:
    int width_;
    int height_;
};

} // namespace lrc
