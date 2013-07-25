/*
 * bobdsp
 * Copyright (C) Bob 2013
 * 
 * bobdsp is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bobdsp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCRIPTMANAGER_H
#define SCRIPTMANAGER_H

#include "jsonsettings.h"
#include "script.h"
#include "util/thread.h"
#include "util/mutex.h"
#include <list>

class CBobTricks;

class CScriptManager : public CJSONSettings, public CThread
{
  public:
    CScriptManager(CBobTricks& bobtricks);
    ~CScriptManager();

  private:
    void                Process();
    void                LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source);
    void                LoadScript(CJSONElement* jsonscript, std::string source);
    CJSONGenerator*     SettingsToJSON(bool tofile);

    CBobTricks&         m_bobtricks;
    CMutex              m_mutex;

    std::list<CScript*> m_scripts;
};

#endif //SCRIPTMANAGER_H
