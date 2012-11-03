#pragma once

#include "StdAfx.h"

namespace CibraryEngine
{
	using namespace std;

	class Shader;
	struct VertexBuffer;

	struct HardwareAcceleratedComputation
	{
		private:

			Shader* shader;

			GLuint shader_program;								// TODO: make it possible to specify uniforms (e.g. by wrapping with a ShaderProgram)

			GLuint output_vertex_array;
			vector<GLuint> output_channels;						// array buffers which will go inside the output vertex array

			GLuint query;

			bool init_ok;

			bool InitShaderProgram();
			bool InitArrayBuffers();
			bool InitVertexArrays();

			void ResizeArrayBuffers(unsigned int num_verts);

		public:

			vector<const GLchar*> varying_names;				// names of the output variables... indices are parallel to output_channels

			HardwareAcceleratedComputation(Shader* shader, vector<const GLchar*>& varying_names);
			~HardwareAcceleratedComputation();

			void Process(VertexBuffer* input_data, VertexBuffer* output_data);
	};
}
