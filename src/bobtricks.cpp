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

#ifndef _GNU_SOURCE
  #define _GNU_SOURCE //for pipe2
#endif //_GNU_SOURCE

#include "bobtricks.h"
#include "util/log.h"
#include "util/misc.h"
#include "util/timeutils.h"
#include "util/lock.h"
#include <unistd.h>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/select.h>
#include <algorithm>
#include <getopt.h>

using namespace std;

CBobTricks::CBobTricks(int argc, char *argv[]) :
  m_outputmanager(*this),
  m_inputmanager(*this),
  m_scriptmanager(*this)
{
  g_printdebuglevel = false;
  m_stop            = false;
  m_process         = false;

  struct option longoptions[] =
  {
   {"debug", no_argument, NULL, 'd'},
   {0, 0, 0, 0}
  };

  const char* shortoptions = "d";
  int         optionindex = 0;
  int         c;

  while ((c = getopt_long(argc, argv, shortoptions, longoptions, &optionindex)) != -1)
  {
    if (c == 'd')
    {
      g_printdebuglevel = true;
    }
    else if (c == '?')
    {
      exit(1);
    }
  }

  if (pipe2(m_pipe, O_NONBLOCK) == -1)
  {
    LogError("creating msg pipe %i:%s", errno, GetErrno().c_str());
    exit(1);
  }
}

CBobTricks::~CBobTricks()
{
}

void CBobTricks::Setup()
{
  SetLogFile("bobtricks.log");
  m_outputmanager.LoadFile(false);
  m_inputmanager.LoadFile(false);
  m_outputmanager.StartThread();
  StartThread();
  //m_scriptmanager.LoadFile(false);
  //m_scriptmanager.StartThread();
}

void CBobTricks::Run()
{
  while(!m_stop)
  {
    if (!m_socket.IsOpen())
      m_socket.Open(6454);

    int maxfds = max(m_pipe[0], m_socket.GetSocket()) + 1;
    fd_set readset;
    FD_ZERO(&readset);
    FD_SET(m_pipe[0], &readset);
    if (m_socket.IsOpen())
      FD_SET(m_socket.GetSocket(), &readset);

    fd_set writeset;
    FD_ZERO(&writeset);
    if (m_socket.IsOpen() && !m_outqueue.empty())
      FD_SET(m_socket.GetSocket(), &writeset);

    int64_t maxdelay = m_outputmanager.MaxDelay();
    timeval timeout = {};
    if (m_socket.IsOpen() && maxdelay >= 0)
    {
      timeout.tv_sec = maxdelay / 1000000;
      timeout.tv_usec = maxdelay % 1000000;
    }
    else
    {
      timeout.tv_sec = 0;
      timeout.tv_usec = 1000000;
    }

    int returnv = select(maxfds, &readset, &writeset, NULL, &timeout);
    if (returnv == -1)
    {
      LogError("select() %i:%s", errno, GetErrno().c_str());
      if (errno != EINTR)
        sleep(1); //don't busy spin
      continue;
    }

    if (m_socket.IsOpen())
    {
      if (FD_ISSET(m_socket.GetSocket(), &readset))
      {
        int64_t start = GetTimeUs();
        do
        {
          Packet* packet = new Packet;
          if (m_socket.GetMessage(*packet))
          {
            CLock lock(m_condition);
            m_inqueue.push_back(packet);
            if (!m_process)
            {
              m_process = true;
              m_condition.Signal();
            }
          }
          else
          {
            delete packet;
            break;
          }
        }
        while (m_socket.IsOpen() && GetTimeUs() - start < 1000 && m_inqueue.size() < 10000);
      }

      if (!m_outqueue.empty() && FD_ISSET(m_socket.GetSocket(), &writeset))
      {
        while (!m_outqueue.empty() && m_socket.IsOpen())
        {
          if (m_socket.SendMessage(*m_outqueue.front()))
          {
            delete m_outqueue.front();
            m_outqueue.pop_front();
          }
          else
          {
            break;
          }
        }
      }
    }
    else
    {
      for (deque<Packet*>::iterator it = m_outqueue.begin(); it != m_outqueue.end(); it++)
        delete *it;

      m_outqueue.clear();
    }

    if (FD_ISSET(m_pipe[0], &readset))
      ProcessPipeMessages();

    CLock lock(m_condition);
    m_process = true;
    m_condition.Signal();
    lock.Leave();
    ProcessInputQueue();
    lock.Enter();
    while (m_process)
      m_condition.Wait();
    lock.Leave();

    m_outputmanager.ProcessOutput();
    m_inputmanager.Process();
  }
}

void CBobTricks::Cleanup()
{
}

void CBobTricks::Process()
{
  SetThreadName("input");

  while(!m_stop)
  {
    CLock lock(m_condition);
    while (!m_process)
      m_condition.Wait(1000000);
    
    if (!m_process)
      continue;

    lock.Leave();
    ProcessInputQueue();
    lock.Enter();

    m_process = false;
    m_condition.Signal();
  }
}

void CBobTricks::ProcessInputQueue()
{
  CLock lock(m_condition);
  while(!m_inqueue.empty())
  {
    Packet* packet = m_inqueue.front();
    m_inqueue.pop_front();

    LogDebug("Received udp packet of size %i source:\"%s\" destination:\"%s\"", (int)packet->data.size(),
             packet->source.c_str(), packet->destination.c_str());

    lock.Leave();
    m_inputmanager.ParsePacket(packet);
    delete packet;
    lock.Enter();
  }
}

void CBobTricks::ProcessPipeMessages()
{
  uint8_t byte;
  int returnv = read(m_pipe[0], &byte, 1);

  if (returnv == 1 && (ThreadMsg)byte == MsgCheckUpdates)
    LogDebug("Received MsgCheckUpdates");
}

void CBobTricks::QueueTransmit(Packet* packet)
{
  CLock lock(m_mutex);
  m_outqueue.push_back(packet);
}

