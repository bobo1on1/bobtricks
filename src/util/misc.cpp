/*
 * boblight
 * Copyright (C) Bob  2009 
 * 
 * boblight is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * boblight is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <locale.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include "misc.h"

using namespace std;

namespace UTILNAMESPACE
{
  void PrintError(const std::string& error)
  {
    std::cerr << "ERROR: " << error << "\n";
  }

  //get the first word (separated by whitespace) from string data and place that in word
  //then remove that word from string data
  bool GetWord(string& data, string& word)
  {
    stringstream datastream(data);
    string end;

    datastream >> word;
    if (datastream.fail())
    {
      data.clear();
      return false;
    }

    size_t pos = data.find(word) + word.length();

    if (pos >= data.length())
    {
      data.clear();
      return true;
    }

    data = data.substr(pos);
    
    datastream.clear();
    datastream.str(data);

    datastream >> end;
    if (datastream.fail())
      data.clear();

    return true;
  }

  //convert . or , to the current locale for correct conversion of ascii float
  void ConvertFloatLocale(std::string& strfloat)
  {
    static struct lconv* locale = localeconv();
    
    size_t pos = strfloat.find_first_of(",.");

    while (pos != string::npos)
    {
      strfloat.replace(pos, 1, 1, *locale->decimal_point);
      pos++;

      if (pos >= strfloat.size())
        break;

      pos = strfloat.find_first_of(",.", pos);
    }
  }

  bool GetHomePath(std::string& homepath)
  {
    const char* homeptr = getenv("HOME");
    if (homeptr && strlen(homeptr) > 0)
    {
      homepath = PutSlashAtEnd(homeptr);
      return true;
    }
    else
    {
      return false;
    }
  }

  std::string PutSlashAtEnd(const std::string& path)
  {
    if (path.empty())
      return "/";
    else if (path[path.length() - 1] != '/')
      return path + '/';
    else
      return path;
  }

  std::string RemoveSlashAtEnd(const std::string& path)
  {
    if (path.length() > 0 && path[path.length() - 1] == '/')
      return path.substr(0, path.length() - 1);
    else
      return path;
  }

  std::string PutSlashAtStart(const std::string& path)
  {
    if (path.empty())
      return "/";
    else if (path[0] != '/')
      return string("/") + path;
    else
      return path;
  }

  std::string FileNameExtension(const std::string& path)
  {
    size_t pos = path.rfind('.');
    if (pos == string::npos || pos >= path.length() - 1)
      return "";
    else
      return path.substr(pos + 1);
  }

  std::string ToLower(const std::string& in)
  {
    string out;
    for (string::const_iterator it = in.begin(); it != in.end(); it++)
      out += (char)tolower(*it);

    return out;
  }

  std::string ToLower(const std::string& in, std::string& out)
  {
    out.clear();
    out.reserve(in.size());

    for (string::const_iterator it = in.begin(); it != in.end(); it++)
      out += (char)tolower(*it);

    return out;
  }

  std::string Capitalize(const std::string& in)
  {
    string out = in;
    if (!out.empty())
      out[0] = toupper(out[0]);

    return out;
  }
  
  bool StrToBool(const std::string& data, bool& value)
  {
    std::string data2 = data;
    std::string word;
    if (!GetWord(data2, word))
      return false;
    
    for (std::string::iterator it = word.begin(); it != word.end(); it++)
      *it = tolower(*it);

    if (word == "1" || word == "true" || word == "on" || word == "yes")
    {
      value = true;
      return true;
    }
    else if (word == "0" || word == "false" || word == "off" || word == "no")
    {
      value = false;
      return true;
    }
    else
    {
      int ivalue;
      if (StrToInt(word, ivalue))
      {
        value = ivalue != 0;
        return true;
      }
    }

    return false;
  }

  //returns < 0 if a directory outside the root is accessed through ..
  int DirLevel(const std::string& url)
  {
    size_t start = 0;
    size_t pos;

    int level = 0;
    while (1)
    {
      pos = url.find('/', start);

      string filename;
      if (pos == string::npos)
        filename = url.substr(start);
      else if (pos > start)
        filename = url.substr(start, pos - start);

      if (filename == "..")
        level--;
      else if (!filename.empty() && filename != ".")
        level++;

      if (level < 0 || pos == string::npos || pos == url.length() - 1)
        return level;

      start = pos + 1;
    }
  }
}
