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
find_package(Boost REQUIRED)
find_package(OpenGL REQUIRED)

include_directories("${PROJECT_SOURCE_DIR}/build/include")          #For OpenAL and OpenGL include files
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${SOIL_ROOT}/src)
include_directories(${LUA_INCLUDE})

add_executable(DoodAnimTool ${ANITOOL_SOURCES})

target_include_directories(DoodAnimTool PUBLIC CibraryEngine)
target_link_libraries(DoodAnimTool PUBLIC CibraryEngine)
target_link_libraries(DoodAnimTool PUBLIC lua51)

#todo: link stuffs