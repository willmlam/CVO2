# CMake file for CVO

cmake_minimum_required(VERSION 2.8)
project(CVO)

# to enable static linking
option(LINK_STATIC "Link binary statically" OFF)

add_definitions(-DDEFINE_PACE16_MAIN_FN)

if(WIN32)
  add_definitions(-DWINDOWS)
else()
  add_definitions(-DLINUX)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(NOT WIN32)
  set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -w -fpermissive -fPIC")
  set(CMAKE_C_FLAGS_DEBUG "-O0 -g3 -fPIC")
  set(CMAKE_CXX_FLAGS_RELEASE "-O3 -w -fpermissive -fPIC")
  set(CMAKE_C_FLAGS_RELEASE "-O3 -fPIC")
else()
  set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG /Od /J")
  set(CMAKE_C_FLAGS_DEBUG "-DDEBUG /Od /J")
  set(CMAKE_CXX_FLAGS_RELEASE "/Ox /J")
  set(CMAKE_C_FLAGS_RELEASE "/Ox /J")
endif()


if(NOT WIN32)
  include(CheckCXXCompilerFlag)
  CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
  if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++11")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++11")
  else()
    message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
  endif()
endif()

if(LINK_STATIC)
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif()

find_package(Threads)

include_directories(
  ./
  ./ARP
  ./ARP/BE
  ./ARP/CVO
  ./ARP/Problem
  ./ARP/Utils
  ./miniSAT
)

# MiniSAT
add_subdirectory(miniSAT)

# Main executable
add_executable(tw-heuristic
  ARP/BE/MiniBucket.cpp
  ARP/BE/Bucket.cpp
  ARP/BE/MBEworkspace.cpp
  ARP/CVO/Graph.cpp
  ARP/CVO/Graph_MinFillOrderComputation.cpp
  ARP/CVO/VariableOrderComputation.cpp
  ARP/CVO/Graph_RemoveRedundantFillEdges.cpp
  ARP/Problem/Problem.cpp
  ARP/Problem/Globals.cpp
  ARP/Problem/Workspace.cpp
  ARP/Problem/Function.cpp
  ARP/Utils/AVLtreeSimple.cpp
  ARP/Utils/Mutex.cpp
  ARP/Utils/MiscUtils.cpp
  ARP/Utils/FnExecutionThread.cpp
  ARP/Utils/Sort.cxx
  $<TARGET_OBJECTS:Minisat>
)
if (LINK_STATIC)
  SET_TARGET_PROPERTIES(tw-heuristic PROPERTIES LINK_SEARCH_START_STATIC 1)
  SET_TARGET_PROPERTIES(tw-heuristic PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()
target_link_libraries(tw-heuristic ${CMAKE_THREAD_LIBS_INIT})
