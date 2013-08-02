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

#include "outputuniverse.h"
#include "util/misc.h"
#include "util/timeutils.h"
#include "util/log.h"
#include <algorithm>

#define ARTPOLLREPLYTIMEOUT 3500000

using namespace std;

COutputUniverse::COutputUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress,
                                 bool enabled, double maxrate, uint8_t fallback) :
  CUniverse(name, portaddress, ipaddress, enabled)
{
  m_maxrate = maxrate;
  m_updated = true;
  m_lasttransmit = GetTimeUs() - Round64(1000000.0 / maxrate);
  m_fallback = fallback;
  m_process = false;
  m_presenttime = GetTimeUs() - (POLLINTERVAL + ARTPOLLREPLYTIMEOUT) * 2;
  m_waspresent = false;
}

COutputUniverse::~COutputUniverse()
{
}

void COutputUniverse::AddUser(CUser* user)
{
  m_users.push_back(user);
  m_users.sort();
  m_users.unique();
}

bool COutputUniverse::NeedsTransmit(int64_t now)
{
  if (!IsPresent(now))
  {
    if (m_waspresent)
    {
      Log("WARNING: universe \"%s\" ip:%s has disappeared", m_name.c_str(), m_ipaddress.c_str());
      m_waspresent = false;
    }
    return false;
  }

  return m_enabled && ((m_updated && now - m_lasttransmit >= Round64(1000000.0 / m_maxrate)) || now - m_lasttransmit >= 1000000);
}

int64_t COutputUniverse::MaxDelay(int64_t now)
{
  if (!m_enabled)
    return -1;

  if (!IsPresent(now))
    return -1;

  int64_t nextupdate;
  if (m_updated)
    nextupdate = m_lasttransmit + Round64(1000000.0 / m_maxrate) - now;
  else
    nextupdate = m_lasttransmit + 1000000 - now;

  return max((int64_t)0, nextupdate);
}

void COutputUniverse::GenerateOutput(int64_t now)
{
  list<COutputMap*> outputmaps;

  memset(m_channels, 0, sizeof(m_channels));

  for (list<CUser*>::iterator it = m_users.begin(); it != m_users.end(); it++)
    (*it)->GetOutputMaps(this, outputmaps, now);

  outputmaps.sort(COutputMap::SortByPriority);

  if (outputmaps.size() > 0)
  {
    for (list<COutputMap*>::iterator it = outputmaps.begin(); it != outputmaps.end(); it++)
      (*it)->FillBuffer(m_channels);
  }
  else
  {
    m_channels[0] = 0xFF;
    memset(m_channels + 1, m_fallback, sizeof(m_channels) - 1);
  }

}

Packet* COutputUniverse::ToArtNet(int64_t now)
{
  for (list<CUser*>::iterator it = m_users.begin(); it != m_users.end(); it++)
    (*it)->PreOutput();

  GenerateOutput(now);

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

  LogDebug("sending art-net packet to ipaddress:\"%s\" portaddress:%i", packet->destination.c_str(), m_portaddress);

  return packet;
}

void COutputUniverse::MarkPresent(int64_t now)
{
  if (!IsPresent(now))
  {
    Log("marking universe \"%s\" ip:%s as present", m_name.c_str(), m_ipaddress.c_str());
    m_waspresent = true;
  }

  m_presenttime = now;
}

bool COutputUniverse::IsPresent(int64_t now)
{
  return now - m_presenttime <= POLLINTERVAL + ARTPOLLREPLYTIMEOUT;
}

