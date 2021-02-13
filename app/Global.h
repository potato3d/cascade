#pragma once
#include <app/Camera.h>
#include <app/Renderer.h>
#include <app/OpenGLScene.h>
#include <app/ModelLoader.h>
#include <app/Selection.h>
#include <app/Hierarchy.h>
#include <app/Overlay.h>
#include <app/Schedule.h>
#include <app/ExplodedView.h>
#include <app/Analysis.h>
#include <QToolBar>
#include <QSettings>

namespace app
{
	struct Global
	{
		static QSettings* settings;
		static QToolBar* toolbar;
		static ModelLoader* loader;
		static Renderer* renderer;
		static OpenGLScene* glscene;
		static Camera* camera;
		static Selection* selection;
		static Hierarchy* hierarchy;
		static Overlay* overlay;
		static Schedule* schedule1;
		static Schedule* schedule2;
		static ExplodedView* explodedview;
		static Analysis* analysis;

		static QWidget* createSeparator()
		{
			QFrame* line = new QFrame();
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			return line;
		}

		static AABB getUpdatedBound(const vector<unsigned int>& drawables)
		{
			AABB b;
			for(auto d : drawables)
			{
				b.expand(Global::renderer->getDrawableBounds(d));
			}
			return b;
		}

		static QString getLabelStyle()
		{
			return "QLabel{font: 9pt; background-color:rgba(255,255,255,128);"
						  "border: 2px solid lightgrey;border-radius: 6px;}";
		}

		static void init()
		{
			// don't change this order!
			glscene->init();
			camera->init();
			selection->init();
			hierarchy->init();
			overlay->init();
			schedule1->init();
			schedule2->init();
			explodedview->init();
			analysis->init();

			// probably should be last
			loader->init();
		}
	};
}
