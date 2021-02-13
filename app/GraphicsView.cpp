#include <app/GraphicsView.h>
#include <QResizeEvent>

namespace app
{
	GraphicsView::GraphicsView()
	{
		setWindowTitle(tr("Schedule3D"));
	}

	void GraphicsView::resizeEvent(QResizeEvent *event)
	{
		if(scene())
		{
			scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
		}
		QGraphicsView::resizeEvent(event);
	}
} // namespace app
