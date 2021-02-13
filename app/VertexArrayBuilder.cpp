#include <app/VertexArrayBuilder.h>
#include <glb/opengl.h>

namespace app
{
	void VertexArrayBuilder::init(unsigned int maxSizeBytes)
	{
		_maxSizeBytes = maxSizeBytes;
	}

	void VertexArrayBuilder::addMesh(const tess::triangle_mesh& mesh)
	{
		DrawCmd cmd;
		cmd.elementCount = mesh.elements.size();
		cmd.instanceCount = 1;
		cmd.firstElement = _ebo.getCount();
		cmd.baseVertex = _vbo.getCount();
		cmd.baseInstance = 0;

		bool needNewVAO = false;
		if(!_vbo.addData(mesh.vertices.size(), mesh.vertices.data()))
		{
			auto maxSizeBytes = math::max(_maxSizeBytes, (unsigned int)(mesh.vertices.size()*sizeof(tess::vertex)));
			_vbo.init(maxSizeBytes);
			if(!_vbo.addData(mesh.vertices.size(), mesh.vertices.data()))
			{
				throw std::exception();
			}
			cmd.baseVertex = 0;
			needNewVAO = true;
		}
		if(!_ebo.addData(mesh.elements.size(), mesh.elements.data()))
		{
			auto maxSizeBytes = math::max(_maxSizeBytes, (unsigned int)(mesh.elements.size()*sizeof(tess::element)));
			_ebo.init(maxSizeBytes);
			if(!_ebo.addData(mesh.elements.size(), mesh.elements.data()))
			{
				throw std::exception();
			}
			cmd.firstElement = 0;
			needNewVAO = true;
		}
		if(needNewVAO)
		{
			_createVAO();
		}

		_vaos.back().cmds.push_back(cmd);
	}

	vector<VAO>& VertexArrayBuilder::getVAOs()
	{
		return _vaos;
	}

	void VertexArrayBuilder::_createVAO()
	{
		VAO vao;
		glCreateVertexArrays(1, &vao.id);
		_vaos.push_back(vao);

		glVertexArrayVertexBuffer(vao.id, VBO_IDX, _vbo.getID(), 0, sizeof(tess::vertex));

		glEnableVertexArrayAttrib(vao.id, POSITION_IDX);
		glVertexArrayAttribBinding(vao.id, POSITION_IDX, VBO_IDX);
		glVertexArrayAttribFormat(vao.id, POSITION_IDX, 3, GL_FLOAT, GL_FALSE, 0);

		glEnableVertexArrayAttrib(vao.id, NORMAL_IDX);
		glVertexArrayAttribBinding(vao.id, NORMAL_IDX, VBO_IDX);
		glVertexArrayAttribFormat(vao.id, NORMAL_IDX, 3, GL_FLOAT, GL_FALSE, sizeof(vec3));

		glVertexArrayElementBuffer(vao.id, _ebo.getID());
	}
}
