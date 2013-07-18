
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

#include <string>
#include <memory>
#include <fstream>
#include "jsonsettings.h"
#include "util/misc.h"
#include "util/log.h"
#include "util/lock.h"

using namespace std;

CJSONSettings::CJSONSettings(const char* filename, const char* type, CMutex& mutex) :
  m_mutex(mutex)
{
  m_filename = filename;
  m_type = type;
}

void CJSONSettings::LoadFile(bool reload)
{
  string homepath;
  if (!GetHomePath(homepath))
  {
    LogError("Unable to get home path");
    return;
  }

  string filename = homepath + m_filename;

  Log("Loading %s settings from %s", m_type, filename.c_str());

  //lock here to make the file access atomic
  CLock lock(m_mutex);

  string* error;
  CJSONElement* json = ParseJSONFile(filename, error);
  auto_ptr<CJSONElement> jsonauto(json);

  if (error)
  {
    LogError("%s: %s", filename.c_str(), error->c_str());
    delete error;
    return;
  }

  if (!json->IsMap())
  {
    LogError("%s: invalid value for root element: %s", filename.c_str(), ToJSON(json).c_str());
    return;
  }

  LoadSettings(json->AsMap(), reload, true, filename);
}

CJSONGenerator* CJSONSettings::LoadString(const std::string& strjson, const std::string& source,
                                          bool returnsettings /*= false*/)
{
  string* error;
  CJSONElement* json = ParseJSON(strjson, error);
  auto_ptr<CJSONElement> jsonauto(json);

  //lock here to make the returned settings always match the settings that were just loaded
  CLock lock(m_mutex);

  if (error)
  {
    LogError("%s: %s", source.c_str(), error->c_str());
    delete error;
  }
  else
  {
    LoadSettings(json->AsMap(), false, false, source);
  }

  if (returnsettings)
    return SettingsToJSON(false);
  else
    return NULL;
}

void CJSONSettings::SaveFile()
{
  string homepath;
  if (!GetHomePath(homepath))
  {
    LogError("Unable to get home path");
    return;
  }

  string filename = homepath + m_filename;

  //lock here to make the file access atomic
  CLock lock(m_mutex);

  ofstream settingsfile(filename.c_str());
  if (!settingsfile.is_open())
  {
    LogError("Unable to open %s: %s", filename.c_str(), GetErrno().c_str());
    return;
  }

  Log("Saving %s settings to %s", m_type, filename.c_str());
  CJSONGenerator* generator = SettingsToJSON(true);
  settingsfile.write((const char*)generator->GetGenBuf(), generator->GetGenBufSize());

  if (settingsfile.fail())
    LogError("Error writing %s: %s", filename.c_str(), GetErrno().c_str());

  delete generator;
}

