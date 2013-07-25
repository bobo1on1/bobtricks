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
#include "util/JSON.h"
#include "inputuniverse.h"
#include "util/mutex.h"
#include <list>

class CBobTricks;

class CInputManager : public CJSONSettings
{
  public:
    CInputManager(CBobTricks& bobtricks);
    ~CInputManager();

    void            LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source);
    void            ParsePacket(Packet* packet);

  private:
    void            LoadUniverse(CJSONElement* jsonuniverse, std::string source);
    CJSONGenerator* SettingsToJSON(bool tofile);
    CInputUniverse* FindUniverse(const std::string& name);
    CInputUniverse* FindUniverse(const std::string& ipaddress, uint16_t portaddress);
    void            ParseOutputs(JSONArray& outputs, std::vector<COutputMap*>& outputmaps,
                                 const std::string& inputname, const std::string& source);

    enum ParseResult
    {
      Missing = -1,
      Invalid = 0,
      Success = 1
    };

    ParseResult     LoadDouble(JSONMap& jsonmap, double& value, const std::string& name, bool mandatory, const std::string& source);
    ParseResult     LoadInt(JSONMap& jsonmap, int64_t& value, const std::string& name, bool mandatory, const std::string& source);
    ParseResult     LoadBool(JSONMap& jsonmap, bool& value, const std::string& name, bool mandatory, const std::string& source);

    CBobTricks&     m_bobtricks;
    CMutex          m_mutex;
    std::list<CInputUniverse*> m_universes;
};

#endif //INPUTMANAGER_H
