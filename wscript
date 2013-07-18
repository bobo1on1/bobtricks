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
  conf.write_config_header('config.h')

def build(bld):
  bld.program(source='src/main.cpp',
              use=[],
              includes='./src',
              cxxflags='-Wall -g -DUTILNAMESPACE=BobTricksUtil',
              target='bobtricks')
