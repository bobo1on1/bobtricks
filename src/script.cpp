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

#include "script.h"
#include "util/misc.h"
#include "util/log.h"
#include <iostream>
#include <fstream>

using namespace std;

CScript::CScript(const std::string& name, const std::string& filename, std::vector<CParameter>& parameters)
{
  m_name = name;
  m_filename = filename;
  m_parameters = parameters;
  m_L = NULL;
  m_loaded = false;
  m_loadfailed = false;
}

CScript::~CScript()
{
}

bool CScript::Load()
{
  if (LoadInternal())
  {
    m_loaded = true;
    m_loadfailed = false;
    return true;
  }
  else
  {
    m_loaded = false;
    m_loadfailed = true;
    return false;
  }
}

bool CScript::LoadInternal()
{
  if (m_loaded)
    return true;

  Log("loading file \"%s\"", m_filename.c_str());

  ifstream scriptfile((string("scripts/") + m_filename).c_str());
  if (scriptfile.fail())
  {
    LogError("%s: %s", m_filename.c_str(), GetErrno().c_str());
    return false;
  }

  return true;
}

