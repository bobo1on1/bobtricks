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

#include "universe.h"
#include "util/timeutils.h"
#include "util/misc.h"
#include <string.h>
#include <algorithm>

using namespace std;

CUniverse::CUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled, double maxrate)
{
  memset(m_channels, 0, sizeof(m_channels));
  m_name = name;
  m_portaddress = portaddress;
  m_ipaddress = ipaddress;
  m_enabled = enabled;
  m_lasttransmit = GetTimeUs();
  m_maxrate = maxrate;
  m_updated = false;
  m_alpha = 0.0;
}

CUniverse::CUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled,
                     double alpha, const std::string& output, int priority, CUniverse* outputuni)
{
  memset(m_channels, 0, sizeof(m_channels));
  m_name = name;
  m_portaddress = portaddress;
  m_ipaddress = ipaddress;
  m_enabled = enabled;
  m_lasttransmit = 0;
  m_updated = false;
  m_maxrate = 0.0;
  m_alpha = alpha;
  m_output = output;
  m_priority = priority;

  m_outputmaps.push_back(new COutputMap(outputuni, 0, 512));
}

CUniverse::~CUniverse()
{
}

bool CUniverse::NeedsTransmit(int64_t now)
{
  return m_enabled && ((m_updated && now - m_lasttransmit >= Round64(1000000.0 / m_maxrate)) || now - m_lasttransmit >= 1000000);
}

int64_t CUniverse::MaxDelay(int64_t now)
{
  if (!m_enabled)
    return -1;

  int64_t nextupdate;
  if (m_updated)
    nextupdate = m_lasttransmit + Round64(1000000.0 / m_maxrate) - now;
  else
    nextupdate = m_lasttransmit + 1000000 - now;

  return max((int64_t)0, nextupdate);
}

Packet* CUniverse::ToArtNet(int64_t now)
{
  for (list<CUser*>::iterator it = m_users.begin(); it != m_users.end(); it++)
    (*it)->PreOutput();

  GenerateOutput();

  m_lasttransmit = now;
  m_updated = false;

  Packet* packet = new Packet;

  packet->destination = m_ipaddress;
  packet->port = 6454;

  packet->data.resize(sizeof(SArtDmx) + sizeof(m_channels));
  SArtDmx* dmxptr = (SArtDmx*)&packet->data[0];

  strcpy((char*)dmxptr->ID, "Art-Net");
  dmxptr->OpCode = OpOutput;
  dmxptr->ProtVerHi = 0;
  dmxptr->ProtVerLow = 14;
  dmxptr->Sequence = 0;
  dmxptr->Physical = 0;
  dmxptr->SubUni = (uint8_t)m_portaddress;
  dmxptr->Net = (m_portaddress & 0x7F00) >> 8;
  dmxptr->LengthHi = (sizeof(m_channels) & 0xFF00) >> 8;
  dmxptr->Length = (uint8_t)sizeof(m_channels);
  memcpy(dmxptr->Data, m_channels, sizeof(m_channels));

  return packet;
}

void CUniverse::AddUser(CUser* user)
{
  m_users.push_back(user);
  m_users.sort();
  m_users.unique();
}

bool CUniverse::FromArtNet(Packet* packet)
{
  SArtDmx* dmxptr = (SArtDmx*)&packet->data[0];

  int nrchannels = ((int)dmxptr->LengthHi << 8) | dmxptr->Length;
  int size = min(nrchannels, (int)sizeof(m_channels));
  size = min(size, (int)packet->data.size() - (int)sizeof(SArtDmx));

  if (size > 0)
  {
    memcpy(m_channels, dmxptr->Data, size);
    m_updated = true;
    SignalUpdate();
    return true;
  }
  else
  {
    return false;
  }
}

void CUniverse::PreOutput()
{
  if (m_updated)
  {
    float* out = &(m_outputmaps.front()->m_outvalues[0]);
    int channels = m_outputmaps.front()->m_outvalues.size();
    for (int i = 0; i < channels; i++)
      out[i] = (float)m_channels[i] / 255.0f;

    m_updated = false;
  }
}

void CUniverse::GenerateOutput()
{
  float outbuf[512];
  memset(outbuf, 0, sizeof(outbuf));

  list<CUser*> users = m_users;
  users.sort(CUser::SortByPriority);

  for (list<CUser*>::iterator it = users.begin(); it != users.end(); it++)
    (*it)->FillBuffer(this, outbuf);

  for (int i = 0; i < 512; i++)
    m_channels[i] = Round32(outbuf[i] * 255.0f);
}

