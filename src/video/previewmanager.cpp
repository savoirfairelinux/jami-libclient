/******************************************************************************
 *   Copyright (C) 2012-2018 Savoir-faire Linux                            *
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

#include "video/renderer.h"

Video::PreviewManager& Video::PreviewManager::instance()
{
   static auto instance = new PreviewManager();
   return *instance;
}

//Getters
bool Video::PreviewManager::isPreviewing()
{
   return false;
}

Video::Renderer* Video::PreviewManager::previewRenderer()
{
   return nullptr;
}

Video::PreviewManager::PreviewManager() : QObject()
{
}

Video::PreviewManager::~PreviewManager()
{

}

void Video::PreviewManager::stopPreview()
{
}

void Video::PreviewManager::startPreview()
{
}
