/*
 * Copyright (C) 2020 by Savoir-faire Linux
 * Author: Andreas Traczyk <andreas.traczyk@savoirfairelinux.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <QJSEngine>
#include <QQmlEngine>
#include <QObject>

#define NS_MODELS      "net.jami.Models"
#define NS_ADAPTERS    "net.jami.Adapters"
#define NS_CONSTANTS   "net.jami.Constants"
#define NS_HELPERS     "net.jami.Helpers"
#define NS_ENUMS       "net.jami.Enums"
#define MODULE_VER_MAJ 1
#define MODULE_VER_MIN 1

class SystemTray;
class LRCInstance;
class AppSettingsManager;
class PreviewEngine;
class ScreenInfo;

// Hack for QtCreator autocomplete (part 1)
// https://bugreports.qt.io/browse/QTCREATORBUG-20569
namespace dummy {
Q_NAMESPACE
Q_CLASSINFO("RegisterEnumClassesUnscoped", "false")
} // namespace dummy

// clang-format off
#define QML_REGISTERSINGLETONTYPE_POBJECT(NS, I, N) \
    QQmlEngine::setObjectOwnership(I, QQmlEngine::CppOwnership); \
    { using T = std::remove_reference<decltype(*I)>::type; \
    qmlRegisterSingletonType<T>(NS, MODULE_VER_MAJ, MODULE_VER_MIN, N, \
                                [i=I](QQmlEngine*, QJSEngine*) -> QObject* { \
                                    return i; }); }

#define QML_REGISTERSINGLETONTYPE_CUSTOM(NS, T, P) \
    QQmlEngine::setObjectOwnership(P, QQmlEngine::CppOwnership); \
    qmlRegisterSingletonType<T>(NS, MODULE_VER_MAJ, MODULE_VER_MIN, #T, \
                                [p=P](QQmlEngine*, QJSEngine*) -> QObject* { \
                                    return p; \
                                });
// clang-format on

namespace Utils {
void registerTypes(QQmlEngine* engine,
                   SystemTray* systemTray,
                   LRCInstance* lrcInstance,
                   AppSettingsManager* appSettingsManager,
                   PreviewEngine* previewEngine,
                   ScreenInfo* screenInfo,
                   QObject* parent);
}
