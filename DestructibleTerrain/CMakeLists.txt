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
find_package(Boost REQUIRED)
find_package(OpenGL REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/build/include")          #For OpenAL and OpenGL include files
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${SOIL_ROOT}/src)
include_directories(${LUA_INCLUDE})

add_executable(DestructibleTerrain ${DT_SOURCES})

## Link intra-project dependencies
target_include_directories(DestructibleTerrain PUBLIC CibraryEngine)
target_link_libraries(DestructibleTerrain PUBLIC CibraryEngine)

## Link external dependencies
target_link_libraries(DestructibleTerrain PUBLIC ${Boost_LIBRARIES})

target_link_directories(DestructibleTerrain PUBLIC LUA_LIB)
target_link_libraries(DestructibleTerrain PUBLIC lua51)