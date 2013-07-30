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

  JSONMap::iterator itoutputs = universemap.find("outputs");
  std::vector<COutputMap*> outputmaps;
  if (itoutputs != universemap.end())
  {
    if (!itoutputs->second->IsArray())
    {
      LogError("%sinvalid value for outputs: %s", source.c_str(), ToJSON(itoutputs->second).c_str());
      return;
    }
    else
    {
      JSONArray& outputs = itoutputs->second->AsArray();
      ParseOutputs(outputs, outputmaps, name, source);
    }
  }
  else
  {
    Log("WARNING:%s no outputs defined", source.c_str());
  }

  Log("adding input universe \"%s\" with portaddress:%"PRIi64" ipaddress:%s enabled:%s",
      name.c_str(), portaddress, itipaddress->second->AsString().c_str(), enabled ? "yes" : "no");

  CInputUniverse* universe = new CInputUniverse(name, portaddress, itipaddress->second->AsString(), enabled);
  for (vector<COutputMap*>::iterator it = outputmaps.begin(); it != outputmaps.end(); it++)
  {
    universe->AddOutputMap(*it);
    (*it)->m_outputuniverse->AddUser(universe);
  }
  
  m_universes.push_back(universe);
}

void CInputManager::ParseOutputs(JSONArray& outputs, std::vector<COutputMap*>& outputmaps,
                                 const std::string& inputname, const std::string& source)
{
  for (JSONArray::iterator it = outputs.begin(); it != outputs.end(); it++)
  {
    if (!(*it)->IsMap())
    {
      LogError("%sinvalid value for output: %s", source.c_str(), ToJSON(*it).c_str());
      continue;
    }

    JSONMap& output = (*it)->AsMap();

    JSONMap::iterator ituniverse = output.find("universe");
    if (ituniverse == output.end())
    {
      LogError("%suniverse not defined in output", source.c_str());
      continue;
    }
    else if (!ituniverse->second->IsString())
    {
      LogError("%sinvalid value for universe in output: %s", source.c_str(), ToJSON(ituniverse->second).c_str());
      continue;
    }

    int64_t priority = 1000;
    if (LoadInt(output, priority, "priority", false, source) == Invalid)
      continue;

    double alpha = 1.0;
    if (LoadDouble(output, alpha, "alpha", false, source) == Invalid)
      continue;

    int64_t instart = 0;
    if (LoadInt(output, instart, "instart", false, source) == Invalid)
      continue;

    int64_t outstart = 0;
    if (LoadInt(output, outstart, "outstart", false, source) == Invalid)
      continue;

    int64_t channels = 512;
    if (LoadInt(output, channels, "channels", false, source) == Invalid)
      continue;

    bool reverse = false;
    if (LoadBool(output, reverse, "reverse", false, source) == Invalid)
      continue;

    int64_t timeout = 2000000;
    if (LoadInt(output, timeout, "timeout", false, source) == Invalid)
      continue;

    bool usehighest = false;
    if (LoadBool(output, usehighest, "usehighest", false, source) == Invalid)
      continue;

    string& universestr = ituniverse->second->AsString();
    COutputUniverse* outputuniverse = m_bobtricks.OutputManager().FindUniverse(universestr);
    if (outputuniverse == NULL)
    {
      LogError("%soutput universe \"%s\" doesn't exist", source.c_str(), universestr.c_str());
      continue;
    }

    Log("input universe \"%s\" adding output map to universe:\"%s\""
        "priority:%"PRIi64" instart:%"PRIi64" outstart:%"PRIi64" channels:%"PRIi64" reverse:%s alpha:%f timeout%"PRIi64,
        inputname.c_str(), universestr.c_str(), priority, instart, outstart, channels, reverse ? "yes" : "no", alpha, timeout);

    COutputMap* outputmap = new COutputMap(outputuniverse, priority, instart, outstart,
                                           channels, reverse, alpha, timeout, usehighest);
    outputmaps.push_back(outputmap);
  }
}

CInputManager::ParseResult CInputManager::LoadDouble(JSONMap& jsonmap, double& value, const std::string& name,
                                                     bool mandatory, const std::string& source)
{
  JSONMap::iterator itvalue = jsonmap.find(name);

  if (itvalue == jsonmap.end())
  {
    if (mandatory)
      LogError("%svalue \"%s\" missing", source.c_str(), name.c_str());
    return Missing;
  }

  if (!itvalue->second->IsNumber())
  {
    LogError("%sinvalue value for %s:%s", source.c_str(), name.c_str(), ToJSON(itvalue->second).c_str());
    return Invalid;
  }

  value = itvalue->second->ToDouble();
  return Success;
}

CInputManager::ParseResult CInputManager::LoadInt(JSONMap& jsonmap, int64_t& value, const std::string& name,
                                                  bool mandatory, const std::string& source)
{
  JSONMap::iterator itvalue = jsonmap.find(name);

  if (itvalue == jsonmap.end())
  {
    if (mandatory)
      LogError("%svalue \"%s\" missing", source.c_str(), name.c_str());
    return Missing;
  }

  if (!itvalue->second->IsNumber())
  {
    LogError("%sinvalue value for %s:%s", source.c_str(), name.c_str(), ToJSON(itvalue->second).c_str());
    return Invalid;
  }

  value = itvalue->second->ToInt64();
  return Success;
}

CInputManager::ParseResult CInputManager::LoadBool(JSONMap& jsonmap, bool& value, const std::string& name,
                                                   bool mandatory, const std::string& source)
{
  JSONMap::iterator itvalue = jsonmap.find(name);

  if (itvalue == jsonmap.end())
  {
    if (mandatory)
      LogError("%svalue \"%s\" missing", source.c_str(), name.c_str());
    return Missing;
  }

  if (!itvalue->second->IsBool())
  {
    LogError("%sinvalue value for %s:%s", source.c_str(), name.c_str(), ToJSON(itvalue->second).c_str());
    return Invalid;
  }

  value = itvalue->second->AsBool();
  return Success;
}

CInputUniverse* CInputManager::FindUniverse(const std::string& name)
{
  for (list<CInputUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
  {
    if ((*it)->Name() == name)
      return *it;
  }

  return NULL;
}

CInputUniverse* CInputManager::FindUniverse(const std::string& ipaddress, uint16_t portaddress)
{
  for (list<CInputUniverse*>::iterator it = m_universes.begin(); it != m_universes.end(); it++)
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

  CInputUniverse* universe = FindUniverse(packet->destination, portaddress);
  if (universe == NULL)
  {
    LogDebug("packet was meant for unknown universe ip:\"%s\" portaddress:%i", packet->destination.c_str(), portaddress);
    return;
  }

  LogDebug("Received packet for universe ip:\"%s\" portaddress:%i accepting for my universe \"%s\"",
      packet->destination.c_str(), portaddress, universe->Name().c_str());

  universe->FromArtNet(packet);
}

