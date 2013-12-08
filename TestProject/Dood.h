#pragma once

#include "StdAfx.h"

#include "Shootable.h"
#include "Damage.h"

#include "Team.h"

namespace Test
{
	using namespace std;
	using namespace CibraryEngine;

	class WeaponEquip;
	class WeaponIntrinsic;

	class PlacedFootConstraint;

	struct Damage;

	class Dood : public Pawn
	{
		private:

			float pose_timer;

		protected:

			struct BoneShootable : Entity, Shootable
			{
				Dood* dood;
				RigidBody* body;

				BillboardMaterial* blood_material;

				BoneShootable(GameState* gs, Dood* dood, RigidBody* body, BillboardMaterial* blood_material) : Entity(gs), dood(dood), body(body), blood_material(blood_material) { }
				~BoneShootable() { }

				bool GetShot(Shot* shot, const Vec3& poi, const Vec3& vel, float mass);
			};

			bool use_cheaty_ori;
			
			float yaw_rate, pitch_rate;

			virtual void InnerDispose();

			void DoPitchAndYawControls(TimingInfo time);
			virtual void DoJumpControls(TimingInfo time, Vec3 forward, Vec3 rightward);
			virtual void DoMovementControls(TimingInfo time, Vec3 forward, Vec3 rightward);
			virtual void DoWeaponControls(TimingInfo time);

			// for PhysicsToCharacter
			virtual void PreUpdatePoses(TimingInfo time);
			virtual void PostUpdatePoses(TimingInfo time);

			// for PoseToPhysics
			virtual void DoCheatyPose(float timestep, const Vec3& net_vel);
			virtual void MaybeSinkCheatyVelocity(float timestep, Vec3& cheaty_vel, Vec3& cheaty_rot, float net_mass, const Mat3& net_moi);

			RigidBody* RigidBodyForNamedBone(const string& name);

		public:

			Team team;

			BillboardMaterial* blood_material;

			Vec3 pos;
			Vec3 vel;
			float yaw, pitch;

			float jump_start_timer;

			float hp;
			bool alive;

			float ragdoll_timer;

			Bone* eye_bone;

			UberModel* model;
			SkinnedCharacter* character;
			PosedCharacter* posey;

			float vis_bs_radius;		// radius of bounding sphere used for frustum culling

			RigidBody* root_rigid_body;
			vector<RigidBody*> rigid_bodies;
			vector<BoneShootable*> shootables;
			vector<Bone*> rbody_to_posey;
			vector<RigidBody*> bone_to_rbody;
			vector<PhysicsConstraint*> constraints;

			set<RigidBody*> velocity_change_bodies;		// rigid bodies which should be affected by StandingCallback::ApplyVelocityChange

			CollisionGroup* collision_group;

			PhysicsWorld* physics;
			ModelPhysics* mphys;

			WeaponEquip* equipped_weapon;
			WeaponIntrinsic* intrinsic_weapon;

			Dood(GameState* gs, UberModel* model, ModelPhysics* mphys, Vec3 pos, Team& team);

			Vec3 GetPosition();
			void SetPosition(Vec3 pos);

			void Vis(SceneRenderer* renderer);
			Mat4 GetViewMatrix();
			Vec3 GetEyePos();

			SoundSource* PlayDoodSound(SoundBuffer* buffer, float vol, bool looping);

			void PoseCharacter(TimingInfo time);

			void PhysicsToCharacter();
			virtual void PoseToPhysics(float timestep);

			virtual void Update(TimingInfo time);

			virtual void Spawned();
			virtual void DeSpawned();

			void TakeDamage(Damage damage, const Vec3& from_dir);
			void Splatter(Shot* shot, const Vec3& poi, const Vec3& vel);

			virtual void Die(Damage cause);

			bool GetAmmoFraction(float& result);
			bool GetAmmoCount(int& result);

			virtual void RegisterFeet();					// use this to create (custom?) FootState instances and add them to feet

			class FootState : public Disposable
			{
				public:

					unsigned int posey_id;

					RigidBody* body;
					PlacedFootConstraint* pfc;

					FootState(unsigned int posey_id) : posey_id(posey_id), body(NULL), pfc(NULL) { }

					bool IsStanding() { return pfc != NULL; }
			};
			vector<FootState*> feet;

			struct StandingCallback : public CollisionCallback
			{
				Dood* dood;

				float angular_coeff;

				StandingCallback();

				void OnCollision(const ContactPoint& collision);			// from CibraryEngine::CollisionCallback
				void MaybeCreateConstraint(FootState* foot, RigidBody* surface, const Vec3& use_pos, const Vec3& use_normal);

				void ApplyVelocityChange(const Vec3& dv);
				void OnPhysicsTick(float timestep);
				void BreakAllConstraints();

				bool IsStanding();

			} standing_callback;

			struct AmmoFailureEvent : public Event
			{
				Dood* dood;
				WeaponEquip* weapon;
				AmmoFailureEvent(Dood* dood, WeaponEquip* weapon) : dood(dood), weapon(weapon) { }
			};
			EventDispatcher OnAmmoFailure;

			struct DamageTakenEvent : public Event
			{
				Dood* dood;
				Vec3 from_dir;
				Damage damage;
				bool cancel;

				DamageTakenEvent(Dood* dood, const Vec3& from_dir, Damage damage) : dood(dood), from_dir(from_dir), damage(damage), cancel(false) { }
			};
			EventDispatcher OnDamageTaken;

			struct JumpFailureEvent : public Event
			{
				Dood* dood;
				JumpFailureEvent(Dood* dood) : dood(dood) { }
			};
			EventDispatcher OnJumpFailure;

			struct DeathEvent : public Event
			{
				Dood* dood;
				Damage cause;
				DeathEvent(Dood* dood, Damage cause) : dood(dood), cause(cause) { }
			};
			EventDispatcher OnDeath;
	};

	void PushDoodHandle(lua_State* L, Dood* dood);
}
