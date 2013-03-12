#include "StdAfx.h"
#include "ConvexMeshShape.h"

#include "MassInfo.h"
#include "AABB.h"
#include "Util.h"

#include "RayCollider.h"
#include "TriangleMeshShape.h"

#include "ContactPoint.h"

#include "DebugLog.h"
#include "Serialize.h"

#include "SceneRenderer.h"
#include "CameraView.h"
#include "RenderNode.h"
#include "VertexBuffer.h"
#include "DebugDrawMaterial.h"

namespace CibraryEngine
{
	using namespace std;

	/*
	 * ConvexMeshShape private implementation struct
	 */
	struct ConvexMeshShape::Imp
	{
		struct Vertex
		{
			Vec3 pos;

			Vertex(const Vec3& pos) : pos(pos) { }
		};

		struct Edge
		{
			unsigned int v1, v2;		// indices into verts array

			Edge(unsigned int v1, unsigned int v2) : v1(v1), v2(v2) { }
		};

		struct Face
		{
			Plane plane;
			vector<unsigned int> verts;
			vector<Plane> boundary_planes;

			Face(const Plane& plane) : plane(plane), verts(), boundary_planes() { }
		};

		struct ConvexPoly
		{
			struct PolyData
			{
				unsigned int index;
				Vec2 pos;

				Vec2 normal;
				float offset;

				PolyData *next, *prev;

			} data[32], *start, *unused;

			unsigned int max_count, count;

			Plane plane;
			Vec3 x_axis, y_axis;

			ConvexPoly(const Plane& plane) : start(NULL), unused(&data[0]), max_count(32), count(0), plane(plane)
			{
				SelectAxes(plane.normal, x_axis, y_axis);

				for(unsigned int i = 1; i < max_count; ++i)
					data[i - 1].next = &data[i];
				data[max_count - 1].next = NULL;
			}

			ConvexPoly(const Plane& plane, const Vec3& x_axis, const Vec3& y_axis) : start(NULL), unused(&data[0]), max_count(32), count(0), plane(plane), x_axis(x_axis), y_axis(y_axis)
			{
				for(unsigned int i = 1; i < max_count; ++i)
					data[i - 1].next = &data[i];
				data[max_count - 1].next = NULL;
			}

			void AddVert(const Vec3& vertex, unsigned int index)
			{
				assert(unused != NULL);

				if(PolyData* iter = start)
				{
					do
					{
						if(iter->index == index)
							return;
						iter = iter->next;

					} while(iter != start);
				}

				Vec3 in_plane = vertex - plane.normal * (Vec3::Dot(plane.normal, vertex) - plane.offset);

				PolyData* noob = unused;
				unused = unused->next;

				noob->index = index;

				Vec2& pos = noob->pos;
				pos.x = Vec3::Dot(in_plane, x_axis);
				pos.y = Vec3::Dot(in_plane, y_axis);

				switch(count)
				{
					case 0:

						start = noob->next = noob->prev = noob;

						++count;

						break;

					case 1:

						start->normal = Vec2::Normalize(start->pos.y - noob->pos.y, noob->pos.x - start->pos.x);
						start->offset = Vec2::Dot(start->normal, start->pos);

						noob->normal = -start->normal;
						noob->offset = -start->offset;

						start->next = start->prev = noob;
						noob->next = noob->prev = start;

						++count;

						break;

					default:
					{
						static const float offset_threshold = 0.000001f;
						static const float colinearity_threshold = 0.000001f;

						PolyData* iter = start;
						do
						{
							if(Vec2::Dot(iter->normal, noob->pos) > iter->offset + offset_threshold)
							{
								PolyData *first = iter, *last = iter;

								if(iter == start)
									while(Vec2::Dot(first->prev->normal, noob->pos) > first->prev->offset - offset_threshold)
										first = first->prev;

								while(Vec2::Dot(last->next->normal, noob->pos) > last->next->offset - offset_threshold)
									last = last->next;

								PolyData* temp = last->next;
								last->next = unused;
								unused = first->next;
								last = temp;

								first->next = last->prev = noob;
								noob->prev = first;
								noob->next = last;

								first->normal = Vec2::Normalize(first->pos.y - noob->pos.y, noob->pos.x - first->pos.x);
								first->offset = Vec2::Dot(first->normal, first->pos);

								noob->normal = Vec2::Normalize(noob->pos.y - last->pos.y, last->pos.x - noob->pos.x);
								noob->offset = Vec2::Dot(noob->normal, noob->pos);

								start = noob;

								count = 0;			// indicates the point was successfully added

								break;
							}

							iter = iter->next;
						} while(iter != start);

						if(count == 0)
						{
							// point was successfully added; recompute count and eliminate any unnecessary colinear verts along the way
							iter = start;
							do
							{
								if((iter->normal - iter->prev->normal).ComputeMagnitudeSquared() < colinearity_threshold && iter->offset - iter->prev->offset < colinearity_threshold)
								{
									iter->prev->next = iter->next;
									iter->next->prev = iter->prev;

									PolyData* temp = iter->next;
									if(iter == start)
										start = temp;

									iter->next = unused;
									unused = iter;

									iter = temp;
								}
								else
								{
									++count;
									iter = iter->next;
								}
							} while(iter != start);
						}
						else
						{
							noob->next = unused;
							unused = noob;
						}

						break;
					}
				}
			}
		};


		vector<Vertex> verts;
		vector<Edge> edges;
		vector<Face> faces;

		AABB aabb;


		Imp() : verts(), edges(), faces(), aabb() { }
		Imp(Vec3* verts, unsigned int count) { Init(verts, count); }
		~Imp() { }

		void Init(Vec3* in_verts, unsigned int count)
		{
			verts = vector<Vertex>();
			edges = vector<Edge>();
			faces = vector<Face>();

			if(count > 0)
			{
				static const float xprod_discard_threshold = 0.0000001f;
				static const float plane_discard_threshold = 0.0001f;
				static const float plane_merge_threshold = 0.00001f;

				// determine aabb
				for(unsigned int i = 0; i < count; ++i)
					if(i == 0)
						aabb = AABB(in_verts[i]);
					else
						aabb.Expand(in_verts[i]);

				// generate unique exterior planes
				vector<ConvexPoly*> planes;
				for(unsigned int i = 2; i < count; ++i)
					for(unsigned int j = 1; j < i; ++j)
						for(unsigned int k = 0; k < j; ++k)
						{
							Vec3 normal = Vec3::Cross(in_verts[j] - in_verts[i], in_verts[k] - in_verts[i]);

							float mag = normal.ComputeMagnitude();
							if(mag > xprod_discard_threshold)
							{
								normal /= mag;
								float offset = Vec3::Dot(normal, in_verts[i]);

								Plane test_planes[2] = { Plane(normal, offset), Plane(-normal, -offset) };
								for(unsigned int m = 0; m < 2; ++m)
								{
									Plane& plane = test_planes[m];

									bool ok = true;
									for(unsigned int n = 0; n < count; ++n)
										if(plane.PointDistance(in_verts[n]) > plane_discard_threshold)
										{
											ok = false;
											break;
										}

									if(ok)
									{								
										for(vector<ConvexPoly*>::iterator iter = planes.begin(); iter != planes.end(); ++iter)
										{
											ConvexPoly* poly = *iter;
											if((plane.normal - poly->plane.normal).ComputeMagnitudeSquared() < plane_merge_threshold && fabs(plane.offset - poly->plane.offset) < plane_merge_threshold)
											{
												poly->AddVert(in_verts[i], i);
												poly->AddVert(in_verts[j], j);
												poly->AddVert(in_verts[k], k);

												ok = false;
												break;
											}
										}

										if(ok)
										{
											ConvexPoly* poly = new ConvexPoly(plane);
											poly->AddVert(in_verts[i], i);
											poly->AddVert(in_verts[j], j);
											poly->AddVert(in_verts[k], k);

											planes.push_back(poly);
										}
									}
								}
							}
						}

				// produce final vert, edge, and face data
				vector<unsigned int> index_map(count);
				memset(index_map.data(), 0, count * sizeof(unsigned int));

				for(vector<ConvexPoly*>::iterator iter = planes.begin(); iter != planes.end(); ++iter)
				{
					ConvexPoly* poly = *iter;
					if(poly->count <= 2)
					{
						DEBUG();

						delete poly;
						continue;
					}

					Face face(poly->plane);

					ConvexPoly::PolyData *start = poly->start, *jter = start;
					do
					{
						assert(jter >= poly->data && jter < poly->data + sizeof(poly->data) / sizeof(ConvexPoly::PolyData));

						// create unique vertices
						unsigned int index = jter->index, mapped_index = index_map[index];
						if(mapped_index == 0)
						{
							mapped_index = index_map[index] = verts.size() + 1;
							verts.push_back(in_verts[index]);
						}
						--mapped_index;

						unsigned int next = jter->next->index, mapped_next = index_map[next];
						if(mapped_next == 0)
						{
							mapped_next = index_map[next] = verts.size() + 1;
							verts.push_back(in_verts[next]);
						}
						--mapped_next;

						// create unique edges
						bool ok = true;
						for(vector<Edge>::iterator kter = edges.begin(); kter != edges.end(); ++kter)
							if(kter->v1 == mapped_index && kter->v2 == mapped_next || kter->v1 == mapped_next && kter->v2 == mapped_index)
							{
								ok = false;
								break;
							}

						if(ok)
							edges.push_back(Edge(mapped_index, mapped_next));

						face.verts.push_back(mapped_index);
						face.boundary_planes.push_back(Plane::FromPositionNormal(in_verts[index], Vec3::Cross(poly->plane.normal, jter->normal.x * poly->x_axis + jter->normal.y * poly->y_axis)));

						jter = jter->next;

					} while(jter != start);

					faces.push_back(face);

					delete poly;
				}

				// OutputParts();
			}
			else
				aabb = AABB();
		}

		void OutputParts()
		{
			Debug(((stringstream&)(stringstream() << "Shape contains " << verts.size() << " verts, " << edges.size() << " edges, and " << faces.size() << " faces" << endl)).str());

			Debug("Verts:\n");
			for(unsigned int i = 0; i < verts.size(); ++i)
			{
				Vec3& pos = verts[i].pos;
				Debug(((stringstream&)(stringstream() << "\tVertex " << i << " =\t(" << pos.x << ", " << pos.y << ", " << pos.z << ")" << endl)).str());
			}

			Debug("Edges:\n");
			for(unsigned int i = 0; i < edges.size(); ++i)
			{
				unsigned int v1 = edges[i].v1, v2 = edges[i].v2;
				const Vec3& a = verts[v1].pos;
				const Vec3& b = verts[v2].pos;

				Debug(((stringstream&)(stringstream() << "\tEdge " << i << " is between vertex " << v1 << " and vertex " << v2 << endl)).str());
				Debug(((stringstream&)(stringstream() << "\t\tVertex " << v1 << " =\t(" << a.x << ", " << a.y << ", " << a.z << ")" << endl)).str());
				Debug(((stringstream&)(stringstream() << "\t\tVertex " << v2 << " =\t(" << b.x << ", " << b.y << ", " << b.z << ")" << endl)).str());
			}

			
			Debug("Faces:\n");
			for(unsigned int i = 0; i < faces.size(); ++i)
			{
				Face& face = faces[i];
				Vec3& normal = face.plane.normal;

				Debug(((stringstream&)(stringstream() << "\tFace " << i << " has normal vector (" << normal.x << ", " << normal.y << ", " << normal.z << ") and offset = " << face.plane.offset << "; it contains " << face.verts.size() << " vertices:" << endl)).str());
				for(unsigned int j = 0; j < face.verts.size(); ++j)
				{
					unsigned int index = face.verts[j];
					const Vec3& vert = verts[index].pos;
					Debug(((stringstream&)(stringstream() << "\t\tVertex " << index << " =\t(" << vert.x << ", " << vert.y << ", " << vert.z << ")" << endl)).str());
				}
			}
		}



		// misc. utility stuff
		void DebugDraw(SceneRenderer* renderer, const Vec3& pos, const Quaternion& ori)
		{
			Mat4 xform = Mat4::FromPositionAndOrientation(pos, ori);

			Vec3 center = xform.TransformVec3_1(aabb.GetCenterPoint());
			if(renderer->camera->CheckSphereVisibility(center, (aabb.max - aabb.min).ComputeMagnitude() * 0.5f))
			{
				DebugDrawMaterial* ddm = DebugDrawMaterial::GetDebugDrawMaterial();

				for(vector<Edge>::iterator iter = edges.begin(); iter != edges.end(); ++iter)
					renderer->objects.push_back(RenderNode(ddm, ddm->New(xform.TransformVec3_1(verts[iter->v1].pos), xform.TransformVec3_1(verts[iter->v2].pos)), 1.0f));
			}
		}

		AABB GetTransformedAABB(const Mat4& xform)
		{
			// this will produce a tighter fitting AABB than aabb.GetTransformedAABB(xform), but it may be slower (especially if there are more than 8 vertices!)
			vector<Vertex>::const_iterator iter = verts.begin(), verts_end = verts.end();

			AABB xformed_aabb(iter->pos);
			for(++iter; iter != verts_end; ++iter)
				xformed_aabb.Expand(xform.TransformVec3_1(iter->pos));

			return xformed_aabb;
		}

		AABB ComputeCachedWorldAABB(const Mat4& xform, ShapeInstanceCache*& cache)
		{
			ConvexMeshShapeInstanceCache* cmsic = (ConvexMeshShapeInstanceCache*)cache;
			if(!cmsic)
				cache = cmsic = new ConvexMeshShapeInstanceCache();

			cmsic->Update(xform, this);

			return cmsic->aabb;
		}

		MassInfo ComputeMassInfo()
		{
			if(aabb.IsDegenerate())
				return MassInfo();
			else
			{
				// approximate the ConvexMeshShape as a box (same as how Bullet does it for its MultiSphereShape type)
				MassInfo temp;

				Vec3 dim = aabb.max - aabb.min;

				temp.mass = dim.x * dim.y * dim.z;							// assumes density = 1
				temp.com = (aabb.min + aabb.max) * 0.5f;

				float coeff = temp.mass / 12.0f;
				temp.moi[0] = coeff * (dim.y * dim.y + dim.z * dim.z);
				temp.moi[4] = coeff * (dim.x * dim.x + dim.z * dim.z);
				temp.moi[8] = coeff * (dim.x * dim.x + dim.y * dim.y);

				return temp;
			}
		}

		static void SelectAxes(const Vec3& plane_normal, Vec3& x_axis, Vec3& y_axis)
		{
			float x = fabs(plane_normal.x), y = fabs(plane_normal.y), z = fabs(plane_normal.z);
			if(x <= y && x <= z)
				x_axis = Vec3::Normalize(Vec3::Cross(Vec3(1, 0, 0), plane_normal));
			else if(y <= x && y <= z)
				x_axis = Vec3::Normalize(Vec3::Cross(Vec3(0, 1, 0), plane_normal));
			else
				x_axis = Vec3::Normalize(Vec3::Cross(Vec3(0, 0, 1), plane_normal));
			y_axis = Vec3::Cross(plane_normal, x_axis);
		}



		// convex mesh collision functions
		bool CollideRay(const Ray& ray, RayResult& result, RayCollider* collider, RigidBody* body)
		{
			Vec3 a = ray.origin, b = ray.origin + ray.direction;
			AABB ray_aabb(Vec3(min(a.x, b.x), min(a.y, b.y), min(a.z, b.z)), Vec3(max(a.x, b.x), max(a.y, b.y), max(a.z, b.z)));

			if(!AABB::IntersectTest(ray_aabb, aabb))
				return false;


			bool any = false;

			for(vector<Face>::const_iterator iter = faces.begin(), faces_end = faces.end(); iter != faces_end; ++iter)
			{
				const Plane& plane = iter->plane;
				const Vec3& normal = plane.normal;

				float dir_dot = Vec3::Dot(ray.direction, normal);
				if(dir_dot != 0.0f)
				{
					float origin_dot = Vec3::Dot(ray.origin, normal);
					float tti = (plane.offset - origin_dot) / dir_dot;
					if(!any || tti < result.t)
					{
						Vec3 pos = ray.origin + ray.direction * tti;

						bool ok = true;
						for(vector<Plane>::const_iterator jter = iter->boundary_planes.begin(), planes_end = iter->boundary_planes.end(); jter != planes_end; ++jter)
							if(Vec3::Dot(jter->normal, pos) > jter->offset)
							{
								ok = false;
								break;
							}

						if(ok)
						{
							result.t = tti;
							result.pos = pos;
							result.norm = normal;

							any = true;
						}
					}
				}
			}

			if(any)
			{
				result.collider = collider;
				result.body = body;

				return true;
			}
			else
				return false;

			return false;
		}

		bool CollidePlane(ConvexMeshShapeInstanceCache* my_cache, const Plane& plane, ContactPointAllocator* alloc, vector<ContactPoint*>& results, RigidBody* ibody, RigidBody* jbody)
		{
			const Vec3& normal = plane.normal;
			float offset = plane.offset;

			for(vector<Vec3>::iterator iter = my_cache->verts.begin(), verts_end = my_cache->verts.end(); iter != verts_end; ++iter)
			{
				float dot = Vec3::Dot(normal, *iter);
				if(offset > dot)
				{
					ContactPoint* result = alloc->New(ibody, jbody);
					result->pos = *iter - normal * (dot - offset);
					result->normal = -normal;

					results.push_back(result);
				}
			}

			return !results.empty();
		}

		bool CollideTri(ConvexMeshShapeInstanceCache* my_cache, const TriangleMeshShape::TriCache& tri, ContactPointAllocator* alloc, vector<ContactPoint*>& results, RigidBody* ibody, RigidBody* jbody)
		{
			struct Scorer
			{
				const Vec3 *begin, *end;
				const TriangleMeshShape::TriCache& tri;

				bool first;

				float least;
				Vec3 direction;

				Scorer(const vector<Vec3>& a, const TriangleMeshShape::TriCache& tri) : begin(a.data()), end(begin + a.size()), tri(tri), first(true) { }
				
				bool Score(const Vec3& dir)
				{
					// get max extent of convex mesh shape
					const Vec3* iter = begin;

					float max_val = Vec3::Dot(dir, *iter);
					++iter;

					while(iter != end)
					{
						max_val = max(max_val, Vec3::Dot(dir, *iter));
						++iter;
					}

					// get min extent of triangle
					float min_val = min(Vec3::Dot(dir, tri.a), min(Vec3::Dot(dir, tri.b), Vec3::Dot(dir, tri.c)));

					// do stuff with the results
					float value = max_val - min_val;

					if(first)
					{
						least = value;
						direction = dir;
						first = false;
					}
					else if(value < least)
					{
						least = value;
						direction = dir;
					}

					return value <= 0;
				}
			} scorer(my_cache->verts, tri);

			// triangle's planes
			if(scorer.Score(-tri.plane.normal))			{ return false; }
			if(scorer.Score(tri.plane.normal))			{ return false; }

			// my faces...
			for(vector<Vec3>::const_iterator iter = my_cache->face_normals.begin(), normals_end = my_cache->face_normals.end(); iter != normals_end; ++iter)
				if(scorer.Score(*iter))					{ return false; }

			// my verts...
			for(vector<Vec3>::const_iterator iter = my_cache->verts.begin(), verts_end = my_cache->verts.end(); iter != verts_end; ++iter)
			{
				const Vec3& s = *iter;

				Vec3 sa = tri.a - s;
				Vec3 sb = tri.b - s;
				Vec3 sc = tri.c - s;

				// ... vs. triangle's verts
				if(scorer.Score(Vec3::Normalize(sa)))	{ return false; }
				if(scorer.Score(Vec3::Normalize(sb)))	{ return false; }
				if(scorer.Score(Vec3::Normalize(sc)))	{ return false; }

				// ... vs. triangle's edges
				Vec3 absnabn = Vec3::Normalize(Vec3::Cross(tri.ab, Vec3::Cross(sa, tri.ab)));
				if(scorer.Score(absnabn))				{ return false; }

				Vec3 bcsnbcn = Vec3::Normalize(Vec3::Cross(tri.bc, Vec3::Cross(sb, tri.bc)));
				if(scorer.Score(bcsnbcn))				{ return false; }

				Vec3 casncan = Vec3::Normalize(Vec3::Cross(tri.ac, Vec3::Cross(sc, tri.ac)));		// would be -ac, but there are two of them
				if(scorer.Score(casncan))				{ return false; }
			}


			// if we get this far, it means the objects are intersecting
			Vec3 tri_points[3] = { tri.a, tri.b, tri.c };
			return GenerateContactPoints(my_cache->verts.data(), my_cache->verts.size(), tri_points, 3, -tri.plane.normal, alloc, results, ibody, jbody);
		}

		bool CollideConvexMesh(ConvexMeshShapeInstanceCache* ishape, ConvexMeshShapeInstanceCache* jshape, ContactPointAllocator* alloc, vector<ContactPoint*>& contact_points, RigidBody* ibody, RigidBody* jbody)
		{
			struct Scorer
			{
				const Vec3 *ibegin, *iend;
				const Vec3 *jbegin, *jend;

				bool first;

				float least;
				Vec3 direction;

				Scorer(const vector<Vec3>& a, const vector<Vec3>& b) : ibegin(a.data()), iend(ibegin + a.size()), jbegin(b.data()), jend(jbegin + b.size()), first(true) { }

				bool Score(const Vec3& dir)
				{
					// get max extent of first convex mesh shape
					const Vec3* iter = ibegin;

					float max_val = Vec3::Dot(dir, *iter);
					++iter;

					while(iter != iend)
					{
						max_val = max(max_val, Vec3::Dot(dir, *iter));
						++iter;
					}

					// get min extent of second convex mesh shape
					const Vec3* jter = jbegin;

					float min_val = Vec3::Dot(dir, *jter);
					++jter;

					while(jter != jend)
					{
						min_val = min(min_val, Vec3::Dot(dir, *jter));
						++jter;
					}

					// do stuff with the results
					float value = max_val - min_val;

					if(first)
					{
						least = value;
						direction = dir;
						first = false;
					}
					else if(value < least)
					{
						least = value;
						direction = dir;
					}

					return value <= 0;
				}
			} scorer(ishape->verts, jshape->verts);
			
			// my faces...
			for(vector<Vec3>::const_iterator iter = ishape->face_normals.begin(), normals_end = ishape->face_normals.end(); iter != normals_end; ++iter)
				if(scorer.Score(*iter))				{ return false; }

			// other faces...
			for(vector<Vec3>::const_iterator iter = jshape->face_normals.begin(), normals_end = jshape->face_normals.end(); iter != normals_end; ++iter)
				if(scorer.Score(-*iter))			{ return false; }

			// verts vs. verts...
			for(vector<Vec3>::const_iterator iter = ishape->verts.begin(), iverts_end = ishape->verts.end(); iter != iverts_end; ++iter)
				for(vector<Vec3>::const_iterator jter = jshape->verts.begin(), jverts_end = jshape->verts.end(); jter != jverts_end; ++jter)
					if(scorer.Score(*jter - *iter))	{ return false; }

			// my edges...
			for(vector<Vec3>::iterator along = ishape->edges_normalized.begin(), along_end = ishape->edges_normalized.end(), point = ishape->edge_points.begin(); along != along_end; ++along, ++point)
				for(vector<Vec3>::const_iterator iter = jshape->verts.begin(), verts_end = jshape->verts.end(); iter != verts_end; ++iter)
				{
					Vec3 to_vert = *iter - *point;
					if(scorer.Score(to_vert - *along * Vec3::Dot(*along, to_vert)))	{ return false; }
				}

			// other edges...
			for(vector<Vec3>::const_iterator along = jshape->edges_normalized.begin(), along_end = jshape->edges_normalized.end(), point = jshape->edge_points.begin(); along != along_end; ++along, ++point)
				for(vector<Vec3>::const_iterator iter = ishape->verts.begin(), verts_end = ishape->verts.end(); iter != verts_end; ++iter)
				{
					Vec3 to_vert = *point - *iter;
					if(scorer.Score(to_vert - *along * Vec3::Dot(*along, to_vert)))	{ return false; }
				}


			// if we get this far, it means the objects are intersecting
			return GenerateContactPoints(ishape->verts.data(), ishape->verts.size(), jshape->verts.data(), jshape->verts.size(), scorer.direction, alloc, contact_points, ibody, jbody);
		}

		static bool GenerateContactPoints(Vec3* my_points, unsigned int my_count, Vec3* other_points, unsigned int other_count, const Vec3& direction, ContactPointAllocator* alloc, vector<ContactPoint*>& contact_points, RigidBody* ibody, RigidBody* jbody)
		{
			// find extents of the overlap region
			float farthest_extent = Vec3::Dot(direction, my_points[0]);
			for(unsigned int i = 1; i < my_count; ++i)
				farthest_extent = max(farthest_extent, Vec3::Dot(direction, my_points[i]));

			float nearest_extent = Vec3::Dot(direction, other_points[0]);
			for(unsigned int i = 1; i < other_count; ++i)
				nearest_extent = min(nearest_extent, Vec3::Dot(direction, other_points[i]));

			// collect lists of which verts from each object extend into the overlap region
			vector<Vec3*> my_verts;
			for(unsigned int i = 0; i < my_count; ++i)
				if(Vec3::Dot(direction, my_points[i]) >= nearest_extent)
					my_verts.push_back(&my_points[i]);

			vector<Vec3*> other_verts;
			for(unsigned int i = 0; i < other_count; ++i)
				if(Vec3::Dot(direction, other_points[i]) <= farthest_extent)
					other_verts.push_back(&other_points[i]);

			if(my_verts.empty() || other_verts.empty())			// lolwut? silly float math
				return false;

			// now generate contact points based on those verts
			float contact_plane_offset = (nearest_extent + farthest_extent) * 0.5f;

			switch(my_verts.size())
			{
				case 1:
				{
					switch(other_verts.size())
					{
						case 1:			// point-point
						{
							ContactPoint* p = alloc->New(ibody, jbody);
							p->normal = direction;
							
							Vec3 pos = (*my_verts[0] + *other_verts[0]) * 0.5f;
							p->pos = pos - direction * (Vec3::Dot(direction, pos) - contact_plane_offset);

							contact_points.push_back(p);

							return true;
						}

						default:		// point-edge and point-polygon
						{
							ContactPoint* p = alloc->New(ibody, jbody);
							p->normal = direction;

							const Vec3& pos = *my_verts[0];
							p->pos = pos - direction * (Vec3::Dot(direction, pos) - contact_plane_offset);

							contact_points.push_back(p);

							return true;
						}
					}
				}

				case 2:
				{
					switch(other_verts.size())
					{
						case 1:			// edge-point
						{
							ContactPoint* p = alloc->New(ibody, jbody);
							p->normal = direction;
							
							const Vec3& pos = *other_verts[0];
							p->pos = pos - direction * (Vec3::Dot(direction, pos) - contact_plane_offset);

							contact_points.push_back(p);

							return true;
						}

						case 2:			// edge-edge
						{
							Vec3 x_axis, y_axis;
							SelectAxes(direction, x_axis, y_axis);

							const Vec3& a1(*my_verts	[0]);
							const Vec3& a2(*my_verts	[1]);
							const Vec3& b1(*other_verts	[0]);
							const Vec3& b2(*other_verts	[1]);

							Vec2 result;
							if(DoEdgeEdge(result,
									Vec2(Vec3::Dot(x_axis, a1), Vec3::Dot(y_axis, a1)),
									Vec2(Vec3::Dot(x_axis, a2), Vec3::Dot(y_axis, a2)),
									Vec2(Vec3::Dot(x_axis, b1), Vec3::Dot(y_axis, b1)),
									Vec2(Vec3::Dot(x_axis, b2), Vec3::Dot(y_axis, b2))))
							{
								ContactPoint* p = alloc->New(ibody, jbody);
								p->normal = direction;
								p->pos = direction * contact_plane_offset + x_axis * result.x + y_axis * result.y;
								contact_points.push_back(p);

								return true;
							}

							return false;
						}

						default:		// edge-polygon
							return DoEdgePolygon(my_verts, other_verts, direction, contact_plane_offset, alloc, contact_points, ibody, jbody);
					}
				}

				default:
				{
					switch(other_verts.size())
					{
						case 1:			// polygon-point
						{
							ContactPoint* p = alloc->New(ibody, jbody);
							p->normal = direction;
							
							const Vec3& pos = *other_verts[0];
							p->pos = pos - direction * (Vec3::Dot(direction, pos) - contact_plane_offset);

							contact_points.push_back(p);

							return true;
						}

						case 2:			// polygon-edge
							return DoEdgePolygon(other_verts, my_verts, direction, contact_plane_offset, alloc, contact_points, jbody, ibody);

						default:		// polygon-polygon
						{
							Vec3 x_axis, y_axis;
							SelectAxes(direction, x_axis, y_axis);

							Plane plane(direction, contact_plane_offset);

							ConvexPoly my_poly(plane, x_axis, y_axis);
							for(unsigned int i = 0, count = my_verts.size(); i < count; ++i)
								my_poly.AddVert(*my_verts[i], i);

							ConvexPoly other_poly(plane, x_axis, y_axis);
							for(unsigned int i = 0, count = other_verts.size(); i < count; ++i)
								other_poly.AddVert(*other_verts[i], i);

							bool any = false;

							Vec2 intersection;
							Vec3 origin = direction * contact_plane_offset;
							ConvexPoly::PolyData* iter = my_poly.start;
							do
							{
								ConvexPoly::PolyData* jter = other_poly.start;

								const Vec2& point = iter->pos;
								bool point_ok = true;

								// add intersection points
								do
								{
									jter = jter->next;

									if(DoEdgeEdge(intersection, point, iter->next->pos, jter->pos, jter->next->pos))
									{
										ContactPoint* p = alloc->New(ibody, jbody);
										p->normal = direction;
										p->pos = origin + x_axis * intersection.x + y_axis * intersection.y;
										contact_points.push_back(p);

										any = true;
									}

									if(Vec2::Dot(iter->normal, point) > iter->offset)
										point_ok = false;

								} while(jter != other_poly.start);

								// add points from my list which were inside both objects
								if(point_ok)
								{
									ContactPoint* p = alloc->New(ibody, jbody);
									p->normal = direction;
									p->pos = origin + x_axis * point.x + y_axis * point.y;
									contact_points.push_back(p);

									any = true;
								}

								iter = iter->next;
							} while(iter != my_poly.start);

							// add points from other list which are inside both objects
							iter = other_poly.start;
							do
							{
								const Vec2& point = iter->pos;
								bool point_ok = true;

								ConvexPoly::PolyData* jter = my_poly.start;
								do
								{
									if(Vec2::Dot(jter->normal, point) >= jter->offset)
									{
										point_ok = false;
										break;
									}

									jter = jter->next;
								} while(jter != my_poly.start);
								
								if(point_ok)
								{
									ContactPoint* p = alloc->New(ibody, jbody);
									p->normal = direction;
									p->pos = origin + x_axis * point.x + y_axis * point.y;
									contact_points.push_back(p);

									any = true;
								}

								iter = iter->next;
							} while(iter != other_poly.start);

							return any;
						}
					}
				}
			}
		}

		static bool DoEdgePolygon(const vector<Vec3*>& edge, const vector<Vec3*>& polygon, const Vec3& direction, float contact_plane_offset, ContactPointAllocator* alloc, vector<ContactPoint*>& contact_points, RigidBody* ibody, RigidBody* jbody)
		{
			ConvexPoly poly(Plane(direction, contact_plane_offset));

			for(unsigned int i = 0, count = polygon.size(); i < count; ++i)
				poly.AddVert(*polygon[i], i);

			const Vec3& x_axis = poly.x_axis;
			const Vec3& y_axis = poly.y_axis;

			const Vec3& v1(*edge[0]);
			const Vec3& v2(*edge[1]);

			Vec2 v1p(Vec3::Dot(x_axis, v1), Vec3::Dot(y_axis, v1));
			Vec2 v2p(Vec3::Dot(x_axis, v2), Vec3::Dot(y_axis, v2));

			bool any = false;

			Vec2 intersection;
			ConvexPoly::PolyData* iter = poly.start;
			do
			{
				if(DoEdgeEdge(intersection, v1p, v2p, iter->pos, iter->next->pos))
				{
					ContactPoint* p = alloc->New(ibody, jbody);
					p->normal = direction;
					p->pos = direction * contact_plane_offset + x_axis * intersection.x + y_axis * intersection.y;
					contact_points.push_back(p);

					any = true;
				}

				iter = iter->next;
			} while(iter != poly.start);

			return any;
		}

		static bool DoEdgeEdge(Vec2& result, const Vec2& a1, const Vec2& a2, const Vec2& b1, const Vec2& b2)
		{
			Vec2 a_dx(a2 - a1);
			Vec2 b_dx(b2 - b1);

			Vec2 cross_a(-a_dx.y, a_dx.x);
			Vec2 cross_b(-b_dx.y, b_dx.x);

			float a_pos = Vec2::Dot(a1, cross_a), b1_a = Vec2::Dot(b1, cross_a), b2_a = Vec2::Dot(b2, cross_a);
			float b_pos = Vec2::Dot(b1, cross_b), a1_b = Vec2::Dot(a1, cross_b), a2_b = Vec2::Dot(a2, cross_b);
							
			if((b1_a - a_pos) * (b2_a - a_pos) <= 0.0f && (a1_b - b_pos) * (a2_b - b_pos) <= 0.0f)
			{
				float dif = a2_b - a1_b;
				if(dif != 0)
				{
					result = a1 + a_dx * ((b_pos - a1_b) / dif);
					return true;
				}
			}

			return false;
		}



		// i/o functions
		void Write(ostream& stream)
		{
			WriteUInt32(verts.size(), stream);
			for(vector<Vertex>::const_iterator iter = verts.begin(), verts_end = verts.end(); iter != verts_end; ++iter)
				WriteVec3(iter->pos, stream);
		}

		unsigned int Read(istream& stream)
		{
			if(unsigned int count = ReadUInt32(stream))
			{
				Vec3* verts = new Vec3[count];
				for(unsigned int i = 0; i < count; ++i)
					verts[i] = ReadVec3(stream);

				Init(verts, count);

				delete[] verts;
			}

			return stream.fail() ? 1 : 0;
		}
	};




	/*
	 * ConvexMeshShape methods
	 */
	ConvexMeshShape::ConvexMeshShape() : CollisionShape(ST_ConvexMesh), imp(new Imp())												{ }
	ConvexMeshShape::ConvexMeshShape(Vec3* verts, unsigned int count) : CollisionShape(ST_ConvexMesh), imp(new Imp(verts, count))	{ }

	void ConvexMeshShape::InnerDispose()																{ if(imp) { delete imp; imp = NULL; } }

	MassInfo ConvexMeshShape::ComputeMassInfo()															{ return imp->ComputeMassInfo(); }

	AABB ConvexMeshShape::GetTransformedAABB(const Mat4& xform)											{ return imp->GetTransformedAABB(xform); }
	AABB ConvexMeshShape::ComputeCachedWorldAABB(const Mat4& xform, ShapeInstanceCache*& cache)			{ return imp->ComputeCachedWorldAABB(xform, cache); }

	void ConvexMeshShape::DebugDraw(SceneRenderer* renderer, const Vec3& pos, const Quaternion& ori)	{ imp->DebugDraw(renderer, pos, ori); }

	void ConvexMeshShape::Write(ostream& stream)														{ imp->Write(stream); }
	unsigned int ConvexMeshShape::Read(istream& stream)													{ return imp->Read(stream); }

	bool ConvexMeshShape::CollideRay(const Ray& ray, RayResult& result, RayCollider* collider, RigidBody* body)																														{ return imp->CollideRay(ray, result, collider, body); }
	bool ConvexMeshShape::CollidePlane(ConvexMeshShapeInstanceCache* my_cache, const Plane& plane, ContactPointAllocator* alloc, vector<ContactPoint*>& results, RigidBody* ibody, RigidBody* jbody)								{ return imp->CollidePlane(my_cache, plane, alloc, results, ibody, jbody); }
	bool ConvexMeshShape::CollideTri(ConvexMeshShapeInstanceCache* my_cache, const TriangleMeshShape::TriCache& tri, ContactPointAllocator* alloc, vector<ContactPoint*>& results, RigidBody* ibody, RigidBody* jbody)				{ return imp->CollideTri(my_cache, tri, alloc, results, ibody, jbody); }
	bool ConvexMeshShape::CollideConvexMesh(ConvexMeshShapeInstanceCache* ishape, ConvexMeshShapeInstanceCache* jshape, ContactPointAllocator* alloc, vector<ContactPoint*>& contact_points, RigidBody* ibody, RigidBody* jbody)	{ return imp->CollideConvexMesh(ishape, jshape, alloc, contact_points, ibody, jbody); }




	/*
	 *  ConvexMeshShape conversion functions
	 */
	ConvexMeshShape* ConvexMeshShape::FromVBO(VertexBuffer* vbo)
	{
		// extract vec3s from vbo data
		unsigned int num_verts = vbo->GetNumVerts();
		float* vert_ptr = vbo->GetFloatPointer("gl_Vertex");

		Vec3* verts = new Vec3[num_verts];
		if(vbo->GetAttribNPerVertex("gl_Vertex") == 4)
		{
			for(unsigned int i = 0; i < num_verts; ++i, vert_ptr += 2)
			{
				Vec3& vert = verts[i];
				vert.x = *(vert_ptr++);
				vert.y = *(vert_ptr++);
				vert.z = *vert_ptr;
			}
		}
		else
		{
			for(unsigned int i = 0; i < num_verts; ++i)
			{
				Vec3& vert = verts[i];
				vert.x = *(vert_ptr++);
				vert.y = *(vert_ptr++);
				vert.z = *(vert_ptr++);
			}
		}

		// eliminate duplicate verts
		unsigned int unique_count = 0;
		for(unsigned int i = 0; i < num_verts; ++i)
		{
			bool ok = true;
			for(unsigned int j = 0; j < unique_count; ++j)
				if((verts[i] - verts[j]).ComputeMagnitudeSquared() == 0.0f)
				{
					ok = false;
					break;
				}

			if(ok)
				verts[unique_count++] = verts[i];
		}

		ConvexMeshShape* result = new ConvexMeshShape();
		result->imp->Init(verts, unique_count);

		delete[] verts;

		return result;
	}




	/*
	 * ConvexMeshShapeInstanceCache methods
	 */
	ConvexMeshShapeInstanceCache::ConvexMeshShapeInstanceCache() : verts(), edges_normalized(), edge_points(), face_normals(), aabb() { }

	void ConvexMeshShapeInstanceCache::Update(const Mat4& xform, ConvexMeshShape::Imp* imp)
	{
		verts.clear();
		edges_normalized.clear();
		edge_points.clear();
		face_normals.clear();

		vector<ConvexMeshShape::Imp::Vertex>::iterator vert_iter = imp->verts.begin(), verts_end = imp->verts.end();
		Vec3 xformed_vert = xform.TransformVec3_1(vert_iter->pos);
		aabb = AABB(xformed_vert);
		verts.push_back(xformed_vert);

		for(++vert_iter; vert_iter != verts_end; ++vert_iter)
		{
			xformed_vert = xform.TransformVec3_1(vert_iter->pos);

			aabb.Expand(xformed_vert);
			verts.push_back(xformed_vert);
		}

		for(vector<ConvexMeshShape::Imp::Edge>::iterator iter = imp->edges.begin(), edges_end = imp->edges.end(); iter != edges_end; ++iter)
		{
			edges_normalized.push_back(Vec3::Normalize(verts[iter->v2] - verts[iter->v1]));
			edge_points.push_back(verts[iter->v2]);
		}

		for(vector<ConvexMeshShape::Imp::Face>::iterator iter = imp->faces.begin(), faces_end = imp->faces.end(); iter != faces_end; ++iter)
			face_normals.push_back(xform.TransformVec3_0(iter->plane.normal));
	}

	void ConvexMeshShapeInstanceCache::Update(const Mat4& xform, ConvexMeshShape* shape) { Update(xform, shape->imp); }
}
