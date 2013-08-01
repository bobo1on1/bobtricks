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

#ifndef BOBTRICKS_H
#define BOBTRICKS_H

#include "util/udpsocket.h"
#include "util/inclstdint.h"
#include "outputmanager.h"
#include "inputmanager.h"
#include "scriptmanager.h"
#include "util/mutex.h"
#include <string>
#include <vector>
#include <deque>

enum ThreadMsg
{
  MsgNone,
  MsgCheckUpdates,
};

class CBobTricks
{
  public:
    CBobTricks(int argc, char *argv[]);
    ~CBobTricks();

    void Setup();
    void Process();
    void Cleanup();

    void QueueTransmit(Packet* packet);

    COutputManager& OutputManager() { return m_outputmanager; }
    CInputManager&  InputManager()  { return m_inputmanager; }

  private:

    void ProcessInputQueue();
    void ProcessPipeMessages();

    CUdpSocket          m_socket;
    std::deque<Packet*> m_outqueue;
    std::deque<Packet*> m_inqueue;
    bool                m_stop;
    int                 m_pipe[2];
    CMutex              m_mutex;

    COutputManager      m_outputmanager;
    CInputManager       m_inputmanager;
    CScriptManager      m_scriptmanager;
};

#endif //BOBTRICKS_H
