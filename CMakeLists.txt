message(STATUS "Parsing './CMakeLists.txt'")
## ./CMakeLists.txt
cmake_minimum_required(VERSION 3.12)

## The project name.  Is the name of the solution files and default name of the executable.
project(cibrary)

## Where the binaries should be copied to on install.  Since this is still a bunch of tests, this should point to \bin
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE FILEPATH "Install path" FORCE)

## Versioning
#set(VERSION_MAJOR 0)
#set(VERSION_MINOR 1)

cmake_policy(VERSION 3.12)   #Use new behavior for find_package()

## Third Party libs/packages configurations
# Boost
set(Boost_USE_MULTITHREADED  ON  CACHE BOOL "Set to OFF to use the non-multithreaded libraries ('mt' tag).  Default is ON")
set(Boost_USE_STATIC_LIBS    OFF CACHE BOOL "Set to ON to force the use of the static libraries.  Default is OFF")
set(Boost_USE_STATIC_RUNTIME OFF CACHE BOOL "Set to ON or OFF to specify whether to use libraries linked statically to the C++ runtime ('s' tag).  Default is platform dependent")
set(Boost_USE_DEBUG_RUNTIME  ON  CACHE BOOL "Set to ON or OFF to specify whether to use libraries linked to the MS debug C++ runtime ('g' tag).  Default is ON.")

# LUA
find_package(Lua51)
if(NOT LUA51_FOUND)
	message(SEND_ERROR "Could not find Lua51, please specify the include and lib directories")
	set(LUA_INCLUDE    $ENV{LUA_DIR}    CACHE PATH "Where lua.h is (Lua 5.1).  Defaults to $LUA_DIR$")
	set(LUA_LIB        ""               CACHE FILEPATH "Where lua and lualib are. (For Windows, these are .dll and .lib)")
else()
	set(LUA_INCLUDE ${LUA_INCLUDE_DIR} CACHE PATH "Where lua.h is (Lua 5.1).")
	set(LUA_LIB     ${LUA_LIBRARIES}   CACHE FILEPATH "Where lua and lualib are. (For Windows, these are .libs)")
endif()

# OpenAL
find_package(OpenAL)
if(NOT OPENAL_FOUND)
	message(SEND_ERROR "Could not find OpenAL, please specify the include and lib directories")
	set(OPENAL_INCLUDE  $ENV{OPENALDIR} CACHE PATH "Where the al.h is.  Default is ./OpenAL SDK/include")
	set(OPENAL_LIB      ""              CACHE PATH "Where the OpenAL32.lib is.  Default is ./OpenAL SDK/libs/Win32")
else()
	get_filename_component(OPENAL_LIBRARY_DIRS ${OPENAL_LIBRARY} DIRECTORY)
	set(OPENAL_INCLUDE  ${OPENAL_INCLUDE_DIR}   CACHE PATH "Where the al.h is.  Default is ./OpenAL SDK/include")
	set(OPENAL_LIB      ${OPENAL_LIBRARY_DIRS}  CACHE PATH "Where the OpenAL32.lib is.  Default is ./OpenAL SDK/libs/Win32")
endif()

#message(STATUS "OPENAL_INCLUDE_DIR = ${OPENAL_INCLUDE_DIR}")
#message(STATUS "OPENAL_LIBRARY = ${OPENAL_LIBRARY}")

get_filename_component(OPENAL_LIBRARY_DIRS ${OPENAL_LIBRARY} DIRECTORY)
#message(STATUS "OPENAL_LIBRARY_DIRS = ${OPENAL_LIBRARY_DIRS}")


# OpenGL
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL found?........${OPENGL_FOUND}")
message(STATUS "GLU found?...........${OPENGL_GLU_FOUND}")
message(STATUS "OpenGL Lib found?....${OpenGL_OpenGL_FOUND}")
message(STATUS "OPENGL_INCLUDE_DIRS..${OPENGL_INCLUDE_DIRS}")
message(STATUS "OPENGL_LIBRARIES.....${OPENGL_LIBRARIES}")

# SOIL
## This form of set allows editing in cmake-gui before generation
## set(<var_name> <default_val> CACHE <datatype> [Description])
set(SOIL_ROOT  $ENV{SOIL_ROOT}  CACHE FILEPATH "Root directory of your SOIL installation")


## Project Source, Libs, and Utils directories
## These get done last, because CMake will dive right into each subdir's generation before finishing here
message(STATUS "Adding project dirs...")
add_subdirectory(CibraryEngine)          #Engine
add_subdirectory(DestructibleTerrain)    #Destructable Terrain test
add_subdirectory(DoodAnimTool)           #Actor Animation Tool
add_subdirectory(TestProject)            #Game Test