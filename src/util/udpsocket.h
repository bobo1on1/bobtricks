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

#ifndef UDPSOCKET_H
#define UDPSOCKET_H

#include "inclstdint.h"
#include <vector>
#include <string>

struct Packet
{
  std::string          source;
  std::string          destination;
  uint16_t             port;
  std::vector<uint8_t> data;
};

class CUdpSocket
{
  public:
    CUdpSocket();
    ~CUdpSocket();

    bool IsOpen() { return m_sock != -1; }
    bool Open(uint16_t port);
    void Close();
    int  GetSocket() { return m_sock; }
    bool SetNonBlock(bool nonblock);

    bool GetMessage(std::vector<uint8_t>& data, std::string& source, std::string& destination, uint16_t& port);
    bool GetMessage(Packet& packet);
    bool SendMessage(std::vector<uint8_t>& data, std::string& destination, uint16_t port);
    bool SendMessage(Packet& packet);

  private:

    int m_sock;
};

#endif //UDPSOCKET_H
