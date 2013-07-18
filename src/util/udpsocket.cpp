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

#include "udpsocket.h"
#include "util/log.h"
#include "util/misc.h"

#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>

CUdpSocket::CUdpSocket()
{
  m_sock = -1;
}

CUdpSocket::~CUdpSocket()
{
  Close();
}

bool CUdpSocket::Open(uint16_t port)
{
  Close();

  m_sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (m_sock == -1)
  {
    LogError("socket(): %i:%s", errno, GetErrno().c_str());
    return false;
  }

  LogDebug("created udp socket");

  sockaddr_in bindaddr = {};
  bindaddr.sin_family=AF_INET;
  bindaddr.sin_addr.s_addr=INADDR_ANY;
  bindaddr.sin_port=htons(port);

  if (bind(m_sock, (sockaddr*)&bindaddr, sizeof(bindaddr)) == -1)
  {
    LogError("bind(): %i:%s", errno, GetErrno().c_str());
    Close();
    return false;
  }

  LogDebug("socket bound to port %i", port);

  int val = 1;
  if (setsockopt(m_sock, IPPROTO_IP, IP_PKTINFO, &val, sizeof(val)) == -1)
  {
    LogError("setsocksopt(): %i:%s", errno, GetErrno().c_str());
    Close();
    return false;
  }

  LogDebug("IP_PKTINFO set");

  SetNonBlock(true);

  return true;
}

bool CUdpSocket::SetNonBlock(bool nonblock)
{
  if (m_sock == -1)
  {
    LogError("can't set a closed socket to non blocking");
    return false;
  }

  //non-blocking socket, because we work with select
  int flags = fcntl(m_sock, F_GETFL);
  if (flags == -1)
  {
    LogError("F_GETFL %i:%s", errno, GetErrno().c_str());
    return false;
  }

  if (nonblock)
    flags |= O_NONBLOCK;
  else
    flags &= ~O_NONBLOCK;
  
  if (fcntl(m_sock, F_SETFL, flags) == -1)
  {
    LogError("F_SETFL %i:%s", errno, GetErrno().c_str());
    return false;
  }

  if (nonblock)
    LogDebug("socket set to non blocking mode");
  else
    LogDebug("socket set to blocking mode");

  return true;
}


void CUdpSocket::Close()
{
  if (m_sock != -1)
  {
    shutdown(m_sock, SHUT_RDWR);
    close(m_sock);
    m_sock = -1;
  }
}

bool CUdpSocket::GetMessage(Packet& packet)
{
  return GetMessage(packet.data, packet.source, packet.destination, packet.port);
}

bool CUdpSocket::GetMessage(std::vector<uint8_t>& data, std::string& source, std::string& destination, uint16_t& port)
{
  if (m_sock == -1)
  {
    LogError("can't read from a closed socket");
    return false;
  }

  struct
  {
    struct cmsghdr    cmh;
    struct in_pktinfo pktinfo;
  } cmsg;

  msghdr header = {};
  header.msg_control = &cmsg;
  header.msg_controllen = sizeof(cmsg);

  if (recvmsg(m_sock, &header, MSG_PEEK) == -1)
  {
    if (errno != EAGAIN)
    {
      LogError("recvmsg() %i:%s", errno, GetErrno().c_str());
      if (errno != EINTR)
        Close();
    }
    return false;
  }

  destination = inet_ntoa(cmsg.pktinfo.ipi_addr);

  data.resize(1500);

  ssize_t returnv;
  sockaddr_in sourceaddr;
  socklen_t len = sizeof(sourceaddr);
  returnv = recvfrom(m_sock, &data[0], data.size(), 0, (sockaddr*)&sourceaddr, &len);
  if (returnv == -1)
  {
    if (errno != EAGAIN)
    {
      LogError("recvfrom() %i:%s", errno, GetErrno().c_str());
      if (errno != EINTR)
        Close();
    }
    return false;
  }

  data.resize(returnv);

  source = inet_ntoa(sourceaddr.sin_addr);
  port = ntohs(sourceaddr.sin_port);

  return true;
}

bool CUdpSocket::SendMessage(Packet& packet)
{
  return SendMessage(packet.data, packet.destination, packet.port);
}

bool CUdpSocket::SendMessage(std::vector<uint8_t>& data, std::string& destination, uint16_t port)
{
  if (m_sock == -1)
  {
    LogError("can't write to a closed socket");
    return false;
  }

  sockaddr_in destaddr = {};
  destaddr.sin_family = AF_INET;
  destaddr.sin_port = htons(port);

  if (inet_aton(destination.c_str(), &destaddr.sin_addr) == 0)
  {
    LogError("error parsing address \"%s\"", destination.c_str());
    return true; //this can't be solved by a retry
  }

  ssize_t returnv;
  returnv = sendto(m_sock, &data[0], data.size(), 0, (const sockaddr*)&destaddr, sizeof(destaddr));

  if (returnv == -1)
  {
    if (errno != EAGAIN)
    {
      LogError("sendto() %i:%s", errno, GetErrno().c_str());
      if (errno != EINTR)
        Close();
    }
    return false;
  }

  return true;
}
