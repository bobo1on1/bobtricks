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

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include "util/inclstdint.h"
#include "util/mutex.h"
#include "outputuniverse.h"
#include "jsonsettings.h"

#include <list>

class CBobTricks;

class COutputManager : public CJSONSettings
{
  public:
    COutputManager(CBobTricks& bobtricks);
    ~COutputManager();

    void             Process();
    int64_t          MaxDelay();

    void             LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source);
    COutputUniverse* FindUniverse(const std::string& name);

  private:
    void             LoadUniverse(CJSONElement* jsonuniverse, std::string source);
    CJSONGenerator*  SettingsToJSON(bool tofile);

    CBobTricks&      m_bobtricks;
    CMutex           m_mutex;

    std::list<COutputUniverse*> m_universes;
};

#endif //OUTPUTMANAGER_H
