#pragma once
#include "StdAfx.h"

#include "Physics.h"

#include "Matrix.h"

namespace CibraryEngine
{
	using namespace std;

	class RigidBody;

	class JointConstraint : public PhysicsConstraint
	{
		protected:

			// some values that are set by DoUpdateAction and which should only be used for the duration of a single physics tick
			Mat3 moi;
			Vec3 apply_pos;
			Vec3 desired_av, desired_dv;

			Mat3 oriented_axes, reverse_oriented_axes;
			Quaternion a_to_b, b_to_a;

			Vec3 r1, r2;
			Mat3 alpha_to_obja, alpha_to_objb;

			float timestep, inv_timestep;

		public:

			/** Position of the joint in the coordinate system of the first bone */
			Vec3 pos;
			/** Axes of this joint relative to the first bone; x and y axes are swing axes; z axis is twist axis */
			Mat3 axes;
			/** Minimum angle the joint can turn in each direction, in the coordinate system specified by axes */
			Vec3 min_extents;
			/** Maximum angle the joint can turn in each direction, in the coordinate system specified by axes */
			Vec3 max_extents;

			JointConstraint(RigidBody* ibody, RigidBody* jbody, const Vec3&, const Mat3& axes, const Vec3& min_extents, const Vec3& max_extents);

			void DoConstraintAction(vector<RigidBody*>& wakeup);
			void DoUpdateAction(float timestep);
	};
}
