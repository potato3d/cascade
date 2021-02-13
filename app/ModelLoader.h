#pragma once
#include <QObject>

namespace app
{
	class ModelLoader : public QObject
	{
		Q_OBJECT
	public:
		void init();

	signals:
		void modelLoaded();
	};
}
