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

#ifndef JSONSETTINGS_H
#define JSONSETTINGS_H

#include "util/mutex.h"
#include "util/JSON.h"

class CJSONSettings
{
  public:
    CJSONSettings(const char* filename, const char* type, CMutex& mutex);

    void            LoadFile(bool reload);
    CJSONGenerator* LoadString(const std::string& strjson, const std::string& source, bool returnsettings = false);

  protected:
    void            SaveFile();

  private:

    virtual void            LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source) = 0;
    virtual CJSONGenerator* SettingsToJSON(bool tofile) = 0;

    const char* m_filename;
    const char* m_type;
    CMutex&     m_mutex;
};

#endif //JSONSETTINGS_H
