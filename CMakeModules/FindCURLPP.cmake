# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# Find the native CURLPP headers and libraries.
#
# CURLPP_INCLUDE_DIRS	- where to find cURLpp.hpp, etc.
# CURLPP_LIBRARIES	- List of libraries when using curlpp.
# CURLPP_FOUND	- True if curlpp found.

find_package(CURL REQUIRED)

# Look for the header file.
FIND_PATH(CURLPP_INCLUDE_DIR NAMES curlpp/cURLpp.hpp)

# Look for the library.
FIND_LIBRARY(CURLPP_LIBRARY NAMES libcurlpp.so)

# Handle the QUIETLY and REQUIRED arguments and set CURLPP_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CURLPP DEFAULT_MSG CURLPP_LIBRARY CURLPP_INCLUDE_DIR)

# Copy the results to the output variables.
IF(CURLPP_FOUND)
  SET(CURLPP_LIBRARIES ${CURL_LIBRARIES} ${CURLPP_LIBRARY})
  SET(CURLPP_INCLUDE_DIRS ${CURL_INCLUDE_DIRS} ${CURLPP_INCLUDE_DIR})
ELSE(CURLPP_FOUND)
	SET(CURLPP_LIBRARIES)
	SET(CURLPP_INCLUDE_DIRS)
ENDIF(CURLPP_FOUND)

MARK_AS_ADVANCED(CURLPP_INCLUDE_DIRS CURLPP_LIBRARIES)