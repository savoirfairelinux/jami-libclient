/****************************************************************************
 *   Copyright (C) 2014-2017 Savoir-faire Linux                          *
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
#include "rate.h"
#include "devicemodel.h"
#include "channel.h"
#include "resolution.h"
#include "../private/videorate_p.h"

Video::Rate::Rate(const Video::Resolution* res,const QString& name) :
   d_ptr(new RatePrivate())
{
   d_ptr->m_Name        = name;
   d_ptr->m_pResolution = res;
}

Video::Rate::~Rate()
{
   delete d_ptr;
}

int Video::Rate::relativeIndex()
{
   return Video::DeviceModel::instance().activeDevice()->activeChannel()->activeResolution()->validRates().indexOf(this);
}

QString Video::Rate::name() const
{
   return d_ptr->m_Name;
}
