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

//#define JSONDEBUG

#include <fstream>
#include <assert.h>
#include "JSON.h"
#include "misc.h"

#ifdef JSONDEBUG
#include "log.h"
#define JSONLOG(fmt, ...) LogDebug(fmt, ##__VA_ARGS__)
#else
#define JSONLOG(fmd, ...)
#endif

using namespace std;

static int JNull(void* ctx)
{
  JSONLOG(" ");

  CJSONElement*& element = *(CJSONElement**)ctx;

  //an element's type is set to TYPENULL in the constructor
  if (element->IsArray())
    element->AsArray().push_back(new CJSONElement(element));
  else
    element = element->GetParent();

  return 1;
}

static int Boolean(void* ctx, int boolVal)
{
  JSONLOG("%i", boolVal);

  CJSONElement*& element = *(CJSONElement**)ctx;

  if (element->IsArray())
  {
    CJSONElement* child = new CJSONElement(element);
    child->SetType(TYPEBOOL);
    child->AsBool() = boolVal ? true : false;
    element->AsArray().push_back(child);
  }
  else
  {
    element->SetType(TYPEBOOL);
    element->AsBool() = boolVal ? true : false;
    element = element->GetParent();
  }

  return 1;
}

static int Number(void* ctx, const char * numberVal, YAJLSTRINGLEN numberLen)
{
  JSONLOG("%.*s", numberLen, numberVal);

  const char* ptr = numberVal;
  const char* end = numberVal + numberLen;
  
  //check if the string contains a decimal point, or an exponent, if it does read it in as a double
  //otherwise read it as int64_t
  bool isfloat = false;
  while (ptr != end)
  {
    if (*ptr == '.' || *ptr == 'e' || *ptr == 'E')
    {
      isfloat = true;
      break;
    }
    ptr++;
  }

  CJSONElement*& element = *(CJSONElement**)ctx;
  CJSONElement* parse;
  if (element->IsArray())
  {
    CJSONElement* child = new CJSONElement(element);
    element->AsArray().push_back(child);
    parse = child;
  }
  else
  {
    parse = element;
    element = element->GetParent();
  }

  bool parsed;
  if (isfloat)
  {
    parse->SetType(TYPEDOUBLE);
    parsed = StrToFloat(string(numberVal, numberLen), parse->AsDouble());
  }
  else
  {
    parse->SetType(TYPEINT64);
    parsed = StrToInt(string(numberVal, numberLen), parse->AsInt64());
  }

  if (!parsed && element)
    element->SetError(new string(string("Unable to parse ") + string(numberVal, numberLen) + " as number"));

  return parsed;
}

static int String(void* ctx, const unsigned char * stringVal, YAJLSTRINGLEN stringLen)
{
  JSONLOG("%.*s", stringLen, stringVal);

  CJSONElement*& element = *(CJSONElement**)ctx;

  if (element->IsArray())
  {
    CJSONElement* child = new CJSONElement(element);
    child->SetType(TYPESTRING);
    child->AsString().assign((const char*)stringVal, stringLen);
    element->AsArray().push_back(child);
  }
  else
  {
    element->SetType(TYPESTRING);
    element->AsString().assign((const char*)stringVal, stringLen);
    element = element->GetParent();
  }

  return 1;
}

static int StartMap(void* ctx)
{
  JSONLOG(" ");

  CJSONElement*& element = *(CJSONElement**)ctx;
  if (element->IsArray())
  {
    CJSONElement* child = new CJSONElement(element);
    child->SetType(TYPEMAP);
    element->AsArray().push_back(child);
    element = child;
  }
  else
  {
    element->SetType(TYPEMAP);
  }

  return 1;
}

static int MapKey(void* ctx, const unsigned char * key, YAJLSTRINGLEN stringLen)
{
  JSONLOG("%.*s", stringLen, key);

  CJSONElement*& element = *(CJSONElement**)ctx;
  
  std::string strkey((const char*)key, stringLen);

  //check if this key already exists
  if (element->AsMap().find(strkey) != element->AsMap().end())
  {
    element->SetError(new string(string("duplicate key ") + strkey));
    return 0;
  }

  //allocate a new child element on the current map
  //then update the pointer, the parse functions will use the pointer to access the child
  //and afterwards set the pointer back to the parent
  CJSONElement*& child = element->AsMap()[strkey];
  child = new CJSONElement(element);
  element = child;

  return 1;
}

static int EndMap(void* ctx)
{
  JSONLOG(" ");

  CJSONElement*& element = *(CJSONElement**)ctx;
  element = element->GetParent();

  return 1;
}

static int StartArray(void* ctx)
{
  //start of an array, all parse functions check if they need to allocate a new element in an array
  JSONLOG(" ");

  CJSONElement*& element = *(CJSONElement**)ctx;

  if (element->IsArray())
  {
    CJSONElement* child = new CJSONElement(element);
    child->SetType(TYPEARRAY);
    element->AsArray().push_back(child);
    element = child;
  }
  else
  {
    element->SetType(TYPEARRAY);
  }

  return 1;
}

static int EndArray(void* ctx)
{
  JSONLOG(" ");

  CJSONElement*& element = *(CJSONElement**)ctx;
  element = element->GetParent();

  return 1;
}

static yajl_callbacks parsecallbacks =
{
  JNull,
  Boolean,
  NULL,
  NULL,
  Number,
  String,
  StartMap,
  MapKey,
  EndMap,
  StartArray,
  EndArray,
};

static yajl_handle AllocHandle(CJSONElement** rootptr)
{
  yajl_handle handle;

#if YAJL_MAJOR == 2
  handle = yajl_alloc(&parsecallbacks, NULL, rootptr);
  yajl_config(handle, yajl_allow_comments, 1);
  yajl_config(handle, yajl_dont_validate_strings, 0);
#else
  yajl_parser_config yajlconfig;
  yajlconfig.allowComments = 1;
  yajlconfig.checkUTF8 = 1;
  handle = yajl_alloc(&parsecallbacks, &yajlconfig, NULL, rootptr);
#endif

  return handle;
}

static void PostProcess(yajl_handle handle, yajl_status status, string*& error,
                        const string& json, int linenr, CJSONElement* jsonelement)
{
  if (status == yajl_status_ok)
  {
#if YAJL_MAJOR == 2
    status = yajl_complete_parse(handle);
#else
    status = yajl_parse_complete(handle);
#endif
  }

  if (status != yajl_status_ok)
  {
    error = new string;
    if (linenr > 0)
      *error = string("line ") + ToString(linenr) + ": ";

    if (status == yajl_status_error)
    {
      unsigned char* yajlerror = yajl_get_error(handle, 1, (unsigned char*)json.c_str(), json.length());
      error->append((char*)yajlerror);
      yajl_free_error(handle, yajlerror);
    }
    else if (status == yajl_status_client_canceled)
    {
      if (jsonelement && jsonelement->GetError())
        error->append(*jsonelement->GetError());
      else
        error->append(yajl_status_to_string(status));
    }
    else
    {
      error->append(yajl_status_to_string(status));
    }
  }
  else
  {
    error = NULL;
  }

  yajl_free(handle);
}

//converts JSON into a tree structure of CJSONElement, using libyajl for parsing
CJSONElement* ParseJSON(const std::string& json, std::string*& error)
{
  //allocate a root element, the type will be set in one of the parser functions
  //for valid JSON, this will be TYPEMAP
  CJSONElement* root = new CJSONElement(NULL);
  CJSONElement* rootptr = root; //pointer for the yajl functions

  yajl_handle handle = AllocHandle(&rootptr);

  yajl_status status = yajl_parse(handle, (const unsigned char*)json.c_str(), json.length());

  PostProcess(handle, status, error, json, 0, rootptr);

  return root;
}

CJSONElement* ParseJSONFile(const std::string& filename, std::string*& error)
{
  //allocate a root element, the type will be set in one of the parser functions
  //for valid JSON, this will be TYPEMAP
  CJSONElement* root = new CJSONElement(NULL);
  CJSONElement* rootptr = root; //pointer for the yajl functions

  ifstream infile(filename.c_str());
  if (!infile.is_open())
  {
    error = new string(string("Unable to open \"") + filename + "\": " + GetErrno());
    return root;
  }

  yajl_handle handle = AllocHandle(&rootptr);

  string line;
  int linenr = 0;
  yajl_status status = yajl_status_ok;
  while (infile.good())
  {
    getline(infile, line);
    line += '\n';
    linenr++;

    status = yajl_parse(handle, (const unsigned char*)line.c_str(), line.length());

    if (status != yajl_status_ok
#if YAJL_MAJOR < 2
        && status != yajl_status_insufficient_data
#endif
      )
      break;
  }

  PostProcess(handle, status, error, line, linenr, rootptr);

  return root;
}

static void PrintElement(CJSONElement* element, CJSONGenerator& generator)
{
  if (element->IsMap())
  {
    generator.MapOpen();
    for (JSONMap::iterator it = element->AsMap().begin(); it != element->AsMap().end(); it++)
    {
      generator.AddString(it->first);
      PrintElement(it->second, generator);
    }
    generator.MapClose();
  }
  else if (element->IsArray())
  {
    generator.ArrayOpen();
    for (JSONArray::iterator it = element->AsArray().begin(); it != element->AsArray().end(); it++)
      PrintElement(*it, generator);
    generator.ArrayClose();
  }
  else if (element->IsBool())
  {
    generator.AddBool(element->AsBool());
  }
  else if (element->IsNull())
  {
    generator.AddNull();
  }
  else if (element->IsInt64())
  {
    generator.AddInt(element->AsInt64());
  }
  else if (element->IsDouble())
  {
    generator.AddDouble(element->AsDouble());
  }
  else if (element->IsString())
  {
    generator.AddString(element->AsString());
  }
}

//converts a CJSONElement* tree to json
std::string ToJSON(CJSONElement* root, bool beautify/* = false*/)
{
  CJSONGenerator generator(beautify);
  PrintElement(root, generator);

  return generator.ToString();
}

CJSONElement::CJSONElement(CJSONElement* parent)
{
  m_type = TYPENULL;
  memset(&m_data, 0, sizeof(m_data));
  m_parent = parent;
  m_error = NULL;
}

void CJSONElement::SetType(ELEMENTTYPE type)
{
  assert(m_type == TYPENULL);

  m_type = type;
  if (type == TYPESTRING)
    m_data.m_ptr = new string;
  else if (type == TYPEMAP)
    m_data.m_ptr = new JSONMap;
  else if (type == TYPEARRAY)
    m_data.m_ptr = new JSONArray;
}

CJSONElement::~CJSONElement()
{
  if (m_type == TYPESTRING)
  {
    delete (string*)m_data.m_ptr;
  }
  else if (m_type == TYPEMAP)
  {
    JSONMap* map = (JSONMap*)m_data.m_ptr;
    for (JSONMap::iterator it = map->begin(); it != map->end(); it++)
      delete it->second;

    delete (JSONMap*)m_data.m_ptr;
  }
  else if (m_type == TYPEARRAY)
  {
    JSONArray* array = (JSONArray*)m_data.m_ptr;
    for (JSONArray::iterator it = array->begin(); it != array->end(); it++)
      delete *it;

    delete (JSONArray*)m_data.m_ptr;
  }

  delete m_error;
}

int64_t CJSONElement::ToInt64()
{
  assert(IsNumber());

  if (m_type == TYPEINT64)
    return AsInt64();
  else if (m_type == TYPEDOUBLE)
    return Round64(AsDouble());
  else
    return 0;
}

double CJSONElement::ToDouble()
{
  assert(IsNumber());

  if (m_type == TYPEDOUBLE)
    return AsDouble();
  else if (m_type == TYPEINT64)
    return AsInt64();
  else
    return 0.0;
}

bool& CJSONElement::AsBool()
{
  assert(m_type == TYPEBOOL);
  return m_data.m_bvalue;
}

int64_t& CJSONElement::AsInt64()
{
  assert(m_type == TYPEINT64);
  return m_data.m_ivalue;
}

double& CJSONElement::AsDouble()
{
  assert(m_type == TYPEDOUBLE);
  return m_data.m_fvalue;
}

std::string& CJSONElement::AsString()
{
  assert(m_type == TYPESTRING);
  return *(string*)m_data.m_ptr;
}

JSONMap& CJSONElement::AsMap()
{
  assert(m_type == TYPEMAP);
  return *(JSONMap*)m_data.m_ptr;
}

JSONArray& CJSONElement::AsArray()
{
  assert(m_type == TYPEARRAY);
  return *(JSONArray*)m_data.m_ptr;
}

CJSONGenerator::CJSONGenerator(bool beautify/* = false*/)
{
#if YAJL_MAJOR == 2
  m_handle = yajl_gen_alloc(NULL);
  yajl_gen_config(m_handle, yajl_gen_beautify, beautify);
  yajl_gen_config(m_handle, yajl_gen_indent_string, "  ");
#else
  yajl_gen_config yajlconfig;
  yajlconfig.beautify = beautify;
  yajlconfig.indentString = "  ";
  m_handle = yajl_gen_alloc(&yajlconfig, NULL);
#endif
}

CJSONGenerator::~CJSONGenerator()
{
  yajl_gen_clear(m_handle);
  yajl_gen_free(m_handle);
}

void CJSONGenerator::Reset()
{
  yajl_gen_clear(m_handle);
}

std::string CJSONGenerator::ToString()
{
  const unsigned char* str;
  YAJLSTRINGLEN length;
  yajl_gen_get_buf(m_handle, &str, &length);
  return string((const char *)str, length);
}

void CJSONGenerator::ToString(std::string& jsonstr)
{
  const unsigned char* str;
  YAJLSTRINGLEN length;
  yajl_gen_get_buf(m_handle, &str, &length);
  jsonstr.assign((const char*)str, length);
}

void CJSONGenerator::AppendToString(std::string& jsonstr)
{
  const unsigned char* str;
  YAJLSTRINGLEN length;
  yajl_gen_get_buf(m_handle, &str, &length);
  jsonstr.append((const char*)str, length);
}

const uint8_t* CJSONGenerator::GetGenBuf(uint64_t& size)
{
  YAJLSTRINGLEN length;
  const unsigned char* str;
  yajl_gen_get_buf(m_handle, &str, &length);
  size = length;
  return str;
}

const uint8_t* CJSONGenerator::GetGenBuf()
{
  YAJLSTRINGLEN length;
  const unsigned char* str;
  yajl_gen_get_buf(m_handle, &str, &length);
  return str;
}

uint64_t CJSONGenerator::GetGenBufSize()
{
  YAJLSTRINGLEN length;
  const unsigned char* str;
  yajl_gen_get_buf(m_handle, &str, &length);
  return length;
}

void CJSONGenerator::AddInt(int64_t in)
{
  string number = ::ToString(in);
  yajl_gen_number(m_handle, number.c_str(), number.length());
}

void CJSONGenerator::AddDouble(double in)
{
  string number = ::ToString(in);
  yajl_gen_number(m_handle, number.c_str(), number.length());
}

