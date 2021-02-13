#pragma once
#include <app/DrawCmd.h>
#include <app/Buffer.h>
#include <tess/triangle_mesh.h>

namespace app
{
	struct VAO
	{
		unsigned int id;
		vector<DrawCmd> cmds;
	};

	class VertexArrayBuilder
	{
	public:
		void init(unsigned int maxSizeBytes);
		void addMesh(const tess::triangle_mesh& mesh);
		vector<VAO>& getVAOs();

	private:
		void _createVAO();

		const int VBO_IDX = 0;
		const int POSITION_IDX = 0;
		const int NORMAL_IDX = 1;

		unsigned int _maxSizeBytes = 0;
		Buffer _vbo;
		Buffer _ebo;
		vector<VAO> _vaos;
	};
}
