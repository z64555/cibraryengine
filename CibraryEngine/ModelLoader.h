#pragma once

#include "StdAfx.h"

#include "Model.h"
#include "VertexBuffer.h"
#include "UberModel.h"

namespace CibraryEngine
{
	/** ContentLoader for VertexBuffer assets */
	struct ModelLoader : public ContentTypeHandler<VertexBuffer>
	{
		ModelLoader(ContentMan* man);

		VertexBuffer* Load(ContentMetadata& what);
		void Unload(VertexBuffer* content, ContentMetadata& meta);
	};

	/** ContentLoader for SkinnedModel assets */
	struct SkinnedModelLoader : public ContentTypeHandler<SkinnedModel>
	{
		SkinnedModelLoader(ContentMan* man);

		SkinnedModel* Load(ContentMetadata& what);
		void Unload(SkinnedModel* content, ContentMetadata& meta);
	};

	/** Loads a .OBJ model file, returning 0 if ok or an int error code otherwise */
	int LoadOBJ(string filename, VertexBuffer* vbo);

	int SaveOBJ(string filename, VertexBuffer* vbo);

	/** Loads a .AAM model file, returning 0 if ok or an int error code otherwise */
	int LoadAAM(string filename, VertexBuffer* vbo);
	/** Saves a .AAM model file, returning 0 if ok or an int error code otherwise */
	int SaveAAM(string filename, VertexBuffer* vbo, bool overwrite = true);		// is overwrite even used?

	/** Loads a .AAK skin file, returning 0 if ok or an int error code otherwise */
	int LoadAAK(string filename, vector<MaterialModelPair>& material_model_pairs, vector<string>& material_names, Skeleton*& skeleton);
	/** Saves a .AAK skin file, returning 0 if ok or an int error code otherwise */
	int SaveAAK(string filename, SkinnedModel* model, bool overwrite = true);			// is overwrite even used?

	class KeyframeAnimation;

	/** Loads a .AAA animation file, returning 0 if ok or an int error code otherwise */
	int LoadAAA(string filename, KeyframeAnimation& anim);
	/** Saves a .AAA animation file, returning 0 if ok or an int error code otherwise */
	int SaveAAA(string filename, KeyframeAnimation& anim);
}
