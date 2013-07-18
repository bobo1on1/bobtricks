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

#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <time.h>
#include <sstream>
#include <vector>
#include <sys/time.h>

#include "log.h"
#include "mutex.h"
#include "lock.h"
#include "misc.h"

using namespace std;

bool g_logtostderr = true;
bool g_printlogtofile = true;
bool g_printdebuglevel = false;
string g_logfilename;

//as pointers, so we can allocate these after the locked memory is set up
CMutex* g_logmutex;
static ofstream* g_logfile;

static int             g_logbuffsize; //size of the buffer
static char*           g_logbuff;     //buffer for vsnprintf
static vector<string>* g_logstrings;  //we save any log lines here while the log isn't open

//returns hour:minutes:seconds:microseconds
string GetStrTime()
{
  struct timeval tv;
  struct tm      time;
  time_t         now;

  //get current time
  gettimeofday(&tv, NULL);
  now = tv.tv_sec; //seconds since EPOCH
  localtime_r(&now, &time); //convert to hours, minutes, seconds

  char buff[16];
  snprintf(buff, sizeof(buff), "%02i:%02i:%02i.%06i", time.tm_hour, time.tm_min, time.tm_sec, (int)tv.tv_usec);

  return buff;
}

bool InitLog(string filename, ofstream& logfile)
{
  string homepath;
  if (!GetHomePath(homepath))
  {
    PrintError("Unable to get home directory path");
    return false;
  }
  
  string directory = homepath + ".bobtricks/";
  string fullpath = directory + filename;

  //try to make the directory the log goes in
  if (mkdir(directory.c_str(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH) == -1)
  {
    //if it already exists we're ok
    if (errno != EEXIST)
    {
      PrintError("unable to make directory " + directory + ":\n" + GetErrno());
      return false;
    }
  }

  //we keep around 5 old logfiles
  for (int i = 4; i > 0; i--)
    rename(string(fullpath + ".old." + ToString(i)).c_str(), string(fullpath + ".old." + ToString(i + 1)).c_str());

  rename(fullpath.c_str(), string(fullpath + ".old.1").c_str());

  //open the logfile in append mode
  logfile.open(fullpath.c_str());
  if (logfile.fail())
  {
    PrintError("unable to open " + fullpath + ":\n" + GetErrno());
    return false;
  }

  Log("successfully set up logfile %s", fullpath.c_str());

  g_logfilename = fullpath;

  return true;
}

//we only want the function name and the namespace, so we search for '(' and get the string before that
string PruneFunction(string function)
{
  size_t parenpos = function.find('(');
  size_t spacepos = function.rfind(' ', parenpos);

  if (parenpos == string::npos)
    return function;
  else if (spacepos == string::npos) //when called from a constructor, there's no return value, thus no space
    return function.substr(0, parenpos);
  else
    return function.substr(spacepos + 1, parenpos - spacepos - 1);
}

void SetLogFile(std::string filename)
{
  if (!g_logmutex)
    g_logmutex = new CMutex;

  CLock lock(*g_logmutex);

  if (!g_logfile)
    g_logfile = new ofstream;

  //deallocate, so it will be allocated in PrintLog gain, but this time in locked memory
  free(g_logbuff);
  g_logbuff = NULL;
  g_logbuffsize = 0;

  if (!InitLog(filename, *g_logfile))
    g_printlogtofile = false;
}

void PrintLog (const char* fmt, const char* function, LogLevel loglevel, ...)
{
  if (loglevel == LogLevelDebug && !g_printdebuglevel)
    return;

  if (g_logmutex)
    g_logmutex->Lock();

  string  logstr;
  string  funcstr;
  va_list args;
  int     nrspaces;

  va_start(args, loglevel);

  //print to the logbuffer and check if our buffer is large enough
  int neededbuffsize = vsnprintf(g_logbuff, g_logbuffsize, fmt, args);
  if (neededbuffsize + 1 > g_logbuffsize)
  {
    g_logbuffsize = neededbuffsize + 1;
    g_logbuff = (char*)(realloc(g_logbuff, g_logbuffsize)); //resize the buffer to the needed size

    va_end(args); //need to reinit or we will crash
    va_start(args, loglevel);
    vsnprintf(g_logbuff, g_logbuffsize, fmt, args);         //write to the buffer again
  }
  
  va_end(args);

  if (loglevel == LogLevelError)
    logstr += "ERROR: ";

  if (g_logbuff)
    logstr += g_logbuff;

  funcstr = "(" + PruneFunction(function) + ")";
  nrspaces = 34 - funcstr.length();
  if (nrspaces > 0)
    funcstr.insert(funcstr.length(), nrspaces, ' ');
  
  if (g_logfile && g_logfile->is_open() && g_printlogtofile)
  {
    //print any saved log lines
    if (g_logstrings)
    {
      for (vector<string>::iterator it = g_logstrings->begin(); it != g_logstrings->end(); it++)
        *g_logfile << *it << flush;

      delete g_logstrings;
      g_logstrings = NULL;
    }
    //write the string to the logfile
    *g_logfile << GetStrTime() << " " << funcstr << " " << logstr << '\n' << flush;
  }
  else if (g_printlogtofile)
  {
    //save the log line if the log isn't open yet
    if (!g_logstrings)
      g_logstrings = new vector<string>;
    g_logstrings->push_back(GetStrTime() + " " + funcstr + " " + logstr + '\n');
  }

  //print to stdout when requested
  if (g_logtostderr)
    cerr << funcstr << logstr << '\n' << flush;

  if (g_logmutex)
    g_logmutex->Unlock();
}
