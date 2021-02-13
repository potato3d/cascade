#pragma once
#include <QGraphicsProxyWidget>
#include <QObject>
#include <QLabel>
#include <bl/bl.h>

namespace app
{
	class Overlay : public QObject
	{
		Q_OBJECT
	public:
		void init();

		void addWidget(QWidget* widget, const vec3& worldPos);
		void removeWidget(QWidget* widget);
		void setWidgetPosition(QWidget* widget, const vec3& worldPos);

	private slots:
		void _onBeforeRender();

	private:
		struct Data
		{
			QGraphicsProxyWidget* proxy;
			vec3 worldPos;
		};

		hash_map<QWidget*, Data> _widgets;
	};
}
