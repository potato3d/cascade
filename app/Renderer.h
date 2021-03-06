#pragma once
#include <glb/irenderer.h>
#include <glb/framebuffer.h>
#include <app/VertexArrayBuilder.h>
#include <app/AABB.h>
#include <app/mat34.h>
#include <QObject>
#include <QSet>

namespace app
{
	class Renderer : public QObject, public glb::irenderer
	{
		Q_OBJECT
	signals:
		void drawableVisiblityChanged(unsigned int drawableID, bool visible);
		void drawableTransformChanged(unsigned int drawableID, const mat4& transform);
		void drawableColorChanged(unsigned int drawableID, unsigned int colorID);
		void drawableTransparencyChanged(unsigned int drawableID, bool transparent);

		void beforeRender();

	public:
		static constexpr float HighlightNone = 0.0f;
		static constexpr float HighlightCyan = 1.0f;
		static constexpr float HighlightGreen = 2.0f;
		static constexpr float HighlightBlue = 3.0f;
		static constexpr float HighlightRed = 4.0f;
		static constexpr float HighlightOrange = 5.0f;
		static constexpr float HighlightMagenta = 6.0f;

		unsigned int addExtraDrawable(const string& name, int colorID, const tess::triangle_mesh& mesh);
		unsigned int addExtraDrawable(const string& name, int colorID, const mat4& transform, const tess::triangle_mesh& mesh);
		unsigned int addSceneDrawable(const string& name, int colorID, const tess::triangle_mesh& mesh);
		unsigned int addDrawableData(const string& name, int colorID, const tess::triangle_mesh& mesh, const AABB& bound);

		void endLoad();

		void setDrawablesHighlight(const unsigned int* drawableIDs, unsigned int count, float mode, bool enable);

		unsigned int addLineDrawable(int colorID, const vec3& p0, const vec3& p1);
		void setLineDrawableVisible(unsigned int drawableID, bool visible);
		void setLineDrawableStipple(unsigned int drawableID, bool stipple);
		void setLineDrawablePoints(unsigned int drawable, const vec3& p0, const vec3& p1);

		unsigned int addColor(float r, float g, float b);

		void setDrawableVisible(unsigned int drawableID, bool visible);
		void setDrawableTransform(unsigned int drawableID, const mat4& transform);
		unsigned int getDrawableColorID(unsigned int drawableID);
		void setDrawableColorID(unsigned int drawableID, unsigned int colorID);
		void setDrawableTransparent(unsigned int drawableID, bool transparent);
		const AABB& getDrawableBounds(unsigned int drawableID);
		const string& getDrawableName(unsigned int drawableID);

		void beginTransformBatch();
		void setBatchDrawableTransform(unsigned int drawableID, const mat4& transform);
		void endTransformBatch();

		unsigned int pickDrawable(unsigned int x, unsigned int y);
		unsigned int getFirstSceneDrawable();

		const AABB& getSceneOriginalBounds();
		const AABB& getSceneBounds();
		unsigned int getLastSceneDrawable();

		bool isDuplicateDrawable(unsigned int drawableID);
		unsigned int getOriginalDrawableID(unsigned int duplicateDrawableID);
		unsigned int getDuplicateDrawableID(unsigned int drawableID);

		bool isDrawableVisible(unsigned int drawableID);
		bool isDrawableTransparent(unsigned int drawableID);

		mat4 getView();
		mat4 getProjection();
		unsigned int getScreenWidth();
		unsigned int getScreenHeight();

		float getTransparencyLevel();
		void setTransparencyLevel(float alpha);

		virtual bool initialize(glb::framebuffer& fbuffer, glb::camera& cam) override;
		virtual bool finalize() override;
		virtual void render() override;

	private:
		struct LineMesh
		{
			vec3 p0;
			vec3 p1;
		};

		struct DrawArraysCmd
		{
			unsigned int count;
			unsigned int instanceCount;
			unsigned int first;
			unsigned int baseInstance;
		};

		struct LineVisual
		{
			unsigned char pad : 6;
			unsigned char stipple : 1;
			unsigned char visible : 1;
		};

		struct LineState
		{
			LineVisual visual;
			AABB bound;
		};



		struct Visual
		{
			unsigned char pad : 6;
			unsigned char transparent : 1;
			unsigned char visible : 1;
		};

		struct State
		{
			Visual visual;
			AABB bound;
		};

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
		struct Handle
		{
			Handle(unsigned int d, unsigned int et, unsigned int t, unsigned int di) : depth(d), extraTransparent(et), transparent(t), drawableID(di) {}
			union
			{
				struct
				{
					unsigned int depth : 30;
					unsigned int extraTransparent : 1;
					unsigned int transparent : 1;
				};
				unsigned int key;
			};
			unsigned int drawableID;
			bool operator<(const Handle& o)
			{
				return key < o.key;
			}
		};
#pragma GCC diagnostic pop

		struct Loader
		{
			unsigned int vertexCount = 0;
			unsigned int elementCount = 0;
			vector<tess::triangle_mesh> meshes;
			vector<unsigned int> colorIDs;
			unsigned int firstSceneDrawable = 0;
			unsigned int lastSceneDrawable = 0;
		};

		struct Scene
		{
			AABB bound;
			AABB originalBound;
			vector<string> names;
			vector<State> states;
			vector<DrawCmd> cmds;
			vector<unsigned int> vertexCounts;
		};

		struct Visible
		{
			vector<Handle> handles;
			vector<DrawCmd> cmds;
			unsigned int opaqueCount;
			unsigned int transparentCount;
			unsigned int extraTransparentCount;
		};

		struct GPU
		{
			Buffer vbo;
			unsigned int vao;
			unsigned int drawProgram;
			unsigned int composeProgram;
			Buffer colors;
			Buffer colorIDs;
			Buffer transforms;
			Buffer drawableIDs;
			Buffer highlightModes;
			unsigned int gpuHighlightModesID;
			float* gpuHighlightModesData;
		};

		struct LineScene
		{
			vector<LineState> states;
			vector<DrawArraysCmd> cmds;
		};

		struct LineVisible
		{
			vector<DrawArraysCmd> smoothCmds;
			vector<DrawArraysCmd> stippleCmds;
		};

		struct LineGPU
		{
			Buffer vbo;
			Buffer colorIDs;
			unsigned int vao;
			unsigned int drawProgram;
		};

		bool _isCulled(const AABB& bound);
		unsigned int _getDepth(const AABB& bound);

		float _transparencyAlpha = 0.6f;

		Loader _loader;
		Scene _scene;
		Visible _visible;
		GPU _gpu;
		LineScene _lineScene;
		LineVisible _lineVisible;
		LineGPU _lineGPU;
		unsigned int _pickBufferID;
		glb::framebuffer* _fbuffer;
		glb::camera* _camera;
		vec3 _camPos;

		bool _sceneBoundsDirty = true;
		QHash<unsigned int, mat4> _dirtyDrawables;

		unsigned int _gpuTimers[3] = {0,0,0};

		std::vector<float> _cpuHighlightModes;
	};
} // namespace app
