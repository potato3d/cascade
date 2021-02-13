#pragma once
#include <app/AABB.h>
#include <QTimer>
#include <QObject>

class QKeyEvent;
class QGraphicsSceneMouseEvent;
class QGraphicsSceneWheelEvent;

namespace app
{
	class Camera : public QObject
	{
		Q_OBJECT
	public:
		void init();
		void saveState();
		void restoreState();

		void setFocus(const AABB& box);
		void setPivot(const AABB& box);
		void setLookAt(const vec3& eye, const vec3& center, const vec3& up);

		void reset();

		const vec3& getCenter();

		float* getView();

	private slots:
		void _onKeyPressed(QKeyEvent* e);
		void _onKeyReleased(QKeyEvent* e);
		void _onMousePressed(QGraphicsSceneMouseEvent* e);
		void _onMouseReleased(QGraphicsSceneMouseEvent* e);
		void _onMouseDoubleClicked(QGraphicsSceneMouseEvent* e);
		void _onMouseMoved(QGraphicsSceneMouseEvent* e);
		void _onMouseWheeled(QGraphicsSceneWheelEvent* e);

		void _onModelLoaded();

		void _onTimerStopped();

	private:
		void _moved();

		vec3 _eye = vec3(0.0f, 10.0f, 0.0f);
		vec3 _center = vec3::ZERO;
		vec3 _up = vec3::UNIT_Z;
		mat4 _view;
		bool _dirty = true;
		float _fb = 0.0f;
		float _lr = 0.0f;
		float _ud = 0.0f;

		unsigned int _pivotDrawable = 0;

		QTimer _timer;
	};
}
