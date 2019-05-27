## ./CMakeLists.txt
cmake_minimum_required(VERSION 3.13)

## The project name.  Is the name of the solution files and default name of the executable.
project(cibrary)

## Where the binaries should be copied to on install.  Since this is still a bunch of tests, this should point to \bin
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE FILEPATH "Install path" FORCE)

## Versioning
#set(VERSION_MAJOR 0)
#set(VERSION_MINOR 1)

## Project Source, Libs, and Utils directories
add_subdirectory(CibraryEngine)          #Engine
add_subdirectory(DestructibleTerrain)    #Destructable Terrain test
add_subdirectory(DoodAnimTool)           #Actor Animation Tool
add_subdirectory(TestProject)            #Game Test

cmake_policy(VERSION 3.13)   #Use new behavior for find_package()


## Third Party libs/packages configurations
# Boost
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

#lua
find_package(Lua51)
if(NOT LUA_FOUND)
	set(LUA_INCLUDE    $ENV{LUA_SRC}    CACHE FILEPATH "Where lua.h is (Lua 5.1)")
	set(LUA_LIB        ""               CACHE FILEPATH "Where lua and lualib are. (For Windows, these are .libs)")
else()
	set(LUA_INCLUDE LUA_INCLUDE_DIR CACHE FILEPATH "Where lua.h is (Lua 5.1)")
	set(LUA_LIB     LUA_LIBRARIES   CACHE FILEPATH "Where lua and lualib are. (For Windows, these are .libs)")
endif()

#SOIL
## This form of set allows editing in cmake-gui before generation
## set(<var_name> <default_val> CACHE <datatype> [Description])
set(SOIL_ROOT  $ENV{SOIL_ROOT}  CACHE FILEPATH "Root directory of your SOIL installation")