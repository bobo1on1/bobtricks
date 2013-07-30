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

using namespace std;

CInputUniverse::CInputUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled) :
  CUniverse(name, portaddress, ipaddress, enabled)
{
  m_updated = false;
}

CInputUniverse::~CInputUniverse()
{
}

bool CInputUniverse::FromArtNet(Packet* packet)
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

void CInputUniverse::PreOutput()
{
  if (m_updated)
  {
    for (list<COutputMap*>::iterator it = m_outputmaps.begin(); it != m_outputmaps.end(); it++)
      ProcessOutputMap(*(*it));

    m_updated = false;
  }
}

void CInputUniverse::ProcessOutputMap(COutputMap& outputmap)
{
  if (outputmap.m_instart < 0 && outputmap.m_instart > 511)
    return;

  int nrchannels = outputmap.m_nrchannels;
  if (outputmap.m_instart + nrchannels > 512)
    nrchannels = 512 - outputmap.m_instart;

  float* out = &outputmap.m_outvalues[0];

  if (!outputmap.m_reverse)
  {
    uint8_t* in = m_channels + outputmap.m_instart;
    uint8_t* inend = in + nrchannels;

    while (in != inend)
      *(out++) = ((float)*(in++)) * (1.0f / 255.0f);
  }
  else
  {
    uint8_t* in = m_channels + outputmap.m_instart + nrchannels - 1;
    uint8_t* inend = in - nrchannels;

    while (in != inend)
      *(out++) = ((float)*(in--)) * (1.0f / 255.0f);
  }
}
