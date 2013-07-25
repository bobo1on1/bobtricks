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

#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include <lua5.1/lua.hpp>

extern "C"
{
  #include <lua5.1/lua-posix.h>
}

class CParameter
{
  public:
    CParameter(const std::string& name, double value)
    {
      m_name = name;
      m_value = value;
    }

  private:

    std::string m_name;
    double      m_value;
};

class CScript
{
  public:
    CScript(const std::string& name, const std::string& filename, std::vector<CParameter>& parameters);
    ~CScript();

    bool                    Load();
    bool                    Loaded() { return m_loaded; }
    bool                    LoadFailed() { return m_loadfailed; }

  private:
    bool                    LoadInternal();

    std::string             m_name;
    std::string             m_filename;
    std::vector<CParameter> m_parameters;
    lua_State*              m_L;
    bool                    m_loaded;
    bool                    m_loadfailed;
};

#endif //SCRIPT_H
