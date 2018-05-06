 cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

enable_testing()

# set Release build type as default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING
    "Configuration type (one of Debug, RelWithDebInfo, Release, MinSizeRel)"
    FORCE)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMakeModules/")

project(CoinLedger CXX)

# enable warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# enable C++11
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

# generate debugging symbols for release and debug
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

# explicitly set DEBUG flag in Debug mode
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG")

# to make shared libraries we need position independent code
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

# enable writing compile_command.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# add the source directory to the include list
include_directories(${CoinLedger_SOURCE_DIR}/src)

# we need Boost
find_package(Boost REQUIRED filesystem)
set(COINLEDGER_EXTERNAL_LIBS "${COINLEDGER_EXTERNAL_LIBS};${Boost_LIBRARIES}")
include_directories(${Boost_INCLUDE_DIRS})

# we need CURLpp
find_package(CURLPP REQUIRED)
set(COINLEDGER_EXTERNAL_LIBS "${COINLEDGER_EXTERNAL_LIBS};${CURLPP_LIBRARIES}")
include_directories(${CURLPP_INCLUDE_DIRS})

# we need JSONCPP
find_package(JSONCPP REQUIRED)
set(COINLEDGER_EXTERNAL_LIBS "${COINLEDGER_EXTERNAL_LIBS};${JSONCPP_LIBRARIES}")
include_directories(${JSONCPP_INCLUDE_DIRS})

# we need SQLITE3
find_package(SQLITE3 REQUIRED)
set(COINLEDGER_EXTERNAL_LIBS "${COINLEDGER_EXTERNAL_LIBS};${SQLITE3_LIBRARIES}")
include_directories(${SQLITE3_INCLUDE_DIRS})

# we need CSV
find_package(CSV REQUIRED)
set(COINLEDGER_EXTERNAL_LIBS "${COINLEDGER_EXTERNAL_LIBS};${CSV_LIBRARIES}")
include_directories(${CSV_INCLUDE_DIRS})

# we need SWIG
find_package(SWIG 3 REQUIRED)
find_package(PythonLibs 2.6 REQUIRED)
include(${SWIG_USE_FILE})
include_directories(${PYTHON_INCLUDE_DIRS})

# we need LibXML++
find_package(LibXML++ REQUIRED)
include_directories(${LibXML++_INCLUDE_DIRS})
set(COINLEDGER_EXTERNAL_LIBS "${COINLEDGER_EXTERNAL_LIBS};${LibXML++_LIBRARIES}")

macro(add_CoinLedger_library library sources)
  add_library(${library} OBJECT ${sources})
  set(COINLEDGER_LIBS "${COINLEDGER_LIBS};$<TARGET_OBJECTS:${library}>" PARENT_SCOPE)
  add_dependencies(${library} src)
endmacro()

macro(add_SWIG_dependencies)
  foreach(dep ${ARGN})
    list(APPEND SWIG_MODULE_CoinLedger_EXTRA_DEPS "${CMAKE_CURRENT_SOURCE_DIR}/${dep}")
  endforeach(dep)
  set(SWIG_MODULE_CoinLedger_EXTRA_DEPS ${SWIG_MODULE_CoinLedger_EXTRA_DEPS} PARENT_SCOPE)
endmacro()

add_subdirectory(src)

# create static library that will be used to link tests and SWIG library
add_library(CoinLedger_static
  ${COINLEDGER_LIBS}
)

# create shared library that can be used to link other C++ applications
add_library(CoinLedger SHARED
  ${COINLEDGER_LIBS}
)

add_subdirectory(bin)

# create SWIG Python bindings
SET_SOURCE_FILES_PROPERTIES("src/CoinLedger.i" PROPERTIES CPLUSPLUS ON)
SWIG_ADD_LIBRARY(CoinLedger LANGUAGE python SOURCES "src/CoinLedger.i")

SWIG_LINK_LIBRARIES(CoinLedger
  CoinLedger_static
  ${COINLEDGER_EXTERNAL_LIBS}
  ${PYTHON_LIBRARIES}
)
