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

#ifndef OUTPUTMANAGER_H
#define OUTPUTMANAGER_H

#include "artnetmanager.h"

class COutputManager : public CArtnetManager
{
  public:
    COutputManager(CBobTricks& bobtricks);
    ~COutputManager();

    void Process();
    int64_t MaxDelay();

  private:
    CJSONGenerator* SettingsToJSON(bool tofile);
};

#endif //OUTPUTMANAGER_H
