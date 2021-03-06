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

#include "universe.h"
#include "util/timeutils.h"
#include "util/misc.h"
#include <string.h>
#include <algorithm>

using namespace std;

CUniverse::CUniverse(const std::string& name, uint16_t portaddress, const std::string& ipaddress, bool enabled)
{
  memset(m_channels, 0, sizeof(m_channels));
  m_name = name;
  m_portaddress = portaddress;
  m_ipaddress = ipaddress;
  m_enabled = enabled;
}

CUniverse::~CUniverse()
{
}

