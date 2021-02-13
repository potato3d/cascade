#pragma once
#include <glb/irenderer.h>
#include <glb/framebuffer.h>
#include <app/VertexArrayBuilder.h>
#include <app/AABB.h>
#include <app/mat34.h>

namespace app
{
	class OITRenderer : public glb::irenderer
	{
	public:
		void beginLoad();
		unsigned int addDrawable(const string& name, int colorID, const mat4& transform, const tess::triangle_mesh& mesh);
		void endLoad();

		unsigned int addColor(float r, float g, float b);

		void setDrawableVisible(unsigned int drawableID, bool visible);
		void setDrawableTransform(unsigned int drawableID, const mat4& transform);
		unsigned int getDrawableColorID(unsigned int drawableID);
		void setDrawableColorID(unsigned int drawableID, unsigned int colorID);
		const AABB& getDrawableBounds(unsigned int drawableID);
		const string& getDrawableName(unsigned int drawableID);

		unsigned int pickDrawable(unsigned int x, unsigned int y);

		const AABB& getSceneBounds();

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

	private:
		Buffer _colors;

		vector<unsigned int> _colorIDsCPU;
		Buffer _colorIDs;
		vector<mat34> _transformsCPU;
		Buffer _transforms;
		Buffer _drawIDs;
		vector<DrawCmd> _cmdsCPU;
		Buffer _cmds;
		vector<tess::triangle_mesh> _meshes;
		unsigned int _vertexCount = 0;
		unsigned int _elementCount = 0;
		unsigned int _vao = 0;
		unsigned int _shader = 0;
		vector<AABB> _bounds;
		AABB _sceneBounds;
		glb::framebuffer* _fbuffer;
		glb::framebuffer::color_buffer_id _pickBufferID;
		vector<string> _names;

		unsigned int _transparentFBO;
		unsigned int _accumTex;
		unsigned int _revealageTex;
		unsigned int _screenWidth;
		unsigned int _screenHeight;
		unsigned int _transparentShader;
		unsigned int _composeShader;
		vector<DrawCmd> _transparentCmdsCPU;
	};
} // namespace app
