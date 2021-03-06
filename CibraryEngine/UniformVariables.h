#pragma once

#include "StdAfx.h"
#include "Shader.h"
#include "Texture1D.h"
#include "Texture2D.h"
#include "Texture3D.h"
#include "TextureCube.h"
#include "Vector.h"
#include "Matrix.h"

namespace CibraryEngine
{
	class UniformInt : public TypedUniformVariable<int>
	{
		public:
			void ApplyValue(int location);

			int value;

			UniformInt(string name);

			int* GetValue();
			void SetValue(int* v);
	};

	class UniformFloat : public TypedUniformVariable<float>
	{
		public:
			void ApplyValue(int location);

			float value;

			UniformFloat(string name);

			float* GetValue();
			void SetValue(float* v);
	};

	class UniformVector3 : public TypedUniformVariable<Vec3>
	{
		public:
			void ApplyValue(int location);

			Vec3 value;

			UniformVector3(string name);

			Vec3* GetValue();
			void SetValue(Vec3* v);
	};

	class UniformTexture1D : public TypedUniformVariable<Texture1D>
	{
		public:
			virtual void ApplyValue(int location);

			int which;
			Texture1D* texture;

			UniformTexture1D(string name, int which);

			Texture1D* GetValue();
			void SetValue(Texture1D* t);

			void Disable();
	};

	class UniformTexture2D : public TypedUniformVariable<Texture2D>
	{
		public:
			virtual void ApplyValue(int location);

			int which;
			Texture2D* texture;

			UniformTexture2D(string name, int which);

			Texture2D* GetValue();
			void SetValue(Texture2D* t);

			void Disable();
	};

	class UniformTexture3D : public TypedUniformVariable<Texture3D>
	{
		public:
			virtual void ApplyValue(int location);

			int which;
			Texture3D* texture;

			UniformTexture3D(string name, int which);

			Texture3D* GetValue();
			void SetValue(Texture3D* t);

			void Disable();
	};

	class UniformTextureCube : public TypedUniformVariable<TextureCube>
	{
		public:

			int which;
			TextureCube* cubemap;

			UniformTextureCube(string name, int which);

			TextureCube* GetValue();
			void SetValue(TextureCube* c);

			void ApplyValue(int location);

			void Disable();
	};

	class UniformMatrix4 : public TypedUniformVariable<Mat4>
	{
		public:

			bool transpose;
			Mat4 matrix;

			UniformMatrix4(string name, bool transpose);

			Mat4* GetValue();
			void SetValue(Mat4* m);

			void ApplyValue(int location);
	};

	template<class T> class UniformArray : public TypedUniformVariable<vector<T> >
	{
		public:

			vector<T>* array;

			UniformArray(string name) : TypedUniformVariable<vector<T> >(name), array(new vector<T>()) { }

			vector<T>* GetValue() { return array; }
			void SetValue(vector<T>* array_values) { array = array_values; }

			virtual void ApplyValue(int location) = 0;
	};

	class UniformMatrix4Array : public UniformArray<Mat4>
	{
		public:

			bool transpose;

			UniformMatrix4Array(string name, bool transpose);

			void ApplyValue(int location);
	};
}
