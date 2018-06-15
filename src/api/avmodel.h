/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
 *   Author: Hugo Lefeuvre <hugo.lefeuvre@savoirfairelinux.com>             *
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

// std
#include <memory>
#include <string>

// Qt
#include <qobject.h>

// LRC
#include "typedefs.h"

namespace lrc
{

class AVModelPimpl;

namespace api
{

class LIB_EXPORT AVModel : public QObject {
    Q_OBJECT
public:
    AVModel();
    ~AVModel();
    /**
     * Stop local record at given path
     * @param path
     */
    void stopLocalRecorder(const std::string& path) const;
    /**
     * Start a local recorder and return it path.
     * @param audioOnly
     */
    std::string startLocalRecorder(const bool& audioOnly) const;

private:
    std::unique_ptr<AVModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
