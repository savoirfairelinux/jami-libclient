/****************************************************************************
 *   Copyright (C) 2015-2016 by Savoir-faire Linux                               *
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
   static media::Media* safeMediaCreator(Call* c, media::Media::Type t, media::Media::Direction d);

   template<typename T>
   static inline media::Media::Type getType() {
      const int id = MediaTypeInference::getId<T>();
      return MediaTypeInference::typeMap(!MediaTypeInference::typeMap().contains(id))[id];
   }

   template<typename T>
   static int getId() {
      static int id = genId();
      return id;
   }
   static QHash<int, media::Media::Type>& typeMap(bool regen = false);
private:
   static int genId();
};

/**
 * Perform a safe cast of the first media of "T" type
 * @example Media::Audio* audio = call->firstMedia<media::Audio>(media::Media::Direction::OUT);
 * @return nullptr if none, the media otherwise.
 */
template<typename T>
T* Call::firstMedia(media::Media::Direction direction) const
{
   const media::Media::Type t = MediaTypeInference::getType<T>();

   QList<media::Media*> ms = media(t, direction);

   if (!ms.isEmpty()) {
      media::Media* m = ms[0];
      Q_ASSERT(m->type() == t);

      return reinterpret_cast<T*>(m);
   }

   return nullptr;
}

template<typename T>
T* Call::addOutgoingMedia(bool useExisting)
{
   #pragma push_macro("OUT")
   #undef OUT
   T* existing = firstMedia<T>(media::Media::Direction::OUT);
   if (useExisting && existing)
      return existing;

   media::Media::Type t = MediaTypeInference::getType<T>();

   return static_cast<T*>(MediaTypeInference::safeMediaCreator(this,t, media::Media::Direction::OUT));
   #pragma pop_macro("OUT")
}

#ifdef _MSC_VER
#define __attribute__(A) /*do nothing*/
#endif // _MSC_VER

#define REGISTER_MEDIA() __attribute__ ((unused)) static auto forceType = [] {\
  QHash<int,::media::Media::Type>& sTypeMap = MediaTypeInference::typeMap(0);\
  sTypeMap[MediaTypeInference::getId<media::Audio>()] = ::media::Media::Type::AUDIO;\
  sTypeMap[MediaTypeInference::getId<media::Video>()] = ::media::Media::Type::VIDEO;\
  sTypeMap[MediaTypeInference::getId<media::Text >()] = ::media::Media::Type::TEXT ;\
  sTypeMap[MediaTypeInference::getId<media::File >()] = ::media::Media::Type::FILE ;\
  return 0;\
}();
