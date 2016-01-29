/***************************************************************************
 * Copyright (C) 2016 by Savoir-faire Linux                                *
 * Author: Edric Ladent Milaret <edric.ladent-milaret@savoirfairelinux.com>*
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify    *
 * it under the terms of the GNU General Public License as published by    *
 * the Free Software Foundation; either version 3 of the License, or       *
 * (at your option) any later version.                                     *
 *                                                                         *
 * This program is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License       *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.   *
 **************************************************************************/

#include "errornotifier.h"

ErrorNotifier* ErrorNotifier::instance_ = nullptr;

ErrorNotifier* ErrorNotifier::instance()
{
    if (not instance_)
        instance_ = new ErrorNotifier();
    return instance_;
}

ErrorNotifier::ErrorNotifier(QObject* parent)
{
    Q_UNUSED(parent)
}

void
ErrorNotifier::queueError(const QString& msg, ErrorLevel level)
{
    errorList_.append(new GlobalError{msg, level});
    emit newErrorAdded();
}

GlobalError*
ErrorNotifier::getFirstError()
{
    if (not errorList_.isEmpty())
        return errorList_.takeFirst();
    return nullptr;
}

void
ErrorNotifier::operator<<(GlobalError* error)
{
    errorList_.append(error);
    emit newErrorAdded();
}
