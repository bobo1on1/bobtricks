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

#ifndef INPUTUNIVERSE_H
#define INPUTUNIVERSE_H

#include "universe.h"
#include "user.h"
#include "util/udpsocket.h"
#include "util/atomic.h"
#include "util/mutex.h"
#include <string>

class COutputUniverse;

class CInputUniverse : public CUniverse, public CUser
{
  public:
    CInputUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled);
    ~CInputUniverse();

    bool        FromArtNet(Packet* packet);
    void        PreOutput();

  private:
    void        ProcessOutputMap(COutputMap& outputmap);

    std::string m_output;
    atom        m_updated;
    std::string m_lastinputip;
    int64_t     m_lastinputtime;
    uint16_t    m_lastinputport;
    CMutex      m_mutex;
};

#endif //INPUTUNIVERSE_H
