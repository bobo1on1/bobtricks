#! /usr/bin/env python
# encoding: utf-8

# the following two variables are used by the target "waf dist"
VERSION='0.0.1'
APPNAME='bobtricks'

# these variables are mandatory ('/' are converted automatically)
top = '.'
out = 'build'

def options(opt):
  opt.load('compiler_cxx')

def configure(conf):
  conf.load('compiler_cxx')

  conf.check(header_name='yajl/yajl_gen.h')
  conf.check(header_name='yajl/yajl_parse.h')
  conf.check(header_name='yajl/yajl_version.h', mandatory=False)

  conf.check(lib='pthread', uselib_store='pthread', mandatory=False)
  conf.check(lib='yajl', uselib_store='yajl')

  conf.check(lib='lua5.1', uselib_store='lua')

  conf.env.append_value('LINKFLAGS', '-llua5.1')
  conf.check(lib='lua5.1-posix', uselib_store='lua-posix')

  conf.check(function_name='pthread_setname_np', header_name='pthread.h', lib='pthread', mandatory=False)

  conf.write_config_header('config.h')

def build(bld):
  bld.program(source='src/main.cpp\
                      src/bobtricks.cpp\
                      src/jsonsettings.cpp\
                      src/universe.cpp\
                      src/outputuniverse.cpp\
                      src/inputuniverse.cpp\
                      src/outputmanager.cpp\
                      src/inputmanager.cpp\
                      src/user.cpp\
                      src/scriptmanager.cpp\
                      src/script.cpp\
                      src/util/udpsocket.cpp\
                      src/util/log.cpp\
                      src/util/mutex.cpp\
                      src/util/condition.cpp\
                      src/util/timeutils.cpp\
                      src/util/JSON.cpp\
                      src/util/thread.cpp\
                      src/util/misc.cpp',
              use=['pthread', 'yajl', 'lua', 'lua-posix'],
              includes='./src',
              cxxflags='-Wall -g -O3 -march=native -DUTILNAMESPACE=BobTricksUtil',
              target='bobtricks')
