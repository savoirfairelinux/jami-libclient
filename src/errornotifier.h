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

#pragma once

#include <QObject>

#include <typedefs.h>

enum ErrorLevel {
    INFO,
    WARNING,
    CRITICAL
};

struct LIB_EXPORT GlobalError {
    QString message;
    ErrorLevel level;
};

class LIB_EXPORT ErrorNotifier : public QObject {
    Q_OBJECT

public:
    void queueError(const QString& msg, ErrorLevel level);
    GlobalError* getFirstError();
    static ErrorNotifier* instance();
    void operator<<(GlobalError* error);

Q_SIGNALS:
    void newErrorAdded();

private:
    ErrorNotifier(QObject* parent = nullptr);
    QList<GlobalError*> errorList_;
    static ErrorNotifier* instance_;
};
