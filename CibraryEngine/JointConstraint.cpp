#include "StdAfx.h"
#include "JointConstraint.h"

#include "RigidBody.h"

#define ENFORCE_JOINT_ROTATION_LIMITS 1

namespace CibraryEngine
{
	/*
	 * JointConstraint methods
	 */
	JointConstraint::JointConstraint(RigidBody* ibody, RigidBody* jbody, const Vec3& pos, const Mat3& axes, const Vec3& min_extents, const Vec3& max_extents) :
		PhysicsConstraint(ibody, jbody),
		pos(pos),
		axes(axes),
		min_extents(min_extents),
		max_extents(max_extents),
		desired_ori(Quaternion::Identity()),
		enable_motor(false),
		motor_torque()
	{
	}

	bool JointConstraint::DoConstraintAction()
	{
		static const float dv_coeff =			1.0f;

		static const float dv_sq_threshold	=	0.01f;
		static const float alpha_sq_threshold =	0.01f;


		bool wakeup = false;

		// linear stuff
		Vec3 current_dv = obj_b->vel - obj_a->vel + Vec3::Cross(r2, obj_b->rot) - Vec3::Cross(r1, obj_a->rot);

		Vec3 dv = desired_dv - current_dv;
		float magsq = dv.ComputeMagnitudeSquared();
		if(magsq > dv_sq_threshold)
		{
			Vec3 impulse = rlv_to_impulse * dv;

			// apply impulse
			if(obj_a->active)
			{
				obj_a->vel += impulse * obj_a->inv_mass;
				if(obj_a->can_rotate)
					obj_a->rot += obj_a->inv_moi * Vec3::Cross(impulse, r1);
			}

			if(obj_b->active && obj_b->can_move)
			{
				obj_b->vel -= impulse * obj_b->inv_mass;
				if(obj_b->can_rotate)
					obj_b->rot -= obj_b->inv_moi * Vec3::Cross(impulse, r2);
			}

			wakeup = true;
		}


		// angular stuff
		Vec3 current_av = obj_b->rot - obj_a->rot;
		Vec3 alpha;												// delta-angular-velocity

		if(enable_motor)
			alpha = current_av - desired_av;

#if ENFORCE_JOINT_ROTATION_LIMITS
		// enforce joint rotation limits
		Vec3 proposed_av = current_av - alpha;
		Quaternion proposed_ori = a_to_b * Quaternion::FromPYR(proposed_av.x * timestep, proposed_av.y * timestep, proposed_av.z * timestep);
		Vec3 proposed_pyr = oriented_axes * -proposed_ori.ToPYR();

		bool any_changes = false;
		if      (proposed_pyr.x < min_extents.x) { proposed_pyr.x = min_extents.x; any_changes = true; }
		else if (proposed_pyr.x > max_extents.x) { proposed_pyr.x = max_extents.x; any_changes = true; }
		if      (proposed_pyr.y < min_extents.y) { proposed_pyr.y = min_extents.y; any_changes = true; }
		else if (proposed_pyr.y > max_extents.y) { proposed_pyr.y = max_extents.y; any_changes = true; }
		if      (proposed_pyr.z < min_extents.z) { proposed_pyr.z = min_extents.z; any_changes = true; }
		else if (proposed_pyr.z > max_extents.z) { proposed_pyr.z = max_extents.z; any_changes = true; }

		if(any_changes)
		{
			// at least one rotation limit was violated, so we must recompute alpha
			Quaternion actual_ori = Quaternion::FromPYR(reverse_oriented_axes * -proposed_pyr);
			Vec3 actual_av = (b_to_a * actual_ori).ToPYR() * inv_timestep;

			alpha = current_av - actual_av;
		}
#endif

		// apply angular velocity changes
		magsq = alpha.ComputeMagnitudeSquared();
		if(magsq > alpha_sq_threshold)
		{
			obj_a->rot += alpha_to_obja * alpha;
			obj_b->rot -= alpha_to_objb * alpha;

			wakeup = true;
		}

		return wakeup;
	}

	void JointConstraint::DoUpdateAction(float timestep_)
	{
		timestep = timestep_;
		inv_timestep = 1.0f / timestep;

		const float spring_coeff =				1.0f;
		const float motor_coeff =				1.0f;

		Quaternion a_ori = obj_a->GetOrientation();
		Quaternion b_ori = obj_b->GetOrientation();

		a_to_b = Quaternion::Reverse(a_ori) * b_ori;
		b_to_a = Quaternion::Reverse(a_to_b);

		Mat3 net_moi = Mat3::Invert(obj_a->GetInvMoI() + obj_b->GetInvMoI());
		alpha_to_obja = obj_a->inv_moi * net_moi;
		alpha_to_objb = obj_b->inv_moi * net_moi;

		oriented_axes = axes * a_ori.ToMat3();
		reverse_oriented_axes = oriented_axes.Transpose();


		// force to keep the two halves of the joint together
		Vec3 a_pos = obj_a->GetTransformationMatrix().TransformVec3_1(pos);
		Vec3 b_pos = obj_b->GetTransformationMatrix().TransformVec3_1(pos);

		apply_pos = (a_pos + b_pos) * 0.5f;
		desired_dv = (b_pos - a_pos) * -(spring_coeff * inv_timestep);

		r1 = apply_pos - obj_a->cached_com;
		r2 = apply_pos - obj_b->cached_com;

		// computing rlv-to-impulse matrix
		Mat3 xr1(
				0,	  r1.z,	  -r1.y,
			-r1.z,	     0,	   r1.x,
			 r1.y,	 -r1.x,	      0		);
		Mat3 xr2(
				0,	  r2.z,	  -r2.y,
			-r2.z,	     0,	   r2.x,
			 r2.y,	 -r2.x,	      0		);

		float invmasses = -(obj_a->inv_mass + obj_b->inv_mass);
		Mat3 impulse_to_rlv = Mat3(invmasses, 0, 0, 0, invmasses, 0, 0, 0, invmasses)
			+ xr1 * obj_a->inv_moi * xr1
			+ xr2 * obj_b->inv_moi * xr2;

		rlv_to_impulse = Mat3::Invert(impulse_to_rlv);


		desired_av = -(Quaternion::Reverse(desired_ori) * a_to_b).ToPYR() * (motor_coeff * inv_timestep);

		// apply motor torque
		Vec3 motor_aimpulse = reverse_oriented_axes * (motor_torque * timestep);
		obj_a->ApplyAngularImpulse(motor_aimpulse);
		obj_b->ApplyAngularImpulse(-motor_aimpulse);
	}
}
