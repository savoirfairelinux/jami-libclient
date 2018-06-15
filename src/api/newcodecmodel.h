/****************************************************************************
 *   Copyright (C) 2018 Savoir-faire Linux                                  *
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
#include <string>
#include <list>

// Qt
#include <qobject.h>
#include <QObject>

// Lrc
#include "api/account.h"
#include "typedefs.h"

namespace lrc
{

class CallbacksHandler;
class NewCodecModelPimpl;

namespace api
{

namespace account { struct Info; }

struct Codec
{
    unsigned int id;
    bool enabled;
    std::string name;
    std::string samplerate;
    std::string bitrate;
    std::string min_bitrate;
    std::string max_bitrate;
    std::string type;
    std::string quality;
    std::string min_quality;
    std::string max_quality;
    bool auto_quality_enabled;
};

/**
  *  @brief Class that manages ring devices for an account
  */
class LIB_EXPORT NewCodecModel : public QObject {
    Q_OBJECT

public:
    const account::Info& owner;

    NewCodecModel(const account::Info& owner, const CallbacksHandler& callbacksHandler);
    ~NewCodecModel();

    /**
     * @return audio codecs for the account
     */
    std::list<Codec> getAudioCodecs() const;
    /**
     * @return video codecs for the account
     */
    std::list<Codec> getVideoCodecs() const;
    /**
     * Set a higher priority to a codec
     * @param codecId
     */
    void increasePriority(const unsigned int& codecid);
    /**
     * Set a lower priority to a codec
     * @param codecId
     */
    void decreasePriority(const unsigned int& codecid);
    /**
     * Enable a codec
     * @param codecId
     * @param enable true if enabled else false
     */
    void enable(const unsigned int& codecid, bool enabled);

private:
    std::unique_ptr<NewCodecModelPimpl> pimpl_;
};

} // namespace api
} // namespace lrc
