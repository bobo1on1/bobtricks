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

#ifndef ARTNETMANAGER_H
#define ARTNETMANAGER_H

#include "jsonsettings.h"
#include "universe.h"
#include "util/mutex.h"
#include <list>

class CBobTricks;

enum EMANAGERTYPE
{
  ArtnetInput,
  ArtnetOutput
};

class CArtnetManager : public CJSONSettings
{
  public:
    CArtnetManager(CBobTricks& bobtricks, EMANAGERTYPE managertype, const char* filename, const char* type, CMutex& mutex);
    ~CArtnetManager();

  protected:
    void                  LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source);
    void                  LoadUniverse(CJSONElement* jsonuniverse, std::string source);
    CUniverse*            FindUniverse(const std::string& name);
    CUniverse*            FindUniverse(const std::string& ipaddress, uint16_t portaddress);

    CBobTricks&           m_bobtricks;
    CMutex                m_mutex;
    std::list<CUniverse*> m_universes;
    EMANAGERTYPE          m_managertype;
};

#endif //ARTNETMANAGER_H
