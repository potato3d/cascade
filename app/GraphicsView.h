#pragma once
#include <QGraphicsView>

namespace app
{
	// ref: https://doc.qt.io/archives/qq/qq26-openglcanvas.html
    class GraphicsView : public QGraphicsView
    {
        Q_OBJECT
    public:
        GraphicsView();

    protected:
        virtual void resizeEvent(QResizeEvent* event);
    };
} // namespace app
