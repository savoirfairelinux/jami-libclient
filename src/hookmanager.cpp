/****************************************************************************
 *   Copyright (C) 2014-2018 Savoir-faire Linux                          *
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
#include "hookmanager.h"

#include <QtCore/QCoreApplication>
#include "dbus/configurationmanager.h"

class HookManagerPrivate
{
public:
   void save();

   class Names {
   public:
      constexpr static const char* PHONE_NUMBER_HOOK_ADD_PREFIX = "PHONE_NUMBER_HOOK_ADD_PREFIX";
      constexpr static const char* URLHOOK_SIP_FIELD            = "URLHOOK_SIP_FIELD"           ;
      constexpr static const char* URLHOOK_COMMAND              = "URLHOOK_COMMAND"             ;
      constexpr static const char* URLHOOK_SIP_ENABLED          = "URLHOOK_SIP_ENABLED"         ;
      constexpr static const char* PHONE_NUMBER_HOOK_ENABLED    = "PHONE_NUMBER_HOOK_ENABLED"   ;
   };

   //Attributes
   QString m_AddPrefix      ;
   QString m_SipFeild       ;
   QString m_Command        ;
   bool m_SipEnabled        ;
   bool m_ContactMethodEnabled;
};

HookManager::HookManager() : QObject(QCoreApplication::instance()),d_ptr(new HookManagerPrivate())
{
   ConfigurationManagerInterface & configurationManager = ConfigurationManager::instance();
   QMap<QString,QString> hooks = configurationManager.getHookSettings();
   d_ptr->m_AddPrefix          = hooks[HookManagerPrivate::Names::PHONE_NUMBER_HOOK_ADD_PREFIX];
   d_ptr->m_SipFeild           = hooks[HookManagerPrivate::Names::URLHOOK_SIP_FIELD           ];
   d_ptr->m_Command            = hooks[HookManagerPrivate::Names::URLHOOK_COMMAND             ];
   d_ptr->m_SipEnabled         = hooks[HookManagerPrivate::Names::URLHOOK_SIP_ENABLED         ]=="true"?true:false;
   d_ptr->m_ContactMethodEnabled = hooks[HookManagerPrivate::Names::PHONE_NUMBER_HOOK_ENABLED   ]=="true"?true:false;

}

HookManager::~HookManager()
{
}

void HookManagerPrivate::save()
{
   ConfigurationManagerInterface & configurationManager = ConfigurationManager::instance();
   QMap<QString,QString> hooks;

   hooks[HookManagerPrivate::Names::PHONE_NUMBER_HOOK_ADD_PREFIX] = m_AddPrefix;
   hooks[HookManagerPrivate::Names::URLHOOK_SIP_FIELD           ] = m_SipFeild;
   hooks[HookManagerPrivate::Names::URLHOOK_COMMAND             ] = m_Command;
   hooks[HookManagerPrivate::Names::URLHOOK_SIP_ENABLED         ] = m_SipEnabled?"true":"false";
   hooks[HookManagerPrivate::Names::PHONE_NUMBER_HOOK_ENABLED   ] = m_ContactMethodEnabled?"true":"false";
   configurationManager.setHookSettings(hooks);
}

HookManager& HookManager::instance()
{
   static auto instance = new HookManager;
   return *instance;
}

QString HookManager::prefix() const
{
   return d_ptr->m_AddPrefix;
}

QString HookManager::sipFeild() const
{
   return d_ptr->m_SipFeild;
}

QString HookManager::command() const
{
   return d_ptr->m_Command;
}

bool HookManager::isSipEnabled() const
{
   return d_ptr->m_SipEnabled;
}

bool HookManager::isContactMethodEnabled() const
{
   return d_ptr->m_ContactMethodEnabled;
}

void HookManager::setPrefix(const QString& prefix)
{
   d_ptr->m_AddPrefix = prefix;
   d_ptr->save();
}

void HookManager::setSipFeild(const QString& field)
{
   d_ptr->m_SipFeild = field;
   d_ptr->save();
}

void HookManager::setCommand(const QString& command)
{
   d_ptr->m_Command = command;
   d_ptr->save();
}

void HookManager::setSipEnabled(bool enabled)
{
   d_ptr->m_SipEnabled = enabled;
   d_ptr->save();
}

void HookManager::setContactMethodEnabled(bool enabled)
{
   d_ptr->m_ContactMethodEnabled = enabled;
   d_ptr->save();
}
