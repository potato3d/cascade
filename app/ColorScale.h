#pragma once
#include <QFrame>
#include <QColor>
#include <QLabel>
#include <bl/bl.h>

namespace app
{
	class ColorQuad : public QFrame
	{
		Q_OBJECT
	public:
		ColorQuad(int idx);

	signals:
		void mouseDoubleClicked(int idx);

	private slots:
		virtual void mouseDoubleClickEvent(QMouseEvent* e);

	private:
		int _idx = 0;
	};

	class ColorScale : public QObject
	{
		Q_OBJECT
	public:
		void init(int w, int h, int numColors);
		unsigned int getNumColors();
		void setColor(int idx, const QColor& color);
		void setLabel(int idx, const QString& text);
		void setTitle(const QString& title);
		QWidget* getWidget();

	signals:
		void colorClicked(int idx);

	private slots:
		void _onQuadMouseDoubleClicked(int idx);

	private:
		QLabel* _title;
		vector<ColorQuad*> _quads;
		vector<QLabel*> _labels;
		QFrame* _widget = nullptr;
	};
}
