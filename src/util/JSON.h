/*
 * bobdsp
 * Copyright (C) Bob 2012
 *
 * bobdsp is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bobdsp is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef JSON_H
#define JSON_H

#include "config.h"
#include "util/inclstdint.h"
#include <string>
#include <cstring>
#include <map>
#include <vector>

#include <yajl/yajl_parse.h>
#include <yajl/yajl_gen.h>
#ifdef HAVE_YAJL_YAJL_VERSION_H
#include <yajl/yajl_version.h>
#endif

#if YAJL_MAJOR == 2
  #define YAJLSTRINGLEN size_t
#else
  #define YAJLSTRINGLEN unsigned int
#endif

class CJSONGenerator
{
  public:
    CJSONGenerator(bool beautify = false);
    ~CJSONGenerator();

    void           Reset();
    std::string    ToString();
    void           ToString(std::string& jsonstr);
    void           AppendToString(std::string& jsonstr);
    const uint8_t* GetGenBuf(uint64_t& size);
    const uint8_t* GetGenBuf();
    uint64_t       GetGenBufSize();

    void MapOpen()    { yajl_gen_map_open(m_handle);    }
    void MapClose()   { yajl_gen_map_close(m_handle);   }
    void ArrayOpen()  { yajl_gen_array_open(m_handle);  }
    void ArrayClose() { yajl_gen_array_close(m_handle); }
    void AddNull()    { yajl_gen_null(m_handle);        }
    void AddString(const std::string& in)
      { yajl_gen_string(m_handle, (const unsigned char*)in.c_str(), in.length()); }
    void AddString(const char* in)
      { yajl_gen_string(m_handle, (const unsigned char*)in, strlen(in)); }
    void AddInt(int64_t in);
    void AddDouble(double in);
    void AddBool(bool in)
      { yajl_gen_bool(m_handle, in); }

  private:
    yajl_gen m_handle;
};

enum ELEMENTTYPE
{
  TYPENULL,
  TYPEBOOL,
  TYPEINT64,
  TYPEDOUBLE,
  TYPESTRING,
  TYPEMAP,
  TYPEARRAY,
};

class CJSONElement;
typedef std::map<std::string, CJSONElement*> JSONMap;
typedef std::vector<CJSONElement*>           JSONArray;

CJSONElement* ParseJSON(const std::string& json, std::string*& error);
CJSONElement* ParseJSONFile(const std::string& filename, std::string*& error);
std::string ToJSON(CJSONElement* root, bool beautify = false);

class CJSONElement
{
  public:
    CJSONElement(CJSONElement* parent);
    ~CJSONElement();

    void          SetType(ELEMENTTYPE type);

    CJSONElement* GetParent() { return m_parent; }

    void          SetError(std::string* error) { m_error = error; }
    std::string*  GetError() { return m_error; }

    bool          IsNull()   { return m_type == TYPENULL; }
    bool          IsBool()   { return m_type == TYPEBOOL; }
    bool          IsInt64()  { return m_type == TYPEINT64; }
    bool          IsDouble() { return m_type == TYPEDOUBLE; }
    bool          IsNumber() { return IsInt64() || IsDouble(); }
    bool          IsString() { return m_type == TYPESTRING; }
    bool          IsMap()    { return m_type == TYPEMAP; }
    bool          IsArray()  { return m_type == TYPEARRAY; }
    int64_t       ToInt64();
    double        ToDouble();

    bool&         AsBool();
    int64_t&      AsInt64();
    double&       AsDouble();
    std::string&  AsString();
    JSONMap&      AsMap();
    JSONArray&    AsArray();

  private:
    union
    {
      void*       m_ptr;
      double      m_fvalue;
      int64_t     m_ivalue;
      bool        m_bvalue;
    }
    m_data;

    ELEMENTTYPE   m_type;
    CJSONElement* m_parent;
    std::string*  m_error;
};

#endif //JSON_H
