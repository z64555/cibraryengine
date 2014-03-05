#pragma once

#include "StdAfx.h"

#include "Dood.h"

namespace Test
{
	class PoseAimingGun;
	class WalkPose;

	class Soldier : public Dood
	{
		protected:

			void DoJumpControls(const TimingInfo& time, const Vec3& forward, const Vec3& rightward);

			void PreUpdatePoses(const TimingInfo& time);

			void MaybeSinkCheatyVelocity(float timestep, Vec3& cheaty_vel, Vec3& cheaty_rot, float net_mass, const Mat3& net_moi);

			Vec3 ComputeDesiredVelocity();
			void SetRootBoneXform(const Vec3& desired_vel);
			bool OverrideFootfallSafety();

			Mat4 proposed_pelvis_xform;

		public:

			Bone* gun_hand_bone;

			PoseAimingGun* p_ag;
			WalkPose* walk_pose;

			float jet_fuel;

			vector<RigidBody*> jet_bones;			// bones to which the jetpack applies force

			SoundBuffer* jet_start_sound;
			SoundBuffer* jet_loop_sound;
			SoundSource* jet_loop;

			Soldier(GameState* game_state, UberModel* model, ModelPhysics* mphys, const Vec3& pos, Team& team);

			void PhysicsToCharacter();
			void RegisterFeet();
			void Update(const TimingInfo& time);

			void Die(const Damage& cause);

			void Spawned();
	};
}
