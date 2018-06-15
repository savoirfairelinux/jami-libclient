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
#include "api/avmodel.h"

// Std
#include <chrono>
#include <iomanip>
#include <fstream>
#include <string>

// Qt
#include <QtCore/QStandardPaths>
#include <QtCore/QDir>

// LRC
#include "private/videorenderermanager.h"

namespace lrc
{

using namespace api;

class AVModelPimpl: public QObject
{
    Q_OBJECT
public:
    AVModelPimpl(AVModel& linked);
    std::string getRecordingPath() const;
    static const std::string recorderSavesSubdir;
    AVModel& linked_;
};

const std::string AVModelPimpl::recorderSavesSubdir = "sent_data";

AVModel::AVModel()
: QObject()
, pimpl_(std::make_unique<AVModelPimpl>(*this))
{
}

AVModel::~AVModel()
{
}

AVModelPimpl::AVModelPimpl(AVModel& linked)
: linked_(linked)
{
    std::srand(std::time(nullptr));
}

std::string
AVModelPimpl::getRecordingPath() const
{
    const QDir dir = QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/" + recorderSavesSubdir.c_str();
    dir.mkpath(".");
    QString savesPath = dir.path();

    std::chrono::time_point<std::chrono::system_clock> time_now = std::chrono::system_clock::now();
    std::time_t time_now_t = std::chrono::system_clock::to_time_t(time_now);
    std::tm now_tm = *std::localtime(&time_now_t);

    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y%m%d-%H%M%S");
    ss << "-";
    ss << std::rand();
    QString file = ss.str().c_str();

    QDir file_name = savesPath + "/" + file;

    return file_name.path().toStdString();
}

void AVModel::stopLocalRecorder(const std::string& path) const
{
   if (path.empty()) {
      qDebug("stopLocalRecorder: can't stop non existing recording");
      return;
   }

   VideoRendererManager::instance().stopLocalRecorder(path);
}

std::string AVModel::startLocalRecorder(const bool& audioOnly) const
{
   std::string path = pimpl_->getRecordingPath();
   return VideoRendererManager::instance().startLocalRecorder(audioOnly, path);
}
} // namespace lrc

#include "api/moc_avmodel.cpp"
#include "avmodel.moc"
