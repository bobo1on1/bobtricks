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

class COutputUniverse;
class CUser;

class COutputMap
{
  public:
    COutputMap(COutputUniverse* outputuniverse, int priority, int instart, int outstart, int nrchannels, bool reverse, float alpha)
    {
      m_outputuniverse = outputuniverse;
      m_priority = priority;
      m_instart = instart;
      m_outstart = outstart;
      m_nrchannels = nrchannels;
      m_reverse = reverse;
      m_alpha = alpha;

      m_outvalues.resize(m_nrchannels);
      m_alphas.resize(m_nrchannels);

      for (int i = 0; i < m_nrchannels; i++)
      {
        m_outvalues[i] = 0.0f;
        m_alphas[i] = 1.0f;
      }
    }

    static bool        SortByPriority(COutputMap* first, COutputMap* second) { return first->m_priority < second->m_priority; }
    void               FillBuffer(float* outbuf);

    COutputUniverse*   m_outputuniverse;
    int                m_priority;
    int                m_instart;
    int                m_outstart;
    int                m_nrchannels;
    bool               m_reverse;
    float              m_alpha;
    std::vector<float> m_outvalues;
    std::vector<float> m_alphas;
};

class CUser
{
  public:
    CUser();
    ~CUser();

    void    AddOutputMap(COutputMap* outputmap);
    void    SignalUpdate();
    virtual void PreOutput() = 0;
    void    GetOutputMaps(COutputUniverse* universe, std::list<COutputMap*>& outputmaps);

  protected:
    std::list<COutputMap*> m_outputmaps;
};

#endif //USER_H
