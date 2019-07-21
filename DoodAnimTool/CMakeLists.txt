message(STATUS "Parsing './DoodAnimTool/CMakeLists.txt'")
#./DoodAnimTool/CMakeLists.txt

set(ANITOOL_SOURCES
	CAlignedAxis.cpp
	CAlignedAxis.h
	CFixedJoint.cpp
	CFixedJoint.h
	CFlatFoot.cpp
	CFlatFoot.h
	Constraint.h
	CSkeletalJoint.cpp
	CSkeletalJoint.h
	DATBone.cpp
	DATBone.h
	DATKeyframe.cpp
	DATKeyframe.h
	DATMain.cpp
	DATScreen.cpp
	DATScreen.h
	FileSelection.cpp
	FileSelection.h
	FixedJointPoseOp.h
	GCanvas.cpp
	GCanvas.h
	GCheckbox.cpp
	GCheckbox.h
	GColumnList.cpp
	GColumnList.h
	GLabel.cpp
	GLabel.h
	GUIComponent.h
	JointOrientations.cpp
	JointOrientations.h
	PoseChainNode.h
	PoseDelta.h
	PoseyDood.cpp
	PoseyDood.h
	SolverInstance.cpp
	SolverInstance.h
	StdAfx.cpp
	StdAfx.h
)

## Resources (scripts, artwork, etc) directories
include_directories("${PROJECT_SOURCE_DIR}/Files")

## Third party libs an utils directores
## Third party libs and utils directores
#Boost
find_package(Boost COMPONENTS date_time REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

#OpenGL and OpenAL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories("${PROJECT_SOURCE_DIR}/build/include")          #For OpenAL and OpenGL include files

#Soil
include_directories(${SOIL_ROOT}/src)

#Lua
include_directories(${LUA_INCLUDE})

## Add target
add_executable(DoodAnimTool ${ANITOOL_SOURCES})

## Link intra-project dependencies
target_include_directories(DoodAnimTool PUBLIC CibraryEngine)
target_link_libraries(DoodAnimTool PUBLIC CibraryEngine)

## Link external dependencies
target_link_libraries(DoodAnimTool PUBLIC ${Boost_LIBRARIES})
target_link_libraries(DoodAnimTool PUBLIC ${LUA_LIB})