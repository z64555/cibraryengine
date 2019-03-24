#./DestructibleTerrain/CMakeLists.txt
set(DT_SOURCES 
	DTMain.cpp
	DTScreen.cpp
	DTScreen.h
	StdAfx.cpp
	StdAfx.h
)

add_executable(DestructibleTerrain ${DT_SOURCES})

#todo: Link stuffs