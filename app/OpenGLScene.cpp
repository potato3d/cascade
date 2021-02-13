#include <app/OpenGLScene.h>
#include <glb/opengl.h>
#include <QApplication>
#include <QFileDialog>
#include <QGraphicsProxyWidget>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPaintEngine>

#include <app/Global.h>

#include <app/CollapsableDialog.h>
#include <QVBoxLayout>

namespace app
{
	OpenGLScene::OpenGLScene()
	{
		_lblFPS = new QLabel();
		_lblFPS->setStyleSheet("QLabel { background-color : rgba(255,255,255,128); }");
//		auto item = addWidget(_lblFPS);
//		item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
//		item->setPos(5,5);
	}

	void OpenGLScene::init()
	{
		if(!_engine.initialize(Global::renderer))
		{
			throw std::exception();
		}

		_w = width();
		_h = height();

		Global::toolbar->addAction("Save State", this, SLOT(_onActSaveStateTriggered()));
		Global::toolbar->addAction("Restore State", this, SLOT(_onActRestoreStateTriggered()));
	}

	void OpenGLScene::addDialog(QDialog* dialog)
	{
		int spacing = 7;
#ifdef _WIN32
		spacing = 15;
#endif
		auto proxy = addWidget(dialog, Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
		proxy->setCacheMode(QGraphicsItem::DeviceCoordinateCache);
		proxy->setPos(_lastDialog? _lastDialog->scenePos().x() + _lastDialog->size().width() + spacing : spacing, height() - dialog->sizeHint().height()-5);
		if(proxy->pos().x() > width()-20)
		{
			proxy->setPos(spacing, proxy->pos().y());
			_lastDialog = nullptr;
		}
		else
		{
			_lastDialog = proxy;
		}
	}

	void OpenGLScene::addOverlayWidget(QWidget* widget, Corner corner)
	{
		auto proxy = addWidget(widget);
		proxy->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

		switch(corner)
		{
		case Corner_LowerRight:
			proxy->setPos(width() - widget->sizeHint().width() - 5, height() - widget->sizeHint().height() - 5);
			break;
		// TODO: other corners
		default:
			throw std::exception();
		}

		_overlayWidgets.push_back(proxy);
	}

	void OpenGLScene::refreshOverlayWidgets()
	{
		for(auto w : _overlayWidgets)
		{
			// TODO: switch based on widget corner
			w->setPos(width() - w->size().width() - 5, height() - w->size().height() - 5);
		}
	}

	void OpenGLScene::drawBackground(QPainter *painter, const QRectF &)
	{
		if(painter->paintEngine()->type() != QPaintEngine::OpenGL &&
		   painter->paintEngine()->type() != QPaintEngine::OpenGL2)
		{
			qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
			return;
		}

		if(_firstRun)
		{
			Global::init();
			_firstRun = false;
		}

		if(_w != width() || _h != height())
		{
			// TODO: only works for lower right corner widgets
			for(auto w : _overlayWidgets)
			{
				w->setPos(width() - w->size().width() - 5, height() - w->size().height() - 5);
			}
			_w = width();
			_h = height();
		}

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		_engine.resize_screen(width(), height());
		_engine.set_view(Global::camera->getView());
		_engine.render();

		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

		++_frameCount;
		auto dt = _timerFPS.sec();
		if(dt > 0.5)
		{
			double fps = _frameCount / dt;
			_lblFPS->setText(QString::number(dt * 1000.0 / _frameCount, 'f', 2) + " ms" + " | " + QString::number(fps, 'f', 2) + " fps");
			_lblFPS->adjustSize();
			_frameCount = 0;
			_timerFPS.restart();
		}

		QTimer::singleShot(0, this, SLOT(update()));
	}

	void OpenGLScene::keyPressEvent(QKeyEvent* e)
	{
		if(e->key() == Qt::Key_Escape)
		{
			qApp->quit();
			e->accept();
			return;
		}
		QGraphicsScene::keyPressEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit keyPressed(e);
	}

	void OpenGLScene::keyReleaseEvent(QKeyEvent* e)
	{
		QGraphicsScene::keyReleaseEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit keyReleased(e);
	}

	void OpenGLScene::mousePressEvent(QGraphicsSceneMouseEvent* e)
	{
		QGraphicsScene::mousePressEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit mousePressed(e);
	}

	void OpenGLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
	{
		QGraphicsScene::mouseReleaseEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit mouseReleased(e);
	}

	void OpenGLScene::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e)
	{
		QGraphicsScene::mouseDoubleClickEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit mouseDoubleClicked(e);
	}

	void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent* e)
	{
		QGraphicsScene::mouseMoveEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit mouseMoved(e);
	}

	void OpenGLScene::wheelEvent(QGraphicsSceneWheelEvent* e)
	{
		QGraphicsScene::wheelEvent(e);
		if(e->isAccepted())
		{
			return;
		}
		emit mouseWheeled(e);
	}

	void OpenGLScene::_onActSaveStateTriggered()
	{
		auto filename = QFileDialog::getSaveFileName(nullptr, QString("Save state"), QDir::currentPath() + "/settings.ini", QString("*.ini"));
		if(!filename.isEmpty())
		{
			app::Global::settings = new QSettings(filename, QSettings::IniFormat);
			Global::camera->saveState();
			Global::explodedview->saveState();
			Global::analysis->saveState();
			Global::selection->saveState();
		}
	}

	void OpenGLScene::_onActRestoreStateTriggered()
	{
		auto filename = QFileDialog::getOpenFileName(nullptr, QString("Restore state"), QDir::currentPath() + "/settings.ini", QString("*.ini"));
		if(!filename.isEmpty())
		{
			app::Global::settings = new QSettings(filename, QSettings::IniFormat);
			Global::camera->restoreState();
			Global::explodedview->restoreState();
			Global::analysis->restoreState();
			Global::selection->restoreState();
		}
	}
} // namespace app

