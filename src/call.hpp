/****************************************************************************
 *   Copyright (C) 2015 by Savoir-Faire Linux                               *
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

class LIB_EXPORT MediaTypeInference {
public:
   static Media::Media* safeMediaCreator(Call* c, Media::Media::Type t, Media::Media::Direction d);

   template<typename T>
   static inline Media::Media::Type getType() {
      const int id = MediaTypeInference::getId<T>();
      return MediaTypeInference::typeMap(!MediaTypeInference::typeMap().contains(id))[id];
   }

   template<typename T>
   static int getId() {
      static int id = genId();
      return id;
   }
   static QHash<int, Media::Media::Type>& typeMap(bool regen = false);
private:
   static int genId();
};

/**
 * Perform a safe cast of the first media of "T" type
 * @example Media::Audio* audio = call->firstMedia<Media::Audio>(Media::Media::Direction::OUT);
 * @return nullptr if none, the media otherwise.
 */
template<typename T>
T* Call::firstMedia(Media::Media::Direction direction) const
{
   const Media::Media::Type t = MediaTypeInference::getType<T>();

   QList<Media::Media*> ms = media(t, direction);

   if (!ms.isEmpty()) {
      Media::Media* m = ms[0];
      Q_ASSERT(m->type() == t);

      return reinterpret_cast<T*>(m);
   }

   return nullptr;
}

template<typename T>
T* Call::addOutgoingMedia(bool useExisting)
{
   T* existing = firstMedia<T>(Media::Media::Direction::OUT);

   if (useExisting && existing)
      return existing;

   Media::Media::Type t = MediaTypeInference::getType<T>();

   return static_cast<T*>(MediaTypeInference::safeMediaCreator(this,t,Media::Media::Direction::OUT));
}

#define REGISTER_MEDIA() static auto forceType = [] {\
  QHash<int,::Media::Media::Type>& sTypeMap = MediaTypeInference::typeMap(0);\
  sTypeMap[MediaTypeInference::getId<Media::Audio>()] = ::Media::Media::Type::AUDIO;\
  sTypeMap[MediaTypeInference::getId<Media::Video>()] = ::Media::Media::Type::VIDEO;\
  sTypeMap[MediaTypeInference::getId<Media::Text >()] = ::Media::Media::Type::TEXT ;\
  sTypeMap[MediaTypeInference::getId<Media::File >()] = ::Media::Media::Type::FILE ;\
  return 0;\
}();
