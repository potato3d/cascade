#pragma once
#include <QGraphicsScene>
#include <QResizeEvent>
#include <app/Renderer.h>
#include <app/Camera.h>
#include <glb/engine.h>
#include <bl/util/timer.h>

class QLabel;

namespace app
{
	// ref: https://doc.qt.io/archives/qq/qq26-openglcanvas.html
	class OpenGLScene : public QGraphicsScene
	{
		Q_OBJECT
	public:
		enum Corner
		{
			Corner_LowerLeft,
			Corner_LowerRight,
			Corner_TopLeft,
			Corner_TopRight
		};

		OpenGLScene();
		void init();
		void addDialog(QDialog* dialog);
		void addOverlayWidget(QWidget* widget, Corner corner);
		void refreshOverlayWidgets();
		void drawBackground(QPainter *painter, const QRectF &rect);

	signals:
		void keyPressed(QKeyEvent* e);
		void keyReleased(QKeyEvent* e);
		void mousePressed(QGraphicsSceneMouseEvent* e);
		void mouseReleased(QGraphicsSceneMouseEvent* e);
		void mouseDoubleClicked(QGraphicsSceneMouseEvent* e);
		void mouseMoved(QGraphicsSceneMouseEvent* e);
		void mouseWheeled(QGraphicsSceneWheelEvent* e);

	protected:
		virtual void keyPressEvent(QKeyEvent* e);
		virtual void keyReleaseEvent(QKeyEvent* e);
		virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* e);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* e);
		virtual void wheelEvent(QGraphicsSceneWheelEvent* e);

	private slots:
		void _onActSaveStateTriggered();
		void _onActRestoreStateTriggered();

	private:
		bool _firstRun = true;
		glb::engine _engine;
		bl::timer _timerFPS;
		int _frameCount = 0;
		QLabel* _lblFPS = nullptr;
		QGraphicsProxyWidget* _lastDialog = nullptr;
		vector<QGraphicsProxyWidget*> _overlayWidgets;
		int _w;
		int _h;
	};
} // namespace app
