# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find HTMLCXX
# Find the native htmlcxx headers and libraries.
#
# HTMLCXX_INCLUDE_DIRS	- where to find headers
# HTMLCXX_LIBRARIES	- List of libraries when using htmlcxx.
# HTMLCXX_FOUND	- True if htmlcxx found.

# Look for the header file.
FIND_PATH(HTMLCXX_INCLUDE_DIR NAMES htmlcxx/html/ParserDom.h)

# Look for the library.
FIND_LIBRARY(HTMLCXX_LIBRARY NAMES htmlcxx)

# Handle the QUIETLY and REQUIRED arguments and set HTMLCXX_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HTMLCXX DEFAULT_MSG HTMLCXX_LIBRARY HTMLCXX_INCLUDE_DIR)

# Copy the results to the output variables.
IF(HTMLCXX_FOUND)
	SET(HTMLCXX_LIBRARIES ${HTMLCXX_LIBRARY})
	SET(HTMLCXX_INCLUDE_DIRS ${HTMLCXX_INCLUDE_DIR})
ELSE(HTMLCXX_FOUND)
	SET(HTMLCXX_LIBRARIES)
	SET(HTMLCXX_INCLUDE_DIRS)
ENDIF(HTMLCXX_FOUND)

MARK_AS_ADVANCED(HTMLCXX_INCLUDE_DIRS HTMLCXX_LIBRARIES)