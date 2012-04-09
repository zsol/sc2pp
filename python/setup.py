#!/usr/bin/env python

"""
setup.py file for sc2pp
"""

from distutils.core import setup, Extension

include_dirs = '${INCLUDE_DIRECTORIES}'

sc2pp_module = Extension('_sc2pp',
						   sources=['${CMAKE_CURRENT_BINARY_DIR}/sc2pp.i'],
						   include_dirs=include_dirs.split(';'),
						   extra_compile_args='${CMAKE_CXX_FLAGS}'.split(),
						   library_dirs=['${SC2pp_SHARED_OBJECT_DIRECTORY}'],
						   libraries=['sc2pp'],
						   swig_opts=['-builtin', '-c++', '-outcurrentdir', '-modern', '-I${CMAKE_CURRENT_SOURCE_DIR}'],
						   )

setup (name = 'sc2pp',
	   version = '${SC2pp_VERSION}',
	   author      = "Zsolt Dollenstein",
	   description = """sc2pp""",
	   ext_modules = [sc2pp_module],
	   py_modules = ['sc2pp'],
	   )
