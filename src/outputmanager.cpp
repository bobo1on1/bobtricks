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
  CArtnetManager(bobtricks, ArtnetOutput, SETTINGSFILE, "outputmanager", m_mutex)
{
}

COutputManager::~COutputManager()
{
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

