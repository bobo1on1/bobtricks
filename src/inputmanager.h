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

#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include "artnetmanager.h"
#include "util/udpsocket.h"

class CInputManager : public CArtnetManager
{
  public:
    CInputManager(CBobTricks& bobtricks);
    ~CInputManager();

    void ParsePacket(Packet* packet);

  private:
    CJSONGenerator* SettingsToJSON(bool tofile);
};

#endif //INPUTMANAGER_H
