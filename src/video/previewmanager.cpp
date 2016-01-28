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
#include "previewmanager.h"

#include "private/videorenderermanager.h"
#include "video/renderer.h"

Video::PreviewManager* Video::PreviewManager::m_spInstance = nullptr;


Video::PreviewManager* Video::PreviewManager::instance()
{
   if (!m_spInstance)
      m_spInstance = new PreviewManager();

   return m_spInstance;
}

//Getters
bool Video::PreviewManager::isPreviewing()
{
   return VideoRendererManager::instance()->isPreviewing();
}

Video::Renderer* Video::PreviewManager::previewRenderer()
{
   return VideoRendererManager::instance()->previewRenderer();
}

Video::PreviewManager::PreviewManager() : QObject(VideoRendererManager::instance())
{
   connect(VideoRendererManager::instance(), &VideoRendererManager::previewStateChanged, [this](bool startStop) {
      emit previewStateChanged(startStop);
   });
   connect(VideoRendererManager::instance(), &VideoRendererManager::previewStarted     , [this](Video::Renderer* renderer) {
      emit previewStarted(renderer);
   });
   connect(VideoRendererManager::instance(), &VideoRendererManager::previewStopped     , [this](Video::Renderer* renderer) {
      emit previewStopped(renderer);
   });
}

Video::PreviewManager::~PreviewManager()
{
   
}

void Video::PreviewManager::stopPreview()
{
   return VideoRendererManager::instance()->stopPreview();
}

void Video::PreviewManager::startPreview()
{
   return VideoRendererManager::instance()->startPreview();
}
