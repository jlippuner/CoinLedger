# Copyright (C) 2007-2009 LuaDist.
# Created by Peter Kapec <kapecp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#	Note:
#		Searching headers and libraries is very simple and is NOT as powerful as scripts
#		distributed with CMake, because LuaDist defines directories to search for.
#		Everyone is encouraged to contact the author with improvements. Maybe this file
#		becomes part of CMake distribution sometimes.

# - Find csv
# Find the native CSV headers and libraries.
#
# CSV_INCLUDE_DIRS	- where to find csv.h, etc.
# CSV_LIBRARIES	- List of libraries when using csv.
# CSV_FOUND	- True if csv found.

# Look for the header file.
FIND_PATH(CSV_INCLUDE_DIR NAMES csv.h)

# Look for the library.
FIND_LIBRARY(CSV_LIBRARY NAMES csv)

# Handle the QUIETLY and REQUIRED arguments and set CSV_FOUND to TRUE if all listed variables are TRUE.
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CSV DEFAULT_MSG CSV_LIBRARY CSV_INCLUDE_DIR)

# Copy the results to the output variables.
IF(CSV_FOUND)
	SET(CSV_LIBRARIES ${CSV_LIBRARY})
	SET(CSV_INCLUDE_DIRS ${CSV_INCLUDE_DIR})
ELSE(CSV_FOUND)
	SET(CSV_LIBRARIES)
	SET(CSV_INCLUDE_DIRS)
ENDIF(CSV_FOUND)

MARK_AS_ADVANCED(CSV_INCLUDE_DIRS CSV_LIBRARIES)