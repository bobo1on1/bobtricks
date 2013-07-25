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

#include "outputmanager.h"
#include "bobtricks.h"
#include "util/log.h"
#include "util/timeutils.h"

#define SETTINGSFILE ".bobtricks/outputs.json"

using namespace std;

COutputManager::COutputManager(CBobTricks& bobtricks) :
  CJSONSettings(SETTINGSFILE, "outputmanager", m_mutex),
  m_bobtricks(bobtricks)
{
}

COutputManager::~COutputManager()
{
}

void COutputManager::LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source)
{
  JSONMap::iterator universes = root.find("universes");
  if (universes == root.end())
  {
    LogError("%s: universes array missing", source.c_str());
    return;
  }
  else if (!universes->second->IsArray())
  {
    LogError("%s: invalid value for universes: %s", source.c_str(), ToJSON(universes->second).c_str());
    return;
  }

  for (JSONArray::iterator it = universes->second->AsArray().begin(); it != universes->second->AsArray().end(); it++)
    LoadUniverse(*it, source + ": ");
}

CUniverse* COutputManager::FindUniverse(const std::string& name)
{
  for (list<CUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
  {
    if ((*it)->Name() == name)
      return *it;
  }

  return NULL;
}

void COutputManager::LoadUniverse(CJSONElement* jsonuniverse, std::string source)
{
  if (!jsonuniverse->IsMap())
  {
    LogError("%sinvalid value for universe: %s", source.c_str(), ToJSON(jsonuniverse).c_str());
    return;
  }

  JSONMap& universe = jsonuniverse->AsMap();

  JSONMap::iterator itname = universe.find("name");
  if (itname == universe.end())
  {
    LogError("%suniverse has no name", source.c_str());
    return;
  }
  else if (!itname->second->IsString())
  {
    LogError("%sinvalid value for universe name: %s", source.c_str(), ToJSON(itname->second).c_str());
    return;
  }

  const string& name = itname->second->AsString();

  LogDebug("Parsing settings for universe \"%s\"", name.c_str());

  //print the universe name in the source
  source += "universe \"";
  source += name;
  source += "\" ";

  if (FindUniverse(name) != NULL)
  {
    LogError("%salready exists", source.c_str());
    return; //the universe name is used as identifier, it needs to be unique
  }

  int64_t portaddress = 0;
  JSONMap::iterator itportaddress = universe.find("portaddress");
  if (itportaddress != universe.end())
  {
    if (!itportaddress->second->IsNumber() || itportaddress->second->ToInt64() < 0 || itportaddress->second->ToInt64() > 0x7FFF)
    {
      LogError("%sinvalid value for portaddress: %s", source.c_str(), ToJSON(itportaddress->second).c_str());
      return;
    }
    else
    {
      portaddress = itportaddress->second->ToInt64();
    }
  }

  JSONMap::iterator itipaddress = universe.find("ipaddress");
  if (itipaddress == universe.end())
  {
    LogError("%sipaddress missing", source.c_str());
  }
  else if (!itipaddress->second->IsString())
  {
    LogError("%sinvalid value for ipaddress: %s", source.c_str(), ToJSON(itipaddress->second).c_str());
    return;
  }

  bool enabled = true;
  JSONMap::iterator itenabled = universe.find("enabled");
  if (itenabled != universe.end())
  {
    if (!itenabled->second->IsBool())
    {
      LogError("%sinvalid value for enabled: %s", source.c_str(), ToJSON(itenabled->second).c_str());
      return;
    }
    else
    {
      enabled = itenabled->second->AsBool();
    }
  }

  double maxrate = 61.0; //allow 60 updates per second without running into the limiter
  JSONMap::iterator itmaxrate = universe.find("maxrate");
  if (itmaxrate != universe.end())
  {
    if (!itmaxrate->second->IsNumber() || itmaxrate->second->ToDouble() <= 0.0)
    {
      LogError("%sinvalid value for maxrate: %s", source.c_str(), ToJSON(itmaxrate->second).c_str());
      return;
    }
    else
    {
      maxrate = itmaxrate->second->ToDouble();
    }
  }

  Log("adding output universe \"%s\" with portaddress:%"PRIi64" ipaddress:%s enabled:%s maxrate:%f",
      name.c_str(), portaddress, itipaddress->second->AsString().c_str(), enabled ? "yes" : "no", maxrate);

  m_universes.push_back(new CUniverse(name, portaddress, itipaddress->second->AsString(), enabled, maxrate));
}

CJSONGenerator* COutputManager::SettingsToJSON(bool tofile)
{
  return new CJSONGenerator();
}

void COutputManager::Process()
{
  int64_t now = GetTimeUs();

  for (list<CUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
  {
    if ((*it)->NeedsTransmit(now))
      m_bobtricks.QueueTransmit((*it)->ToArtNet(now));
  }
}

int64_t COutputManager::MaxDelay()
{
  int64_t now = GetTimeUs();
  int64_t maxdelay = -1;
  for (list<CUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
  {
    int64_t universedelay = (*it)->MaxDelay(now);
    if (universedelay != -1 && (universedelay < maxdelay || maxdelay == -1))
      maxdelay = universedelay;
  }

  return maxdelay;
}

