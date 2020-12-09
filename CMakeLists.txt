# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14)

# Subproject of efika
project(efika-io)

# Include required CMake modules
include(CheckSymbolExists)
include(FetchContent)

#-------------------------------------------------------------------------------
# FUNCTION availability
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# COMPONENT configuration
#-------------------------------------------------------------------------------
add_library(${PROJECT_NAME}     OBJECT)
add_library(${PROJECT_NAME}-sdk INTERFACE)

target_sources(${PROJECT_NAME}
  PRIVATE src/getline.c src/cluto.c src/dimacs.c src/metis.c src/mm.c src/snap.c
          #[[src/ugraph.c]])

target_include_directories(${PROJECT_NAME}
  PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

target_include_directories(${PROJECT_NAME}-sdk
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src/include)

add_library(Efika::io    ALIAS ${PROJECT_NAME})
add_library(EfikaSDK::io ALIAS ${PROJECT_NAME}-sdk)

#-------------------------------------------------------------------------------
# FEATURE AVAILABILITY checks
#-------------------------------------------------------------------------------
set(CMAKE_REQUIRED_DEFINITIONS "-D_POSIX_C_SOURCE=200809L")

check_symbol_exists(getline "stdio.h" HAVE_GETLINE)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC $<$<BOOL:${HAVE_GETLINE}>:HAVE_GETLINE>)

#-------------------------------------------------------------------------------
# Declare efika dependencies
#-------------------------------------------------------------------------------
foreach(dep core)
  FetchContent_Declare(efika-${dep}
    GIT_REPOSITORY https://github.com/jiverson002/efika-${dep}.git)

  FetchContent_MakeAvailable(efika-${dep})

  target_link_libraries(${PROJECT_NAME} PRIVATE Efika::${dep} EfikaSDK::${dep})
endforeach()

#-------------------------------------------------------------------------------
# Unit test configuration
#-------------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include( CTest )
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()
