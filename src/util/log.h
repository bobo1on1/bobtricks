/*
 * bobdsp
 * Copyright (C) Bob  2009 
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

#ifndef LOG
#define LOG

#include <string>
#include "mutex.h"

enum LogLevel
{
  LogLevelBasic,
  LogLevelError,
  LogLevelDebug,
};

//this has to be a macro, because we want __PRETTY_FUNCTION__
#define Log(fmt, ...) PrintLog(fmt, __PRETTY_FUNCTION__, LogLevelBasic, ##__VA_ARGS__)
#define LogError(fmt, ...) PrintLog(fmt, __PRETTY_FUNCTION__, LogLevelError, ##__VA_ARGS__)
#define LogDebug(fmt, ...) g_printdebuglevel ? PrintLog(fmt, __PRETTY_FUNCTION__, LogLevelDebug, ##__VA_ARGS__) : (void)0

void PrintLog (const char* fmt, const char* function, LogLevel loglevel, ...) __attribute__ ((format (printf, 1, 4)));
void SetLogFile(std::string logfile);

extern bool g_logtostderr;
extern bool g_printlogtofile;
extern bool g_printdebuglevel;
extern std::string g_logfilename;
extern CMutex*     g_logmutex;

#endif //LOG
