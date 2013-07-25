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

#include "scriptmanager.h"
#include "bobtricks.h"
#include "util/log.h"
#include <string>
#include <vector>

#define SETTINGSFILE ".bobtricks/scripts.json"

using namespace std;

CScriptManager::CScriptManager(CBobTricks& bobtricks) :
  CJSONSettings(SETTINGSFILE, "scriptmanager", m_mutex),
  m_bobtricks(bobtricks)
{
}

CScriptManager::~CScriptManager()
{
}

void CScriptManager::Process()
{
  while (!m_stop)
  {
    for (list<CScript*>::iterator it = m_scripts.begin(); it != m_scripts.end(); it++)
    {
      if (!(*it)->Loaded() && !(*it)->LoadFailed())
        (*it)->Load();
    }

    sleep(1);
  }
}

void CScriptManager::LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source)
{
  JSONMap::iterator scripts = root.find("scripts");

  if (scripts == root.end())
  {
    LogError("%s: scripts array missing", source.c_str());
    return;
  }
  else if (!scripts->second->IsArray())
  {
    LogError("%s: invalid value for scripts: %s", source.c_str(), ToJSON(scripts->second).c_str());
    return;
  }

  for (JSONArray::iterator it = scripts->second->AsArray().begin(); it != scripts->second->AsArray().end(); it++)
    LoadScript(*it, source + ": ");
}

void CScriptManager::LoadScript(CJSONElement* jsonscript, std::string source)
{
  if (!jsonscript->IsMap())
  {
    LogError("%sinvalid value for script: %s", source.c_str(), ToJSON(jsonscript).c_str());
    return;
  }

  JSONMap& script = jsonscript->AsMap();

  JSONMap::iterator itname = script.find("name");
  if (itname == script.end())
  {
    LogError("%sno name defined for script", source.c_str());
    return;
  }
  else if (!itname->second->IsString())
  {
    LogError("%sinvalid value for script name: %s", source.c_str(), ToJSON(itname->second).c_str());
    return;
  }

  string& name = itname->second->AsString();

  JSONMap::iterator itfilename = script.find("filename");
  if (itfilename == script.end())
  {
    LogError("%sno filename defined for script \"%s\"", source.c_str(), name.c_str());
    return;
  }
  else if (!itfilename->second->IsString())
  {
    LogError("%sinvalid value for filename of script \"%s\": %s", source.c_str(), name.c_str(), ToJSON(itfilename->second).c_str());
    return;
  }

  JSONMap::iterator itparameters = script.find("parameters");
  if (itparameters != script.end())
  {
    if (!itparameters->second->IsMap())
    {
      LogError("%sinvalid value for parameters of script \"%s\": %s", source.c_str(), name.c_str(), ToJSON(itparameters->second).c_str());
      return;
    }
  }

  JSONMap& parametersmap = itparameters->second->AsMap();
  vector<CParameter> parameters;
  for (JSONMap::iterator it = parametersmap.begin(); it != parametersmap.end(); it++)
  {
    if (!it->second->IsNumber())
    {
      LogError("%sscript \"%s\" invalid value for parameter \"%s\": %s", name.c_str(),
               source.c_str(), it->first.c_str(), ToJSON(it->second).c_str());
      continue;
    }

    double parmvalue = it->second->ToDouble();

    LogDebug("script \"%s\" found parameter \"%s\" value %f", name.c_str(), it->first.c_str(), parmvalue);

    parameters.push_back(CParameter(it->first, parmvalue));
  }

  Log("loading script \"%s\" instance \"%s\"", itfilename->second->AsString().c_str(), name.c_str());
  CScript* scriptptr = new CScript(name, itfilename->second->AsString(), parameters);
  m_scripts.push_back(scriptptr);
}

CJSONGenerator* CScriptManager::SettingsToJSON(bool tofile)
{
  return NULL;
}

