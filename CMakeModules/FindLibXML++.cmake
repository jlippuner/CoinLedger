# from https://github.com/Azure/azure-storage-cpp
#
# -----------------------------------------------------------------------------------------
# <copyright file="basic_types.cpp" company="Microsoft">
#    Copyright 2013 Microsoft Corporation
#
#    Licensed under the Apache License, Version 2.0 (the "License");
#    you may not use this file except in compliance with the License.
#    You may obtain a copy of the License at
#      http://www.apache.org/licenses/LICENSE-2.0
#
#    Unless required by applicable law or agreed to in writing, software
#    distributed under the License is distributed on an "AS IS" BASIS,
#    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#    See the License for the specific language governing permissions and
#    limitations under the License.
# </copyright>
# -----------------------------------------------------------------------------------------
#
# find libxml++
#
# exports:
#
#   LibXML++_FOUND
#   LibXML++_INCLUDE_DIRS
#   LibXML++_LIBRARIES
#

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LibXML++_PKGCONF QUIET libxml++-2.6)

# Include dir
find_path(LibXML++_INCLUDE_DIR
  NAMES libxml++/libxml++.h
  PATHS
    ${LibXML++_PKGCONF_INCLUDE_DIRS}
    ${LibXML++_ROOT_DIR}
    /usr
  PATH_SUFFIXES
    include/libxml++-2.6
)

# Config Include dir
find_path(LibXML++Config_INCLUDE_DIR
  NAMES libxml++config.h
  PATHS
    ${LibXML++_PKGCONF_INCLUDE_DIRS}
    ${LibXML++_ROOT_DIR}
    /usr
  PATH_SUFFIXES
    lib/libxml++-2.6/include
)

# Finally the library itself
find_library(LibXML++_LIBRARY
  NAMES xml++ xml++-2.6
  PATHS
    ${LibXML++_PKGCONF_LIBRARY_DIRS}
    ${LibXML++_ROOT_DIR}
    /usr
  PATH_SUFFIXES
    lib
)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXML++ DEFAULT_MSG LibXML++_LIBRARY LibXML++_INCLUDE_DIR)

if(LibXML++_INCLUDE_DIR AND LibXML++_LIBRARY)
  set(LibXML++_LIBRARIES ${LibXML++_LIBRARY} ${LibXML++_PKGCONF_LIBRARIES})
  set(LibXML++_INCLUDE_DIRS ${LibXML++_INCLUDE_DIR} ${LibXML++Config_INCLUDE_DIR} ${LibXML++_PKGCONF_INCLUDE_DIRS})
  set(LibXML++_FOUND yes)
else()
  set(LibXML++_LIBRARIES)
  set(LibXML++_INCLUDE_DIRS)
  set(LibXML++_FOUND no)
endif()
