## ./CMakeLists.txt
cmake_minimum_required(VERSION 2.6)

## The project name.  Is the name of the solution files and default name of the executable.
project(cibrary)

## Where the binaries should be copied to on install.  Since this is still a bunch of tests, this should point to \bin
set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/bin" CACHE FILEPATH "Install path" FORCE)

## Versioning
#set(VERSION_MAJOR 0)
#set(VERSION_MINOR 1)

## Source, Libs, an Utils directories
add_subdirectory(CibraryEngine)          #Engine
add_subdirectory(DestructibleTerrain)    #Destructable Terrain test
add_subdirectory(DoodAnimTool)           #Actor Animation Tool
add_subdirectory(TestProject)            #Game Test

## Resources (scripts, artwork, etc) directories
include_directories("${PROJECT_SOURCE_DIR}/Files")

## Third party libs an utils directores
## This form of set allows editing in cmake-gui before generation
## set(<var_name> <default_val> CACHE <datatype> [Description])
set(SOIL_ROOT  $ENV{SOIL_ROOT}  CACHE FILEPATH "Root directory of your SOIL installation")
set(LUA_SRC    $ENV{LUA_SRC}    CACHE FILEPATH "Root directory of your LUA installation")

find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

#things to do still:
# add_executable to the three tools and the test project
# add_library to the engine lib
# determine how to set the default project to build, which at this point should be TestProject
