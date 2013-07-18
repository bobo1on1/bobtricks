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
#include <string>

class CUniverse;

class COutputMap
{
  public:
    COutputMap(CUniverse* output, int start, int nrchannels)
    {
      m_output = output;
      m_start = start;

      m_outvalues.resize(nrchannels);
      m_alphas.resize(nrchannels);

      for (int i = 0; i < nrchannels; i++)
      {
        m_outvalues[i] = 0.0f;
        m_alphas[0] = 1.0f;
      }
    }

    CUniverse*         m_output;
    int                m_start;
    std::vector<float> m_outvalues;
    std::vector<float> m_alphas;
};

class CUser
{
  public:
    CUser();
    ~CUser();

    int   Priority() { return m_priority; }
    float Alpha()    { return m_alpha;    }
    void  SignalUpdate();
    virtual void PreOutput() = 0;
    void  FillBuffer(CUniverse* universe, float* output);

    static bool SortByPriority(CUser* first, CUser* second);

  protected:
    int   m_priority;
    float m_alpha;

    std::vector<COutputMap*> m_outputmaps;
};

#endif //USER_H
