/****************************************************************************
 *   Copyright (C) 2012-2018 Savoir-faire Linux                          *
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
#include "mime.h"

//Qt
#include <QtCore/QMimeData>
#include <QtCore/QTextStream>
#include <QtCore/QDateTime>

//LRC
#include <person.h>
#include <contactmethod.h>
#include <numbercategory.h>

/**
 * This method enable inter-operability with various third party applications
 * by converting Ring objects into various standard MIME representations.
 *
 * This is designed to be an unified algorithm for drag and drop and copy and
 * paste. It replace no less than 17 chunks of code (LRC + KDE client) that
 * either all did the same thing or were missing parts.
 *
 * This methods need to be accessed directly in same specific scenarios,
 * otherwise, helper methods "mimePayload() const" are available for
 * Call, Person and ContactMethod.
 *
 * @note If new MIME are ever to be supported, this algorithm has to be
 * updated instead of hardcoding the payload in a random file.
 */
QMimeData* RingMimes::payload(const void* c, const ContactMethod* cm, const Person* p)
{
    return nullptr;
}
