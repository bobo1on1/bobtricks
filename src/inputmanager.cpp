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
  CArtnetManager(bobtricks, ArtnetInput, SETTINGSFILE, "inputmanager", m_mutex)
{
}

CInputManager::~CInputManager()
{
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

