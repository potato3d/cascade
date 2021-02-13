#include <app/OITRenderer.h>
#include <glb/opengl.h>
#include <glb/shader_program_builder.h>
#include <glb/camera.h>
#include <glb/framebuffer.h>
#include <rvm/MaterialTable.h>
#include <app/Color.h>

namespace app
{
	void OITRenderer::beginLoad()
	{
	}

	unsigned int OITRenderer::addDrawable(const string& name, int colorID, const mat4& transform, const tess::triangle_mesh& mesh)
	{
		_names.push_back(name);

		_colorIDsCPU.push_back(colorID);
		_transformsCPU.push_back(mat34(transform));
		_meshes.push_back(mesh);

		DrawCmd cmd;
		cmd.elementCount = mesh.elements.size();
		cmd.instanceCount = 1;
		cmd.firstElement = _elementCount;
		cmd.baseVertex = _vertexCount;
		cmd.baseInstance = _cmdsCPU.size(); // for drawID
		_cmdsCPU.push_back(cmd);

		_vertexCount += mesh.vertices.size();
		_elementCount += mesh.elements.size();

		AABB bounds;
		for(const auto& v : mesh.vertices)
		{
			auto tv = transform.mul(v.position);
			bounds.expand(tv);
			_sceneBounds.expand(tv);
		}
		_bounds.push_back(bounds);

		return cmd.baseInstance;
	}

	void OITRenderer::endLoad()
	{
		_colorIDs.init(_colorIDsCPU.size()*sizeof(unsigned int));
		_colorIDs.addData(_colorIDsCPU.size(), _colorIDsCPU.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _colorIDs.getID());
		_colorIDsCPU.clear();
		_colorIDsCPU.shrink_to_fit();

		_transforms.init(_transformsCPU.size()*sizeof(mat34));
		_transforms.addData(_transformsCPU.size(), _transformsCPU.data());
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _transforms.getID());
		_transformsCPU.clear();
		_transformsCPU.shrink_to_fit();

		vector<unsigned int> drawIDs(_cmdsCPU.size());
		unsigned int counter = 0;
		for(auto& id : drawIDs)
		{
			id = counter++;
		}
		_drawIDs.init(drawIDs.size()*sizeof(unsigned int));
		_drawIDs.addData(drawIDs.size(), drawIDs.data());

		Buffer vbo;
		vbo.init(_vertexCount*sizeof(tess::vertex));
		Buffer ebo;
		ebo.init(_elementCount*sizeof(tess::element));
		for(const auto& m : _meshes)
		{
			vbo.addData(m.vertices.size(), m.vertices.data());
			ebo.addData(m.elements.size(), m.elements.data());
		}
		_meshes.clear();
		_meshes.shrink_to_fit();

		glCreateVertexArrays(1, &_vao);

		glVertexArrayVertexBuffer(_vao, 0, vbo.getID(), 0, sizeof(tess::vertex));

		glEnableVertexArrayAttrib(_vao, 0);
		glVertexArrayAttribBinding(_vao, 0, 0);
		glVertexArrayAttribFormat(_vao, 0, 3, GL_FLOAT, GL_FALSE, 0);

		glEnableVertexArrayAttrib(_vao, 1);
		glVertexArrayAttribBinding(_vao, 1, 0);
		glVertexArrayAttribFormat(_vao, 1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3));

		glVertexArrayVertexBuffer(_vao, 1, _drawIDs.getID(), 0, sizeof(unsigned int));
		glVertexArrayBindingDivisor(_vao, 1, 1);

		glEnableVertexArrayAttrib(_vao, 2);
		glVertexArrayAttribBinding(_vao, 2, 1);
		glVertexArrayAttribIFormat(_vao, 2, 1, GL_UNSIGNED_INT, 0);

		glVertexArrayElementBuffer(_vao, ebo.getID());

//		_cmds.init(_cmdsCPU.size()*sizeof(DrawCmd));
//		_cmds.addData(_cmdsCPU.size(), _cmdsCPU.data());
//		_cmdsCPU.clear();
//		_cmdsCPU.shrink_to_fit();

		io::print("drawable count:", _transforms.getCount());
		io::print("triangle count:", ebo.getCount()/3);

		// TODO: transparent tests
//		_transparentCmdsCPU.insert(_transparentCmdsCPU.end(), _cmdsCPU.begin(), _cmdsCPU.end()); // TODO
//		_cmdsCPU.erase(_cmdsCPU.begin()+1, _cmdsCPU.end());

		unsigned int pickBufferName = _fbuffer->get_color_buffer_name(_pickBufferID);

		_screenWidth = _fbuffer->get_width();
		_screenHeight = _fbuffer->get_height();

		glCreateFramebuffers(1, &_transparentFBO);

		glCreateTextures(GL_TEXTURE_2D, 1, &_accumTex);
		glTextureImage2DEXT(_accumTex, GL_TEXTURE_2D, 0, GL_RGBA16F, _screenWidth, _screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
		glTextureParameteri(_accumTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_accumTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_accumTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(_accumTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glNamedFramebufferTexture(_transparentFBO, GL_COLOR_ATTACHMENT0, _accumTex, 0);

		glCreateTextures(GL_TEXTURE_2D, 1, &_revealageTex);
		glTextureImage2DEXT(_revealageTex, GL_TEXTURE_2D, 0, GL_R8, _screenWidth, _screenHeight, 0, GL_RED, GL_FLOAT, nullptr);
		glTextureParameteri(_revealageTex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_revealageTex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureParameteri(_revealageTex, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(_revealageTex, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glNamedFramebufferTexture(_transparentFBO, GL_COLOR_ATTACHMENT1, _revealageTex, 0);

		glNamedFramebufferTexture(_transparentFBO, GL_COLOR_ATTACHMENT2, pickBufferName, 0);

		glNamedFramebufferTexture(_transparentFBO, GL_DEPTH_ATTACHMENT, _fbuffer->get_depth_buffer_name(), 0);

		GLenum drawBuffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
		glNamedFramebufferDrawBuffers(_transparentFBO, 3, drawBuffers);


		glb::shader_program_builder tspb;
		tspb.begin();
		if(!tspb.add_file(glb::shader_vertex, "../shaders/schedule3d/draw.vert"))
		{
			throw std::exception();
		}
		if(!tspb.add_file(glb::shader_fragment, "../shaders/schedule3d/transparent_draw.frag"))
		{
			throw std::exception();
		}
		if(!tspb.end())
		{
			throw std::exception();
		}
		auto transparent_draw_shader = tspb.get_shader_program();
		_transparentShader = transparent_draw_shader.get_id();


		glb::shader_program_builder cspb;
		cspb.begin();
		if(!cspb.add_file(glb::shader_geometry, "../shaders/depth_mipmap.geom"))
		{
			throw std::exception();
		}
		if(!cspb.add_file(glb::shader_vertex, "../shaders/depth_mipmap.vert"))
		{
			throw std::exception();
		}
		if(!cspb.add_file(glb::shader_fragment, "../shaders/schedule3d/compose_draw.frag"))
		{
			throw std::exception();
		}
		if(!cspb.end())
		{
			throw std::exception();
		}
		auto compose_draw_shader = cspb.get_shader_program();
		_composeShader = compose_draw_shader.get_id();
	}

	unsigned int OITRenderer::addColor(float r, float g, float b)
	{
		unsigned int id = _colors.getCount();
		Color c;
		c.r = r*255.0f;
		c.g = g*255.0f;
		c.b = b*255.0f;
		c.a = 255;
		_colors.addData(1, &c);
		return id;
	}

	void OITRenderer::setDrawableVisible(unsigned int drawableID, bool visible)
	{
//		unsigned int instanceCount = visible? 1 : 0;
//		glNamedBufferSubData(_cmds.getID(), drawableID*sizeof(DrawCmd)+sizeof(unsigned int), sizeof(unsigned int), &instanceCount);
		_cmdsCPU.at(drawableID).instanceCount = visible? 1 : 0;
	}

	void OITRenderer::setDrawableTransform(unsigned int drawableID, const mat4& transform)
	{
		glNamedBufferSubData(_transforms.getID(), drawableID*sizeof(mat34), sizeof(mat34), mat34(transform).data);
	}

	unsigned int OITRenderer::getDrawableColorID(unsigned int drawableID)
	{
		unsigned int id = 0;
		glGetNamedBufferSubData(_colorIDs.getID(), drawableID*sizeof(unsigned int), sizeof(unsigned int), &id);
		return id;
	}

	void OITRenderer::setDrawableColorID(unsigned int drawableID, unsigned int colorID)
	{
		glNamedBufferSubData(_colorIDs.getID(), drawableID*sizeof(unsigned int), sizeof(unsigned int), &colorID);
	}

	const AABB& OITRenderer::getDrawableBounds(unsigned int drawableID)
	{
		return _bounds.at(drawableID);
	}

	const string& OITRenderer::getDrawableName(unsigned int drawableID)
	{
		return _names.at(drawableID);
	}

	unsigned int OITRenderer::pickDrawable(unsigned int x, unsigned int y)
	{
		union
		{
			float f;
			unsigned int ui;
		} drawableID;
		_fbuffer->read_color_pixels(_pickBufferID, glb::type_float, x, y, 1, 1, &drawableID.f);
		return drawableID.ui;
	}

	const AABB& OITRenderer::getSceneBounds()
	{
		return _sceneBounds;
	}

	bool OITRenderer::initialize(glb::framebuffer& fbuffer, glb::camera& cam)
	{
		rvm::MaterialTable mt;
		_colors.init(1024*sizeof(Color));
		for(int i = 0; i < mt.getNumberMaterials(); ++i)
		{
			const auto m = mt.getMaterial(i);
			Color c;
			c.r = m.diffuseColor[0]*255.0f;
			c.g = m.diffuseColor[1]*255.0f;
			c.b = m.diffuseColor[2]*255.0f;
			c.a = 255; // TODO: transparent tests
			_colors.addData(1, &c);
		}
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, _colors.getID());

		glb::shader_program_builder spb;
		spb.begin();
		if(!spb.add_file(glb::shader_vertex, "../shaders/schedule3d/draw.vert"))
		{
			return false;
		}
		if(!spb.add_file(glb::shader_fragment, "../shaders/schedule3d/draw.frag"))
		{
			return false;
		}
		if(!spb.end())
		{
			return false;
		}
		auto draw_shader = spb.get_shader_program();
		_shader = draw_shader.get_id();

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, cam.get_uniform_buffer().get_id());
		glUniformBlockBinding(_shader, 0, 0);

		fbuffer.set_use_textures(true);
		fbuffer.set_clear_color(0, 1,1,1);

		_pickBufferID = fbuffer.add_color_buffer();
		fbuffer.set_color_format(_pickBufferID, glb::internal_format_r32f);
		fbuffer.set_clear_color(_pickBufferID, 0,0,0,0);

		_fbuffer = &fbuffer;

		return true;
	}

	bool OITRenderer::finalize()
	{
		return true;
	}

	void OITRenderer::render()
	{
		// OPAQUE ----------------------------------------------------------------------------------------------------------------------------

		glUseProgram(_shader);
		glBindVertexArray(_vao);
//		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, _cmds.getID());
//		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, _cmds.getCount(), 0);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, _cmdsCPU.data(), _cmdsCPU.size(), 0);

		return;

		// TODO: use flag to also set transparency directly inside shader (maybe negative colorID)
		// TODO: refactor code to collect visible and also implement traditional transparency to compare
		// TODO: improve OIT quality: increase precision, change algorithm

		// TRANSPARENT -----------------------------------------------------------------------------------------------------------------------

		if(_fbuffer->get_width() != _screenWidth || _fbuffer->get_height() != _screenHeight)
		{
			_screenWidth = _fbuffer->get_width();
			_screenHeight = _fbuffer->get_height();
			glTextureImage2DEXT(_accumTex, GL_TEXTURE_2D, 0, GL_RGBA16F, _screenWidth, _screenHeight, 0, GL_RGBA, GL_FLOAT, nullptr);
			glTextureImage2DEXT(_revealageTex, GL_TEXTURE_2D, 0, GL_R8, _screenWidth, _screenHeight, 0, GL_RED, GL_FLOAT, nullptr);
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _transparentFBO);
		float accumClear[] = {0,0,0,0};
		glClearNamedFramebufferfv(_transparentFBO, GL_COLOR, 0, accumClear);
		float revealageClear[] = {1,1,1,1};
		glClearNamedFramebufferfv(_transparentFBO, GL_COLOR, 1, revealageClear);

		glDepthMask(GL_FALSE);
		// TODO: how to preserve correct pickID for transparent geometries?

		glEnable(GL_BLEND);
		glBlendFunci(0, GL_ONE, GL_ONE);
		glBlendFunci(1, GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

		glUseProgram(_transparentShader);
		glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, _transparentCmdsCPU.data(), _transparentCmdsCPU.size(), 0);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, _fbuffer->get_id());
		glDrawBuffer(GL_COLOR_ATTACHMENT0);

		glDisable(GL_DEPTH_TEST);

		glBlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, _accumTex);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, _revealageTex);

		glUseProgram(_composeShader);
		glDrawArrays(GL_POINTS, 0, 1);

		glDepthMask(GL_TRUE);
	}
}
