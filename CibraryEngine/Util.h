#pragma once

#include "Vector.h"

namespace CibraryEngine
{
	struct Mat3;
	struct Plane;

	using namespace std;

	struct Ray
	{
		Vec3 origin;
		Vec3 direction;
	};

	struct Intersection
	{
		float time;
		Vec3 position;

		int face;
		Vec3 normal;
		Ray ray;

		float i, j;				// triangle-space coordinates, (0,0) = vertex A, (1,0) = vertex B, (0, 1) = vertex C
	};

	/** Class providing a few utility functions */
	class Util
	{
		private:
			// This is a class containing static utility methods only - you cannot instantiate it!
			Util() { }
			Util(Util& other) { }
			void operator =(Util& other) { }

		public:

			/** Given the position and velocity of a target, and the muzzle speed of a projectile, returns how many seconds ahead of the target the shooter should aim in order to hit */
			static float LeadTime(Vec3 dx, Vec3 dv, float muzzle_speed);

			/** Given a direction vector, finds an orientation matrix with that as its forward, and the other directions selected randomly */
			static Mat3 FindOrientationZEdge(Vec3 dir);

			/** Batch test a bunch of rays and triangles for intersection */
			static vector<vector<Intersection> > RayTriangleListIntersect(const vector<Vec3>& verts, const vector<unsigned int>& a_vert, const vector<unsigned int>& b_vert, const vector<unsigned int>& c_vert, const vector<Ray>& rays);

			/** Finds the minimum distance from the triangle formed by ABC to the point at X */
			static float TriangleMinimumDistance(const Vec3& a, const Vec3& b, const Vec3& c, const Vec3& x);

			static float RayPlaneIntersect(const Ray& ray, const Plane& plane);
	};
}
