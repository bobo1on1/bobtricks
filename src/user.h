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

#ifndef USER_H
#define USER_H

#include <vector>
#include <list>
#include <string>
#include "util/timeutils.h"

class COutputUniverse;
class CUser;

class COutputMap
{
  public:
    COutputMap(COutputUniverse* outputuniverse, int priority, int instart, int outstart, int nrchannels, bool reverse, float alpha, int64_t timeout, bool usehighest)
    {
      m_outputuniverse = outputuniverse;
      m_priority = priority;
      m_instart = instart;
      m_outstart = outstart;
      m_nrchannels = nrchannels;
      m_reverse = reverse;
      m_alpha = alpha;
      m_timeout = timeout;
      m_usehighest = usehighest;

      m_outvalues.resize(m_nrchannels);

      for (int i = 0; i < m_nrchannels; i++)
      {
        m_outvalues[i] = 0;
      }

      m_lastupdate = GetTimeUs() - m_timeout;
    }

    static bool        SortByPriority(COutputMap* first, COutputMap* second) { return first->m_priority < second->m_priority; }
    void               FillBuffer(uint8_t* outbuf);

    COutputUniverse*     m_outputuniverse;
    int                  m_priority;
    int                  m_instart;
    int                  m_outstart;
    int                  m_nrchannels;
    bool                 m_reverse;
    float                m_alpha;
    std::vector<uint8_t> m_outvalues;
    int64_t              m_timeout;
    int64_t              m_lastupdate;
    bool                 m_usehighest;
};

class CUser
{
  public:
    CUser();
    ~CUser();

    void    AddOutputMap(COutputMap* outputmap);
    void    SignalUpdate();
    virtual void PreOutput() = 0;
    void    GetOutputMaps(COutputUniverse* universe, std::list<COutputMap*>& outputmaps, int64_t now);

  protected:
    std::list<COutputMap*> m_outputmaps;
};

#endif //USER_H
