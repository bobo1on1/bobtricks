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

#include "inputmanager.h"
#include "bobtricks.h"
#include "universe.h"
#include "util/log.h"
#include "util/inclstdint.h"

#define SETTINGSFILE ".bobtricks/inputs.json"

using namespace std;

CInputManager::CInputManager(CBobTricks& bobtricks) :
  CJSONSettings(SETTINGSFILE, "inputmanager", m_mutex),
  m_bobtricks(bobtricks)
{
}

CInputManager::~CInputManager()
{
}

void CInputManager::LoadSettings(JSONMap& root, bool reload, bool fromfile, const std::string& source)
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

void CInputManager::LoadUniverse(CJSONElement* jsonuniverse, std::string source)
{
  if (!jsonuniverse->IsMap())
  {
    LogError("%sinvalid value for universe: %s", source.c_str(), ToJSON(jsonuniverse).c_str());
    return;
  }

  JSONMap& universemap = jsonuniverse->AsMap();

  JSONMap::iterator itname = universemap.find("name");
  if (itname == universemap.end())
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
  JSONMap::iterator itportaddress = universemap.find("portaddress");
  if (itportaddress != universemap.end())
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

  JSONMap::iterator itipaddress = universemap.find("ipaddress");
  if (itipaddress == universemap.end())
  {
    LogError("%sipaddress missing", source.c_str());
  }
  else if (!itipaddress->second->IsString())
  {
    LogError("%sinvalid value for ipaddress: %s", source.c_str(), ToJSON(itipaddress->second).c_str());
    return;
  }

  bool enabled = true;
  JSONMap::iterator itenabled = universemap.find("enabled");
  if (itenabled != universemap.end())
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

  double alpha = 1.0;
  JSONMap::iterator italpha = universemap.find("alpha");
  if (italpha != universemap.end())
  {
    if (!italpha->second->IsNumber() || italpha->second->ToDouble() <= 0.0)
    {
      LogError("%sinvalid value for alpha: %s", source.c_str(), ToJSON(italpha->second).c_str());
      return;
    }
    else
    {
      alpha = italpha->second->ToDouble();
    }
  }

  int priority = 1000;
  JSONMap::iterator itpriority = universemap.find("priority");
  if (itpriority != universemap.end())
  {
    if (!itpriority->second->IsNumber())
    {
      LogError("%sinvalid value for priority: %s", source.c_str(), ToJSON(itpriority->second).c_str());
      return;
    }
    else
    {
      priority = itpriority->second->ToInt64();
    }
  }

  string output;
  JSONMap::iterator itoutput = universemap.find("output");
  if (itoutput == universemap.end())
  {
    LogError("%soutput missing", source.c_str());
  }
  else if (!itoutput->second->IsString())
  {
    LogError("%sinvalid value for output: %s", source.c_str(), ToJSON(itoutput->second).c_str());
    return;
  }
  output = itoutput->second->AsString();

  CUniverse* outputuni = m_bobtricks.OutputManager().FindUniverse(output);
  if (outputuni == NULL)
  {
    LogError("%soutput universe \"%s\" doesn't exist", source.c_str(), output.c_str());
    return;
  }

  Log("adding input universe \"%s\" with portaddress:%"PRIi64" ipaddress:%s enabled:%s alpha:%f output:%s priority:%i",
      name.c_str(), portaddress, itipaddress->second->AsString().c_str(),
      enabled ? "yes" : "no", alpha, output.c_str(), priority);

  CUniverse* universe = new CUniverse(name, portaddress, itipaddress->second->AsString(),
                                      enabled, alpha, output, priority, outputuni);
  m_universes.push_back(universe);
  outputuni->AddUser(universe);
}

CUniverse* CInputManager::FindUniverse(const std::string& name)
{
  for (list<CUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
  {
    if ((*it)->Name() == name)
      return *it;
  }

  return NULL;
}

CUniverse* CInputManager::FindUniverse(const std::string& ipaddress, uint16_t portaddress)
{
  for (list<CUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
  {
    if ((*it)->IpAddress() == ipaddress && (*it)->PortAddress() == portaddress)
      return *it;
  }
 
  return NULL;
}

CJSONGenerator* CInputManager::SettingsToJSON(bool tofile)
{
  return new CJSONGenerator();
}

void CInputManager::ParsePacket(Packet* packet)
{
  if (packet->data.size() < sizeof(SArtDmx))
  {
    LogDebug("received too small artnet packet of size %i", (int)packet->data.size());
    return;
  }

  SArtDmx* dmxptr = (SArtDmx*)&packet->data[0];
  if (strcmp((char*)dmxptr->ID, "Art-Net") != 0)
  {
    LogDebug("packet has invalid header");
    return;
  }

  if (dmxptr->OpCode != OpOutput)
  {
    LogDebug("packet has opcode %i, will not be handled", dmxptr->OpCode);
    return;
  }

  uint16_t portaddress = dmxptr->SubUni | (((uint16_t)dmxptr->Net) << 8);

  CUniverse* universe = FindUniverse(packet->destination, portaddress);
  if (universe == NULL)
  {
    LogDebug("packet was meant for unknown universe ip:\"%s\" portaddress:%i", packet->destination.c_str(), portaddress);
    return;
  }

  universe->FromArtNet(packet);
}

