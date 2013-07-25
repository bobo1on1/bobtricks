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

#ifndef UNIVERSE_H
#define UNIVERSE_H

#include "util/inclstdint.h"
#include <string>
#include <list>

enum Opcode
{
  OpPoll = 0x2000, //This is an ArtPoll packet, no other data is contained in this UDP packet.
  OpPollReply = 0x2100, //This is an ArtPollReply Packet. It contains device status information.
  OpDiagData = 0x2300, //Diagnostics and data logging packet.
  OpCommand = 0x2400, //Used to send text based parameter commands.
  OpOutput = 0x5000, //This is an ArtDmx data packet. It contains zero start code DMX512 information for a single Universe.
  OpNzs = 0x5100, //This is an ArtNzs data packet. It contains non-zero start code (except RDM) DMX512 information for a single Universe.
  OpAddress = 0x6000, //This is an ArtAddress packet. It contains remote programming information for a Node.
  OpInput = 0x7000, //This is an ArtInput packet. It contains enable – disable data for DMX inputs.
  OpTodRequest = 0x8000, //This is an ArtTodRequest packet. It is used to request a Table of Devices (ToD) for RDM discovery.
  OpTodData = 0x8100, //This is an ArtTodData packet. It is used to send a Table of Devices (ToD) for RDM discovery.
  OpTodControl = 0x8200, //This is an ArtTodControl packet. It is used to send RDM discovery control messages.
  OpRdm = 0x8300, //This is an ArtRdm packet. It is used to send all non discovery RDM messages.
  OpRdmSub = 0x8400, //This is an ArtRdmSub packet. It is used to send compressed, RDM Sub-Device data.
  OpVideoSetup = 0xa010, //This is an ArtVideoSetup packet. It contains video screen setup information for nodes that implement the extended video features.
  OpVideoPalette = 0xa020, //This is an ArtVideoPalette packet. It contains colour palette setup information for nodes that implement the extended video features.
  OpVideoData = 0xa040, //This is an ArtVideoData packet. It contains display data for nodes that implement the extended video features.
  OpMacMaster = 0xf000, //This is an ArtMacMaster packet. It is used to program the Node’s MAC address, Oem device type and ESTA manufacturer code. This is for factory initialisation of a Node. It is not to be used by applications.
  OpMacSlave = 0xf100, //This is an ArtMacSlave packet. It is returned by the node to acknowledge receipt of an ArtMacMaster packet. 
  OpFirmwareMaster = 0xf200, //This is an ArtFirmwareMaster packet. It is used to upload new firmware or firmware extensions to the Node.
  OpFirmwareReply = 0xf300, //This is an ArtFirmwareReply packet. It is returned by the node to acknowledge receipt of an ArtFirmwareMaster packet or ArtFileTnMaster packet.
  OpFileTnMaster = 0xf400, //Uploads user file to node.
  OpFileFnMaster = 0xf500, //Downloads user file from node.
  OpFileFnReply = 0xf600, //Node acknowledge for downloads.
  OpIpProg = 0xf800, //This is an ArtIpProg packet. It is used to reprogramme the IP, Mask and Port address of the Node
  OpIpProgReply = 0xf900, //This is an ArtIpProgReply packet. It is returned by the node to acknowledge receipt of an ArtIpProg packet.
  OpMedia = 0x9000, //This is an ArtMedia packet. It is Unicast by a Media Server and acted upon by a Controller.
  OpMediaPatch = 0x9100, //This is an ArtMediaPatch packet. It is Unicast by a Controller and acted upon by a Media Server.
  OpMediaControl = 0x9200, //This is an ArtMediaControl packet. It is Unicast by a Controller and acted upon by a Media Server.
  OpMediaContrlReply  = 0x9300, //This is an ArtMediaControlReply packet. It is Unicast by a Media Server and acted upon by a Controller.
  OpTimeCode = 0x9700, //This is an ArtTimeCode packet. It is used to transport time code over the network.
  OpTimeSync = 0x9800, //Used to synchronise real time date and clock
  OpTrigger = 0x9900, //Used to send trigger macros
  OpDirectory = 0x9a00, //Requests a node's file list
  OpDirectoryReply = 0x9b00, //Replies to OpDirectory with file list
};

struct SArtDmx
{
  uint8_t  ID[8];
  uint16_t OpCode;
  uint8_t  ProtVerHi;
  uint8_t  ProtVerLow;
  uint8_t  Sequence;
  uint8_t  Physical;
  uint8_t  SubUni;
  uint8_t  Net;
  uint8_t  LengthHi;
  uint8_t  Length;
  uint8_t  Data[];
} __attribute__((packed));

class CUniverse
{
  public:
    CUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled);
    ~CUniverse();

    const std::string& Name()        { return m_name;        }
    const std::string& IpAddress()   { return m_ipaddress;   }
    uint16_t           PortAddress() { return m_portaddress; }


  protected:
    uint8_t            m_channels[512];
    std::string        m_name;
    uint16_t           m_portaddress;
    std::string        m_ipaddress;
    bool               m_enabled;
};

#endif //UNIVERSE_H
