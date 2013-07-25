/*
 * bobtricks
 * Copyright (C) Bob 2013
 * 
 * bobtricks is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bobtricks is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "util/udpsocket.h"
#include "jsonsettings.h"
#include "universe.h"
#include "util/mutex.h"
#include <list>

class CBobTricks;

class CInputManager : public CJSONSettings
{
  public:
    CInputManager(CBobTricks& bobtricks);
    ~CInputManager();

    void                  LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source);
    void                  ParsePacket(Packet* packet);

  private:
    void                  LoadUniverse(CJSONElement* jsonuniverse, std::string source);
    CJSONGenerator*       SettingsToJSON(bool tofile);
    CUniverse*            FindUniverse(const std::string& name);
    CUniverse*            FindUniverse(const std::string& ipaddress, uint16_t portaddress);

    CMutex                m_mutex;
    std::list<CUniverse*> m_universes;
    CBobTricks&           m_bobtricks;
};

#endif //INPUTMANAGER_H
