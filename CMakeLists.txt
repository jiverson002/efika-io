# SPDX-License-Identifier: MIT
cmake_minimum_required(VERSION 3.14)

#-------------------------------------------------------------------------------
# PROJECT configuration
#-------------------------------------------------------------------------------
set(Library_NAME Efika CACHE STRING "The name of the main library.")
set(component_NAME io)

string(TOLOWER ${Library_NAME} library_NAME)

project(${library_NAME}-${component_NAME})

#-------------------------------------------------------------------------------
# COMPONENT configuration
#-------------------------------------------------------------------------------
add_library(${PROJECT_NAME} OBJECT)
add_library(${Library_NAME}::${component_NAME} ALIAS ${PROJECT_NAME})

target_sources(${PROJECT_NAME}
  PRIVATE src/getline.c src/cluto.c src/dimacs.c src/metis.c src/mm.c src/snap.c
          #[[src/ugraph.c]])

target_include_directories(${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/include>
         $<INSTALL_INTERFACE:include>)

#-------------------------------------------------------------------------------
# FEATURE AVAILABILITY checks
#-------------------------------------------------------------------------------
include(CheckSymbolExists)

set(CMAKE_REQUIRED_DEFINITIONS "-D_POSIX_C_SOURCE=200809L")

check_symbol_exists(getline "stdio.h" HAVE_GETLINE)

target_compile_definitions(${PROJECT_NAME}
  PUBLIC $<$<BOOL:${HAVE_GETLINE}>:HAVE_GETLINE>)

#-------------------------------------------------------------------------------
# INTERNAL DEPENDENCY configuration
#-------------------------------------------------------------------------------
include(FetchContent)

set(component_DEPS "core")

foreach(dep ${component_DEPS})
  FetchContent_Declare(${library_NAME}-${dep}
    GIT_REPOSITORY https://github.com/jiverson002/${library_NAME}-${dep}.git)

  FetchContent_MakeAvailable(${library_NAME}-${dep})

  target_link_libraries(${PROJECT_NAME} PRIVATE ${Library_NAME}::${dep})
endforeach()

#-------------------------------------------------------------------------------
# TEST configuration
#-------------------------------------------------------------------------------
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  if(BUILD_TESTING)
    add_subdirectory(test)
  endif()
endif()

#-------------------------------------------------------------------------------
# INSTALL configuration
#-------------------------------------------------------------------------------
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

configure_package_config_file(${component_NAME}-config.cmake.in
                      cmake/${component_NAME}-config.cmake
  INSTALL_DESTINATION lib/cmake/${library_NAME})

install(TARGETS ${PROJECT_NAME}
  EXPORT   ${PROJECT_NAME}Targets
  RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR}
           COMPONENT   ${Library_NAME}_Runtime
  LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
           COMPONENT          ${Library_NAME}_Runtime
           NAMELINK_COMPONENT ${Library_NAME}_Development
  ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
           COMPONENT   ${Library_NAME}_Development
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT ${PROJECT_NAME}Targets
  FILE        ${component_NAME}-targets.cmake
  NAMESPACE   ${Library_NAME}::
  DESTINATION lib/cmake/${library_NAME}
  COMPONENT   ${Library_NAME}_Development)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  COMPONENT   ${Library_NAME}_Development)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/cmake/
  DESTINATION lib/cmake/${library_NAME}
  COMPONENT   ${Library_NAME}_Development)
