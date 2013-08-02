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

#include "user.h"
#include "outputuniverse.h"
#include "util/log.h"
#include <algorithm>
#include <string.h>
#include "util/lock.h"

using namespace std;

void COutputMap::FillBuffer(uint8_t* outbuf)
{
  CLock lock(m_mutex);
  if (m_outstart < 0 || m_outstart > 511)
    return;

  int nrchannels = m_nrchannels;
  if (m_outstart + nrchannels > 512)
    nrchannels = 512 - m_outstart;

  uint8_t* outptr = outbuf + m_outstart;
  uint8_t* outend = outptr + nrchannels;
  uint8_t* inptr = &m_outvalues[0];
  //float* alphaptr = &m_alphas[0];

  if (!m_usehighest)
  {
    memcpy(outptr, inptr, nrchannels);
  }
  else
  {
    while (outptr != outend)
    {
      *outptr = max(*inptr, *outptr);
      outptr++;
      inptr++;
    }
  }
}

CUser::CUser()
{
}

CUser::~CUser()
{
}

void CUser::AddOutputMap(COutputMap* outputmap)
{
  m_outputmaps.push_back(outputmap);
}

void CUser::SignalUpdate()
{
  int64_t now = GetTimeUs();

  for (list<COutputMap*>::iterator it = m_outputmaps.begin(); it != m_outputmaps.end(); it++)
  {
    LogDebug("Signaling update of output universe \"%s\"", (*it)->m_outputuniverse->Name().c_str());
    (*it)->m_outputuniverse->SetUpdated();
    (*it)->m_lastupdate = now;
  }
}

void CUser::GetOutputMaps(COutputUniverse* universe, std::list<COutputMap*>& outputmaps, int64_t now)
{
  for (list<COutputMap*>::iterator it = m_outputmaps.begin(); it != m_outputmaps.end(); it++)
  {
    if ((*it)->m_outputuniverse == universe && ((*it)->m_timeout <= 0 || now - (*it)->m_lastupdate < (*it)->m_timeout))
      outputmaps.push_back(*it);
  }
}

