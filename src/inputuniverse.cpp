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

#include "inputuniverse.h"
#include "outputuniverse.h"
#include <string.h>
#include <algorithm>
#include "util/timeutils.h"
#include "util/log.h"
#include "util/lock.h"

#define INPUTTIMEOUT 1000000

using namespace std;

CInputUniverse::CInputUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled) :
  CUniverse(name, portaddress, ipaddress, enabled)
{
  m_updated = 0;
  m_lastinputtime = GetTimeUs() - INPUTTIMEOUT;
  m_lastinputport = 0;
}

CInputUniverse::~CInputUniverse()
{
}

bool CInputUniverse::FromArtNet(Packet* packet)
{
  int64_t now = GetTimeUs();

  if (now - m_lastinputtime < INPUTTIMEOUT && (m_lastinputport != packet->port || m_lastinputip != packet->source))
    return false;

  LogDebug("ip:%s port:%i delay:%"PRIi64, packet->source.c_str(), packet->port, now - m_lastinputtime);

  m_lastinputtime = now;
  m_lastinputip = packet->source;
  m_lastinputport = packet->port;

  SArtDmx* dmxptr = (SArtDmx*)&packet->data[0];

  int nrchannels = ((int)dmxptr->LengthHi << 8) | dmxptr->Length;
  int size = min(nrchannels, (int)sizeof(m_channels));
  size = min(size, (int)packet->data.size() - (int)sizeof(SArtDmx));

  if (size > 0)
  {
    memcpy(m_channels, dmxptr->Data, size);
    m_updated = 1;
    SignalUpdate();
    return true;
  }
  else
  {
    return false;
  }
}

void CInputUniverse::PreOutput()
{
  if (MsgCAS(&m_updated, 1, 0))
  {
    for (list<COutputMap*>::iterator it = m_outputmaps.begin(); it != m_outputmaps.end(); it++)
      ProcessOutputMap(*(*it));
  }
}

void CInputUniverse::ProcessOutputMap(COutputMap& outputmap)
{
  if (outputmap.m_instart < 0 && outputmap.m_instart > 511)
    return;

  int nrchannels = outputmap.m_nrchannels;
  if (outputmap.m_instart + nrchannels > 512)
    nrchannels = 512 - outputmap.m_instart;

  uint8_t* out = &outputmap.m_outvalues[0];

  if (!outputmap.m_reverse)
  {
    uint8_t* in = m_channels + outputmap.m_instart;
    memcpy(out, in, nrchannels);
  }
  else
  {
    int intop = outputmap.m_instart + nrchannels - 1;
    int inbottom = outputmap.m_instart;
    for (int i = intop; i >= inbottom + 2; i -= 3)
    {
      *(out++) = m_channels[i - 2];
      *(out++) = m_channels[i - 1];
      *(out++) = m_channels[i];
    }
  }
}

