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

#ifndef OUTPUTUNIVERSE_H
#define OUTPUTUNIVERSE_H

#include "universe.h"
#include "user.h"
#include "util/udpsocket.h"

class COutputUniverse : public CUniverse
{
  public:
    COutputUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress,
                    bool enabled, double maxrate, uint8_t fallback);
    ~COutputUniverse();

    int64_t MaxDelay(int64_t now);
    bool    NeedsTransmit(int64_t now);
    Packet* ToArtNet(int64_t now);
    void    SetUpdated() { m_updated = true; }

    void    AddUser(CUser* user);

    void    GenerateOutput(int64_t now);

    void    MarkPresent(int64_t now);

    void    MarkProcess() { m_process = true; }
    bool    NeedProcess() { return m_process; }
    void    ClearProcess() { m_process = false; }

  private:
    double  m_maxrate;
    bool    m_updated;
    int64_t m_lasttransmit;
    std::list<CUser*> m_users;
    uint8_t m_fallback;
    int64_t m_presenttime;
    bool    m_process;
};

#endif //OUTPUTUNIVERSE_H
