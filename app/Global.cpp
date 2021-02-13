#include <app/Global.h>

namespace app
{
	QSettings* Global::settings = nullptr;
	QToolBar* Global::toolbar = nullptr;
	ModelLoader* Global::loader = nullptr;
	Renderer* Global::renderer = nullptr;
	OpenGLScene* Global::glscene = nullptr;
	Camera* Global::camera = nullptr;
	Selection* Global::selection = nullptr;
	Hierarchy* Global::hierarchy = nullptr;
	Overlay* Global::overlay = nullptr;
	Schedule* Global::schedule1 = nullptr;
	Schedule* Global::schedule2 = nullptr;
	ExplodedView* Global::explodedview = nullptr;
	Analysis* Global::analysis = nullptr;
}
