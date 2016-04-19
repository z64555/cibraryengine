#pragma once

#include "StdAfx.h"

#include "Matrix.h"

namespace CibraryEngine
{
	using namespace std;

	class VoxelTerrain;
	struct CubeTriangles;
	struct TerrainNode;
	struct TerrainVertex;

	class VoxelMaterial;
	struct VoxelMaterialVBO;
	struct VoxelMaterialVBOBuilder;

	struct TerrainAction;

	struct VertexBuffer;
	class SceneRenderer;

	class TerrainChunk
	{
		private:

			vector<TerrainNode> node_data;

			int chunk_x, chunk_y, chunk_z;

			Mat4 xform;

			VoxelMaterial* material;

			struct CombinedVBO
			{
				TerrainChunk* owner;
				int chunk_x, chunk_y, chunk_z;
				int lod;
				int use_size, use_size_squared;
				float inv_use_size;

				vector<CubeTriangles> tri_data;

				unordered_map<unsigned char, VoxelMaterialVBO> vbos;
				VertexBuffer* depth_vbo;

				bool valid;

				CombinedVBO();
				CombinedVBO(TerrainChunk* chunk, int lod);

				void Vis(SceneRenderer* renderer, const Mat4& main_xform);
				void Invalidate();

				bool GetRelativePositionInfo(int x, int y, int z, TerrainChunk*& chunk, int& dx, int &dy, int& dz);						// internal utility function, but possibly useful for external code as well (thus, public)
				CubeTriangles* GetCube(int x, int y, int z);
				CubeTriangles* GetCubeRelative(int x, int y, int z);
			};

			CombinedVBO vbos[1];

			bool solidified;

			struct RelativeTerrainVertex
			{
				TerrainVertex* vertex;
				Vec3 offset;

				RelativeTerrainVertex(TerrainVertex* vertex) : vertex(vertex), offset() { }
				RelativeTerrainVertex(TerrainVertex* vertex, const Vec3& offset) : vertex(vertex), offset(offset) { }

				Vec3 GetPosition();

				static vector<RelativeTerrainVertex*> rel_vert_recycle_bin;
		
				static RelativeTerrainVertex* New(TerrainVertex* vertex)
				{
					if(rel_vert_recycle_bin.empty())
						return new RelativeTerrainVertex(vertex);
					else
					{
						RelativeTerrainVertex* result = *rel_vert_recycle_bin.rbegin();
						rel_vert_recycle_bin.pop_back();

						return new(result) RelativeTerrainVertex(vertex);
					}
				}
				static RelativeTerrainVertex* New(TerrainVertex* vertex, const Vec3& offset)
				{
					if(rel_vert_recycle_bin.empty())
						return new RelativeTerrainVertex(vertex, offset);
					else
					{
						RelativeTerrainVertex* result = *rel_vert_recycle_bin.rbegin();
						rel_vert_recycle_bin.pop_back();

						return new(result) RelativeTerrainVertex(vertex, offset);
					}
				}
				static void Delete(RelativeTerrainVertex* v) { rel_vert_recycle_bin.push_back(v); }

				static void PurgeRecycleBin()
				{
					for(vector<RelativeTerrainVertex*>::iterator iter = rel_vert_recycle_bin.begin(); iter != rel_vert_recycle_bin.end(); ++iter)
						delete *iter;
					rel_vert_recycle_bin.clear();
				}
			};

			void CreateVBOs(CombinedVBO& target);

			// these are functions used within CreateVBOs...
			void ProcessTriangle(RelativeTerrainVertex* v1, RelativeTerrainVertex* v2, RelativeTerrainVertex* v3, unordered_map<unsigned char, VoxelMaterialVBO>& vbos, unordered_map<unsigned char, VoxelMaterialVBOBuilder>& vbo_builders, float*& depth_vert_ptr, unsigned int num_verts);
			VertexBuffer* CreateVBO(unsigned int allocate_n);
			VoxelMaterialVBOBuilder& GetOrCreateVBO(unordered_map<unsigned char, VoxelMaterialVBOBuilder>& builders, unordered_map<unsigned char, VoxelMaterialVBO>& vbos, unsigned char material, unsigned int size_to_create);
			void ProcessVert(RelativeTerrainVertex& vert, VoxelMaterialVBOBuilder& target_vbo, unsigned char material, float inv_total);

			VoxelTerrain* owner;

		public:

			// If these were unsigned ints it would cause some stupid errors with underflow
			static const int ChunkSize = 16;
			static const int ChunkSizeSquared = ChunkSize * ChunkSize;
			static const float InvChunkSize;



			TerrainChunk(VoxelMaterial* material, VoxelTerrain* owner, int x, int y, int z);
			~TerrainChunk();

			template <class T> void PopulateValues(T t);



			/** 
			 * Get a pointer to the specified node
			 */
			TerrainNode* GetNode(int x, int y, int z);

			void GetChunkPosition(int& x, int& y, int& z);
			
			/**
			 * Get a pointer to the node at the specified position relative to this chunk, or NULL if the position is not within a non-NULL TerrainChunk
			 * Unlike GetNode, this works for nodes outside the range of this chunk
			 */
			TerrainNode* GetNodeRelative(int x, int y, int z);

			bool GetRelativePositionInfo(int x, int y, int z, TerrainChunk*& chunk, int& dx, int &dy, int& dz);						// internal utility function, but possibly useful for external code as well (thus, public)

			void InvalidateNode(int x, int y, int z);
			void InvalidateCubeRelative(int x, int y, int z);
			void InvalidateCubeNormalsRelative(int x, int y, int z);
			void InvalidateVBO();

			void SolidifyAsNeeded();
			void Solidify();

			void ModifySphere(const Vec3& center, float inner_radius, float outer_radius, TerrainAction& action);

			void Vis(SceneRenderer* renderer, const Mat4& main_xform);

			bool IsEmpty();				// check if this chunk is completely empty; call Solidify before calling this!
			bool IsEntirelySolid();		// similar to IsEmpty, but checks for solidity instead

			// Member I/O functions
			unsigned int Write(ostream& stream);
			unsigned int Read(istream& stream);
	};

	template <class T> void TerrainChunk::PopulateValues(T t) 
	{
		for(int x = 0; x < ChunkSize; ++x)
			for(int y = 0; y < ChunkSize; ++y)
				for(int z = 0; z < ChunkSize; ++z)
					*GetNode(x, y, z) = t(x, y, z);
	}
}
