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
#include "api/renderers.h"

// Models and database
#include "api/lrc.h"
//~ #include "baserender.h"
//~ #include "newshmrenderer.h"

// legacy
#include <dbus/videomanager.h>


namespace lrc
{

using namespace api;

class RenderersPimpl: public QObject
{
    Q_OBJECT
public:
    RenderersPimpl(Renderers& linked);
    ~RenderersPimpl();

    std::map<std::string, std::unique_ptr<RENDERER_TYPE>> renderers_;
    bool isPreviewing_ = false;
    Renderers& linked;

public Q_SLOTS:
    void slotStoppedDecoding(const QString& id, const QString& shmPath);
    void slotStartedDecoding(const QString& id, const QString& shmPath, int width, int height);

};

RenderersPimpl::RenderersPimpl(Renderers& linked)
: linked(linked)
{
    connect( &VideoManager::instance() , &VideoManagerInterface::startedDecoding, this, &RenderersPimpl::slotStartedDecoding);
    connect( &VideoManager::instance() , &VideoManagerInterface::stoppedDecoding, this, &RenderersPimpl::slotStoppedDecoding);
}

RenderersPimpl::~RenderersPimpl()
{

}

Renderers::Renderers()
: QObject()
, pimpl_(std::make_unique<RenderersPimpl>(*this))
{
}

Renderers::~Renderers()
{
}

RENDERER_TYPE&
Renderers::getRenderer(const std::string& callId)
{
    auto it = pimpl_->renderers_.find(callId);
    
    if (it != pimpl_->renderers_.end())
        //return *it->second;
        return std::move(std::string());
    else
        throw std::out_of_range("No render for callId: " + callId);
}

//~ NewShmRenderer*
//~ Renderers::getPreviewRenderer() const
//~ {
    //~ return nullptr;
//~ }

//~ void
//~ Renderers::removeRenderer(const std::string& rendererId)
//~ {

//~ }

void
RenderersPimpl::slotStoppedDecoding(const QString& id, const QString& shmPath)
{
    qDebug() << "@4 slotStoppedDecoding";
    return;
}


// [jn] il faut déplacer le renderer dans un thread ??
void
RenderersPimpl::slotStartedDecoding(const QString& rendererId, const QString& shmPath, int width, int height)
{
    qDebug() << "@4 slotStartedDecoding : " << shmPath;
    
    if (rendererId == "local") {
        qDebug() << "  X@X ajouter la preview (local) pour l'instant on l'ignore";
        return;
    }

//~ #ifdef ENABLE_LIBWRAP // Mac, Win32
    //~ qDebug() << "TODO: add direct renderer";
//~ #else // not ENABLE_LIBWRAP, Linux
    //~ qDebug() << "$1 : new shared mem. renderer";
//~ #endif

    //~ auto it = renderers_.find(rendererId.toStdString());

    //~ if (it == renderers_.end()) {
        //~ qDebug() << " @$ : adding renderer : " << rendererId << width << "x" << height;
        //~ renderers_[rendererId.toStdString()] = std::make_unique<RENDERER_TYPE>(shmPath.toStdString(), width, height);
    //~ }

    //~ if (renderers_[rendererId.toStdString()]->startRendering())
        //~ emit renderStarted(rendererId.toStdString());

    return;

}

//~ void
//~ Renderers::switchDevice(const std::string& rendererId)
//~ {

//~ }

//~ void
//~ Renderers::startPreview()
//~ {
    //~ if (isPreviewing())
        //~ return;

    //~ VideoManager::instance().startCamera();
//~ }

//~ void
//~ Renderers::stopPreview()
//~ {
    //~ if (not isPreviewing())
        //~ return;

    //~ VideoManager::instance().stopCamera();
//~ }

//~ bool
//~ Renderers::isPreviewing()
//~ {
    //~ return VideoManager::instance().hasCameraStarted();
//~ }

bool
Renderers::hasRenderer(const std::string& rendererId)
{
    return false;//renderers_.count(rendererId) == 1;
}

//~ lrc::NewShmRenderer&
//~ Renderers::from(const std::string& rendererId)
//~ {
    //~ auto it = renderers_.find(rendererId);
    
    //~ if (it != renderers_.end())
        //~ return *it->second;
    //~ else
        //~ throw std::out_of_range("No render for rendererId: " + rendererId);
//~ }

} // namespace lrc

#include "renderers.moc"
