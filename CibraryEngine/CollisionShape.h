#pragma once

#include "StdAfx.h"
#include "Disposable.h"

#include "Vector.h"
#include "Plane.h"

#include "ContentTypeHandler.h"

namespace CibraryEngine
{
	using namespace std;

	struct Quaternion;

	struct MassInfo;
	class RigidBody;

	struct VertexBuffer;
	class SceneRenderer;
	class Material;

	enum ShapeType
	{
		ST_Ray = 1,
		ST_Sphere = 2,
		ST_TriangleMesh = 3,
		ST_InfinitePlane = 4,
		ST_MultiSphere = 5,

		ST_ShapeTypeMax
	};

	/** A shape usable for collision detection and/or response */
	class CollisionShape : public Disposable
	{
		private:

			ShapeType type;

		protected:

			CollisionShape(ShapeType type);

		public:

			/** Compute the mass info for this shape, assuming a density of 1 */
			virtual MassInfo ComputeMassInfo();

			ShapeType GetShapeType();

			bool CanMove();

			virtual void DebugDraw(SceneRenderer* renderer, const Vec3& pos, const Quaternion& ori);

			/** Reads a collision shape of the appropriate type from an input stream */
			virtual unsigned int Read(istream& stream);
			/** Write a collision shape to an output stream */
			virtual void Write(ostream& stream);

			static unsigned int ReadCollisionShape(CollisionShape*& shape, istream& stream);
			static unsigned int WriteCollisionShape(CollisionShape* shape, ostream& stream);
	};
}
