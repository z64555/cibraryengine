message(STATUS "Parsing './DestructibleTerrain/CMakeLists.txt")
#./DestructibleTerrain/CMakeLists.txt
set(DT_SOURCES 
	DTMain.cpp
	DTScreen.cpp
	DTScreen.h
	StdAfx.cpp
	StdAfx.h
)

## Resources (scripts, artwork, etc) directories
include_directories("${PROJECT_SOURCE_DIR}/Files")

## Third party libs and utils directores
# Boost
find_package(Boost COMPONENTS date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

# OpenAL
# Found in root CMakeLists.txt
include_directories(${OPENAL_INCLUDE})
link_directories(${OPENAL_LIB})

# OpenGL
# Found in root CMakeLists.txt
include_directories(${OPENGL_INCLUDE})

#include_directories("${PROJECT_SOURCE_DIR}/build/include")          #For OpenAL and OpenGL include files

# SOIL
include_directories(${SOIL_INCLUDE})
link_directories(${SOIL_LIB})

# LUA
# Found in root CMakeLists.txt
include_directories(${LUA_INCLUDE})

## Add target
add_executable(DestructibleTerrain ${DT_SOURCES})

## Link intra-project dependencies
target_include_directories(DestructibleTerrain PUBLIC CibraryEngine)
target_link_libraries(DestructibleTerrain PUBLIC CibraryEngine)

## Link external dependencies
target_link_libraries(DestructibleTerrain PUBLIC ${Boost_LIBRARIES})
target_link_libraries(DestructibleTerrain PUBLIC ${LUA_LIB})
target_link_libraries(DestructibleTerrain PUBLIC OpenAL32)
target_link_libraries(DestructibleTerrain PUBLIC ${SOIL_LIBRARIES})

# Windows libraries
target_link_libraries(DestructibleTerrain PUBLIC Shcore)