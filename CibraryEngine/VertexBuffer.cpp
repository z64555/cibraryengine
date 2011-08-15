#include "StdAfx.h"
#include "VertexBuffer.h"

#include "DebugLog.h"

namespace CibraryEngine
{
	/*
	 * VertexBuffer methods
	 */
	VertexBuffer::VertexBuffer(DrawMode storage_mode) :
		attributes(),
		attribute_data(),
		storage_mode(storage_mode),
		num_verts(0),
		allocated_size(0),
		vbo_id(0)
	{
	}

	void VertexBuffer::InnerDispose()
	{
		InvalidateVBO();

		vector<VertexAttribute> attribs = GetAttributes();
		for(unsigned int i = 0; i < attribs.size(); i++)
			RemoveAttribute(attribs[i].name);

		SetNumVerts(0);
	}

	DrawMode VertexBuffer::GetStorageMode() { return storage_mode; }

	unsigned int VertexBuffer::GetNumVerts() { return num_verts; }

	void VertexBuffer::SetNumVerts(unsigned int verts)
	{
		if(verts > allocated_size)
		{
			allocated_size = verts + 20;			// in case they want to upsize it again soon

			map<string, VertexData> nu_attribute_data;
			for(map<string, VertexData>::iterator iter = attribute_data.begin(); iter != attribute_data.end(); iter++)
			{
				VertexAttribute attrib = GetAttribute(iter->first);
				if(attrib.type == Float)
				{
					float* new_data = new float[allocated_size * attrib.n_per_vertex];
					float* old_data = iter->second.floats;
					if(old_data != NULL)
					{
						for(unsigned int i = 0; i < num_verts * attrib.n_per_vertex; i++)
							new_data[i] = old_data[i];
					}
					nu_attribute_data[iter->first] = VertexData(new_data);
				}
			}

			for(map<string, VertexData>::iterator iter = attribute_data.begin(); iter != attribute_data.end(); iter++)
			{
				VertexAttribute attrib = GetAttribute(iter->first);
				if(attrib.type == Float)
				{
					float* floats = iter->second.floats;
					delete[] floats;
				}
			}
			attribute_data.clear();

			attribute_data = nu_attribute_data;
		}
		else if(verts == 0)
		{
			allocated_size = 0;

			map<string, VertexData> nu_attribute_data;
			for(map<string, VertexData>::iterator iter = attribute_data.begin(); iter != attribute_data.end(); iter++)
			{
				VertexAttribute attrib = GetAttribute(iter->first);

				if(attrib.type == Float)
				{
					float* old_data = iter->second.floats;
					if(old_data != NULL)
						delete[] old_data;
				}

				nu_attribute_data[iter->first] = VertexData();
			}

			attribute_data = nu_attribute_data;
		}

		num_verts = verts;
	}

	void VertexBuffer::AddAttribute(string name, VertexAttributeType type, int n_per_vertex)
	{
		RemoveAttribute(name);				// in case it already exists!

		VertexAttribute attribute = VertexAttribute(name, type, n_per_vertex);
		attributes[name] = attribute;

		int num_elements = n_per_vertex * allocated_size;

		if(type == Float)
			attribute_data[name].floats = num_elements > 0 ? new float[num_elements] : NULL;
	}

	void VertexBuffer::RemoveAttribute(string name)
	{
		map<string, VertexAttribute>::iterator found_name = attributes.find(name);
		if(found_name != attributes.end())
		{
			VertexAttribute attribute = found_name->second;
			attributes.erase(found_name);

			map<string, VertexData>::iterator found_attrib = attribute_data.find(name);
			if(found_attrib != attribute_data.end())
			{
				VertexData data = found_attrib->second;

				if(attribute.type == Float)
					delete[] data.floats;

				attribute_data.erase(found_attrib);
			}
		}
	}

	VertexAttribute VertexBuffer::GetAttribute(string name)
	{
		map<string, VertexAttribute>::iterator found_name = attributes.find(name);
		if(found_name != attributes.end())
			return found_name->second;
		else
			return VertexAttribute();
	}

	vector<VertexAttribute> VertexBuffer::GetAttributes()
	{
		vector<VertexAttribute> results;
		for(map<string, VertexAttribute>::iterator iter = attributes.begin(); iter != attributes.end(); iter++)
			results.push_back(iter->second);
		return results;
	}

	float* VertexBuffer::GetFloatPointer(string name)
	{
		VertexAttribute a = GetAttribute(name);
		if(a.type == Float)
			return attribute_data[name].floats;
		else
			return NULL;
	}

	void VertexBuffer::InvalidateVBO()
	{ 
		if(vbo_id != 0)
		{
			glDeleteBuffers(1, &vbo_id);
			vbo_id = 0;
		}
	}

	void VertexBuffer::BuildVBO()
	{
		GLDEBUG();

		vector<VertexAttribute> attribs = GetAttributes();

		// figure out the total size of our vertex info
		int total_size = 0;
		for(unsigned int i = 0; i < attribs.size(); i++)
		{
			VertexAttribute attrib = attribs[i];

			if(attrib.type == Float)
				total_size += sizeof(float) * attrib.n_per_vertex;
		}

		InvalidateVBO();					// just in case...

		// generate a vbo
		glGenBuffers(1, &vbo_id);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

		// set the total size (based on the value we computed earlier)
		glBufferData(GL_ARRAY_BUFFER, total_size * num_verts, NULL, GL_STATIC_DRAW);

		int offset = 0;
		for(unsigned int i = 0; i < attribs.size(); i++)
		{
			VertexAttribute attrib = attribs[i];

			int attrib_size = 0;
			if(attrib.type == Float)
				attrib_size = sizeof(float) * attrib.n_per_vertex;

			if(attrib_size > 0)
			{
				if(attrib.type == Float)
					glBufferSubData(GL_ARRAY_BUFFER, offset * num_verts, attrib_size * num_verts, attribute_data[attrib.name].floats);

				offset += attrib_size;
			}
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);						// don't leave hardware vbo on

		GLDEBUG();
	}
	
	unsigned int VertexBuffer::GetVBO()
	{
		if(vbo_id == 0) 
			BuildVBO(); 

		return vbo_id;
	}

	string MultiTexName(int i)
	{
		stringstream ss;
		ss << "gl_MultiTexCoord" << i;
		return ss.str();
	}

	void VertexBuffer::Draw()
	{
		GLDEBUG();

		unsigned int vbo = GetVBO();
		vector<VertexAttribute> attribs = GetAttributes();


		/*
		 * First, we set everything up for our VBO draw operation...
		 */
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		int offset = 0;
		for(unsigned int i = 0; i < attribs.size(); i++)
		{
			VertexAttribute attrib = attribs[i];
			if(attrib.name.length() >= 3 && attrib.name.substr(0, 3) == "gl_")
			{
				if(attrib.name == "gl_Vertex")
				{
					glEnable(GL_VERTEX_ARRAY);
					glVertexPointer(attrib.n_per_vertex, (GLenum)attrib.type, 0,	(void*)(num_verts * offset));
				}
				else if(attrib.name == "gl_Normal")
				{
					glEnable(GL_NORMAL_ARRAY);
					glNormalPointer((GLenum)attrib.type, 0, (void*)(num_verts * offset));
				}
				else
				{
					int max_texture_units;
					glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
					for(int j = 0; j < max_texture_units; j++)
					{
						if(attrib.name == MultiTexName(j))
						{
							glClientActiveTexture(GL_TEXTURE0 + j);
							glEnable(GL_TEXTURE_COORD_ARRAY);
							glTexCoordPointer(attrib.n_per_vertex,	(GLenum)attrib.type, 0, (void*)(num_verts * offset));
						}
					}
				}
			}
			else
			{
				GLuint index = 0;	//(GLuint)glGetAttribLocation(program, name);
				glEnableVertexAttribArray(index);
				glVertexAttribPointer(index, attrib.n_per_vertex, (GLenum)attrib.type, true, 0, (void*)(num_verts * offset));
			}

			if(attrib.type == Float)
				offset += attrib.n_per_vertex * sizeof(float);
		}


		/*
		 * Now for the draw call itself...
		 */
		glDrawArrays((GLenum)storage_mode, 0, num_verts);


		/*
		 * Now to put everything back the way we found it...
		 */
		for(unsigned int i = 0; i < attribs.size(); i++)
		{
			VertexAttribute attrib = attribs[i];
			if(attrib.name.length() >= 3 && attrib.name.substr(0, 3) == "gl_")
			{
				if(attrib.name == "gl_Vertex")
					glDisable(GL_VERTEX_ARRAY);
				else if(attrib.name == "gl_Normal")
					glDisable(GL_NORMAL_ARRAY);
				else
				{
					int max_texture_units;
					glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_texture_units);
					for(int j = 0; j < max_texture_units; j++)
					{
						if(attrib.name == MultiTexName(j))
						{
							glClientActiveTexture(GL_TEXTURE0 + j);
							glDisable(GL_TEXTURE_COORD_ARRAY);
						}
					}
				}
			}
			else
			{
				GLuint index = 0;	//(GLuint)glGetAttribLocation(program, name);
				glDisableVertexAttribArray(index);
			}
		}

		glClientActiveTexture(GL_TEXTURE0);			// get texcoords back to working "the normal way"
		glBindBuffer(GL_ARRAY_BUFFER, 0);			// don't leave hardware vbo on

		GLDEBUG();
	}

	// TODO: implement these
	void VertexBuffer::Draw(DrawMode mode) { }
	void VertexBuffer::Draw(DrawMode mode, unsigned int* indices, int num_indices) { }
	void VertexBuffer::Draw(unsigned int num_instances) { }
	void VertexBuffer::Draw(unsigned int num_instances, DrawMode mode) { }
	void VertexBuffer::Draw(unsigned int num_instances, DrawMode mode, unsigned int* indices, int num_indices) { }
}