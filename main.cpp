#include <QApplication>
#include <QGLWidget>
#include <QFontDatabase>
#include <QMainWindow>
#include <QToolBar>

#include <app/Global.h>
#include <app/GraphicsView.h>

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

#ifdef _WIN32
	if(QFontDatabase::addApplicationFont(QString::fromStdString("C:/develop/doctorate/sources/app_schedule3d/SegoeUI.ttf")) != -1)
#else
	if(QFontDatabase::addApplicationFont(QString::fromStdString("/home/potato/Develop/doctorate/sources/app_schedule3d/SegoeUI.ttf")) != -1)
#endif
	{
		QFont defaultFont(QString::fromStdString("Segoe UI"), 9, QFont::Normal);
		defaultFont.setStyleStrategy(QFont::PreferAntialias);
		QApplication::setFont(defaultFont);
	}

#ifdef _WIN32
		QString schedule1 = "C:/develop/doctorate/sources/app_schedule3d/schedule1.txt";
		QString schedule2 = "C:/develop/doctorate/sources/app_schedule3d/schedule2.txt";

//		QString schedule1 = "d:/develop/doctorate/sources/app_schedule3d/schedule1.txt";
//		QString schedule2 = "d:/develop/doctorate/sources/app_schedule3d/schedule2.txt";
#else
		QString schedule1 = "/home/potato/Develop/doctorate/sources/app_schedule3d/schedule1.txt";
		QString schedule2 = "/home/potato/Develop/doctorate/sources/app_schedule3d/schedule2.txt";
#endif

	app::Global::settings = new QSettings("settings.ini", QSettings::IniFormat);
	app::Global::toolbar = new QToolBar();
	app::Global::loader = new app::ModelLoader();
	app::Global::renderer = new app::Renderer();
	app::Global::glscene = new app::OpenGLScene();
	app::Global::camera = new app::Camera();
	app::Global::selection = new app::Selection();
	app::Global::hierarchy = new app::Hierarchy();
	app::Global::overlay = new app::Overlay();
	app::Global::schedule1 = new app::Schedule(schedule1);
	app::Global::schedule2 = new app::Schedule(schedule2);
	app::Global::explodedview = new app::ExplodedView();
	app::Global::analysis = new app::Analysis();

	auto view = new app::GraphicsView();
	view->setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
	view->setScene(app::Global::glscene);

	auto mw = new QMainWindow();
	mw->addToolBar(Qt::TopToolBarArea, app::Global::toolbar);
	mw->setCentralWidget(view);

	mw->resize(1024, 768);
	mw->show();

	return app.exec();
}
