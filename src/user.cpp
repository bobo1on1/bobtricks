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
#include "universe.h"

using namespace std;

CUser::CUser()
{
  m_priority = 1000;
  m_alpha    = 1.0f;
}

CUser::~CUser()
{
}

void CUser::SignalUpdate()
{
  for (vector<COutputMap*>::iterator it = m_outputmaps.begin(); it != m_outputmaps.end(); it++)
    (*it)->m_output->SetUpdated();
}

bool CUser::SortByPriority(CUser* first, CUser* second)
{
  return first->Priority() < second->Priority();
}

void CUser::FillBuffer(CUniverse* universe, float* output)
{
  for (vector<COutputMap*>::iterator it = m_outputmaps.begin(); it != m_outputmaps.end(); it++)
  {
    if ((*it)->m_output == universe)
    {
      int start = (*it)->m_start;
      int nr = (*it)->m_outvalues.size();
      nr = min(512 - start, nr);

      float* inptr = &((*it)->m_outvalues[0]);
      float* alphaptr = &((*it)->m_alphas[0]);
      float* outptr = output + start;
      float* outptrend = outptr + nr;

      while (outptr != outptrend)
      {
        float alpha = *alphaptr * m_alpha;
        *outptr = *outptr * (1.0f - alpha) + *inptr * alpha;
        outptr++;
        inptr++;
        alphaptr++;
      }
    }
  }
}

