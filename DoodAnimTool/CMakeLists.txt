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

add_executable(DoodAnimTool ${ANITOOL_SOURCES})

#todo: link stuffs