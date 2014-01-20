#pragma once

#include "StdAfx.h"

#include "Constraint.h"
#include "DATKeyframe.h"

namespace DoodAnimTool
{
	using namespace CibraryEngine;

	struct DATBone;

	class CSkeletalJoint : public Constraint
	{
		public:

			ModelPhysics::JointPhysics* joint;

		private:

			int bone_a, bone_b;

			DATKeyframe::KBone* initial_a;
			DATKeyframe::KBone* initial_b;

			DATKeyframe::KBone* obja;
			DATKeyframe::KBone* objb;

			DATKeyframe::KBone* nexta;
			DATKeyframe::KBone* nextb;

			Vec3 joint_pos;
			Vec3 lcenter_a, lcenter_b;

			float ComputeMatricesAndEndpoints(const Vec3& apos, const Vec3& bpos, const Quaternion& aori, const Quaternion& bori, Mat4& amat, Mat4& bmat, Vec3& aend, Vec3& bend, Vec3& dx);
			void DoHalfRot(Vec3& rot, const Vec3& bone_point, const Vec3& bone_center, const Vec3& midpoint);

		public:

			bool enforce_rotation_limits;

			CSkeletalJoint(ModelPhysics::JointPhysics* joint, const vector<DATBone>& bones);
			~CSkeletalJoint();

			void InitCachedStuff(PoseSolverState& pose);
			bool ApplyConstraint(PoseSolverState& pose);
			void OnAnyChanges(PoseSolverState& pose);

			float GetErrorAmount(const DATKeyframe& pose);
	};
}
