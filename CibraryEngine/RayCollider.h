#include "StdAfx.h"

#include "DynamicsObject.h"

#include "Vector.h"
#include "AABB.h"

namespace CibraryEngine
{
	struct Ray;
	struct RayResult;
	class RayCallback;

	class RayCollider;
	class RigidBody;
	class CollisionGroup;
	
	using namespace std;

	struct RayResult
	{
		RayCollider* collider;
		RigidBody* body;

		Vec3 pos, norm;

		float t;

		RayResult() : collider(NULL), body(NULL) { }
		RayResult(RayCollider* collider, RigidBody* body, const Vec3& pos, const Vec3& norm, float t) : collider(collider), body(body), pos(pos), norm(norm), t(t) { }
		bool operator <(const RayResult& other) { return t < other.t; }
	};

	class RayCollider : public DynamicsObject
	{
		friend class PhysicsWorld;

		protected:

			RayCallback* ray_callback;

			static void CollideRigidBody(		RigidBody* body,		const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);
			static void CollideCollisionGroup(	CollisionGroup* group,	const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);

			static void CollideSphere(			RigidBody* body,		const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);
			static void CollideMesh(			RigidBody* body,		const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);
			static void CollidePlane(			RigidBody* body,		const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);
			static void CollideMultisphere(		RigidBody* body,		const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);
			static void CollideConvexMesh(		RigidBody* body,		const Ray& ray, float max_time, list<RayResult>& hits, RayCollider* collider = NULL);

		public:

			RayCollider(Entity* user_entity, const Vec3& pos, const Vec3& vel, float mass);

			AABB GetAABB(float timestep)               { AABB result(pos); result.Expand(pos + vel * timestep); return result; }

			void DoCollisionResponse(RayResult& collision);

			RayCallback* GetRayCallback() const        { return ray_callback; }
			void SetRayCallback(RayCallback* callback) { ray_callback = callback; }

			void DebugDraw(SceneRenderer* renderer) const;
	};

	class RayCallback
	{
		public:

			virtual bool OnCollision(RayResult& collision) = 0;
	};
}