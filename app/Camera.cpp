#include <app/Camera.h>
#include <QKeyEvent>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneWheelEvent>
#include <app/Global.h>
#include <tess/tessellator.h>

namespace app
{
	static const float MOVE_SPEED = 0.1f;
	static const float ROTATION_SPEED = 0.01f;

	void Camera::init()
	{
		connect(&_timer, &QTimer::timeout, [this]{_onTimerStopped();});
		_timer.setInterval(500);

		auto colorID = Global::renderer->addColor(0,1,1);
		_pivotDrawable = Global::renderer->addExtraDrawable("cam", colorID, mat4::translation(_center), tess::tessellate_sphere(0.5f));
		Global::renderer->setDrawableTransparent(_pivotDrawable, true);

		connect(Global::glscene, SIGNAL(keyPressed(QKeyEvent*)), this, SLOT(_onKeyPressed(QKeyEvent*)));
		connect(Global::glscene, SIGNAL(keyReleased(QKeyEvent*)), this, SLOT(_onKeyReleased(QKeyEvent*)));
		connect(Global::glscene, SIGNAL(mousePressed(QGraphicsSceneMouseEvent*)), this, SLOT(_onMousePressed(QGraphicsSceneMouseEvent*)));
		connect(Global::glscene, SIGNAL(mouseReleased(QGraphicsSceneMouseEvent*)), this, SLOT(_onMouseReleased(QGraphicsSceneMouseEvent*)));
		connect(Global::glscene, SIGNAL(mouseDoubleClicked(QGraphicsSceneMouseEvent*)), this, SLOT(_onMouseDoubleClicked(QGraphicsSceneMouseEvent*)));
		connect(Global::glscene, SIGNAL(mouseMoved(QGraphicsSceneMouseEvent*)), this, SLOT(_onMouseMoved(QGraphicsSceneMouseEvent*)));
		connect(Global::glscene, SIGNAL(mouseWheeled(QGraphicsSceneWheelEvent*)), this, SLOT(_onMouseWheeled(QGraphicsSceneWheelEvent*)));

		connect(Global::loader, SIGNAL(modelLoaded()), this, SLOT(_onModelLoaded()));
	}

	void Camera::saveState()
	{
		QByteArray eye((char*)(_eye.data()), 3*sizeof(float));
		Global::settings->setValue("Camera/Eye", QVariant::fromValue(eye));
		QByteArray center((char*)(_center.data()), 3*sizeof(float));
		Global::settings->setValue("Camera/Center", QVariant::fromValue(center));
		QByteArray up((char*)(_up.data()), 3*sizeof(float));
		Global::settings->setValue("Camera/Up", QVariant::fromValue(up));
	}

	void Camera::restoreState()
	{
		vec3 eye = _eye, center = _center, up = _up;
		if(Global::settings->contains("Camera/Eye"))
		{
			QByteArray eyeb = Global::settings->value("Camera/Eye").toByteArray();
			eye = vec3((float*)(eyeb.data()));
		}
		if(Global::settings->contains("Camera/Center"))
		{
			QByteArray centerb = Global::settings->value("Camera/Center").toByteArray();
			center = vec3((float*)(centerb.data()));
		}
		if(Global::settings->contains("Camera/Up"))
		{
			QByteArray upb = Global::settings->value("Camera/Up").toByteArray();
			up = vec3((float*)(upb.data()));
		}
		setLookAt(eye, center, up);
	}

	void Camera::setFocus(const AABB& box)
	{
		auto radius = (box.max - box.min).length() * 0.5f;
		auto finalDistance = 1.1f * radius / math::tan(math::to_radians(60.0f) * 0.5f); // assuming fov = 60 deg
		finalDistance = math::max(finalDistance, 1.1f);
		_center = (box.min + box.max) * 0.5f;
		auto dirToCam = (_eye - _center).normalized();
		_eye = _center + dirToCam * finalDistance;
		_moved();
	}

	void Camera::setPivot(const AABB& box)
	{
		_center = (box.min + box.max) * 0.5f;
		_moved();
	}

	void Camera::setLookAt(const vec3& eye, const vec3& center, const vec3& up)
	{
		_eye = eye;
		_center = center;
		_up = up;
		_moved();
	}

	void Camera::reset()
	{
		// TODO: use this when the models don't have geometries flying all over the place
		setFocus(Global::renderer->getSceneBounds());

//		// small
//		auto eye = vec3(5306.69, 2537.95, 54.7586); auto center = vec3(5246.83, 2579.81, 28.1845); auto up = vec3(0.0f, 0.0f, 1.0f);
//		// medium
//		auto eye = vec3(5609.45, 2945.72, 70.6695); auto center = vec3(5608.86, 2946.46, 70.359); auto up = vec3(0.0f, 0.0f, 1.0f);
//		// large
//		auto eye = vec3(4895.23, 2842.66, 84.3734); auto center = vec3(4895.87, 2843.34, 84.0221); auto up = vec3(0.0f, 0.0f, 1.0f);
//		// massive
//		auto eye = vec3(4895.23, 2842.66, 84.3734); auto center = vec3(4895.87, 2843.34, 84.0221); auto up = vec3(0.0f, 0.0f, 1.0f);
//		setLookAt(eye, center, up);
	}

	const vec3& Camera::getCenter()
	{
		return _center;
	}

	float* Camera::getView()
	{
		if(_dirty)
		{
			_dirty = false;
			_view = mat4::look_at(_eye, _center, _up);
		}
		return _view.data();
	}

	// ----------------------------------------------------------------------------------------------------------

	void Camera::_onKeyPressed(QKeyEvent* e)
	{
		if(e->isAutoRepeat())
		{
			e->ignore();
			return;
		}

		switch(e->key())
		{
		case Qt::Key_Space:
			reset();
			break;
		case Qt::Key_W:
			++_fb;
			break;
		case Qt::Key_A:
			--_lr;
			break;
		case Qt::Key_S:
			--_fb;
			break;
		case Qt::Key_D:
			++_lr;
			break;
		case Qt::Key_F:
			--_ud;
			break;
		case Qt::Key_R:
			++_ud;
			break;
		default:
			e->ignore();
			return;
		}

		e->accept();
		_moved();
	}

	void Camera::_onKeyReleased(QKeyEvent* e)
	{
		if(e->isAutoRepeat())
		{
			e->ignore();
			return;
		}

		switch(e->key())
		{
		case Qt::Key_W:
			--_fb;
			break;
		case Qt::Key_A:
			++_lr;
			break;
		case Qt::Key_S:
			++_fb;
			break;
		case Qt::Key_D:
			--_lr;
			break;
		case Qt::Key_F:
			++_ud;
			break;
		case Qt::Key_R:
			--_ud;
			break;
		default:
			e->ignore();
			return;
		}

		e->accept();
		_moved();
	}

	void Camera::_onMousePressed(QGraphicsSceneMouseEvent* e)
	{
		e->ignore();
	}

	void Camera::_onMouseReleased(QGraphicsSceneMouseEvent* e)
	{
		e->ignore();
	}

	void Camera::_onMouseDoubleClicked(QGraphicsSceneMouseEvent* e)
	{
		e->ignore();
	}

	void Camera::_onMouseMoved(QGraphicsSceneMouseEvent* e)
	{
		if(e->buttons() & Qt::LeftButton)
		{
			auto delta = (e->scenePos() - e->lastScenePos()) * ROTATION_SPEED;
			auto right = (_center - _eye).cross(_up).normalized();
			auto newEye = quat(-delta.y(), right).mul(quat(-delta.x(), _up)).mul(_eye - _center) + _center;
			if(math::abs((_center - newEye).normalized().dot(_up)) > 0.999f)
			{
				e->ignore();
				return;
			}
			_eye = newEye;
		}
		else if(e->buttons() & Qt::RightButton)
		{
			auto deltaY = e->scenePos().y() - e->lastScenePos().y();
			auto move = (_center - _eye).normalized() * deltaY * MOVE_SPEED;
			_eye += move;
			_center += move;
		}
		else if(e->buttons() & Qt::MidButton)
		{
			auto delta = (e->scenePos() - e->lastScenePos()) * MOVE_SPEED;
			auto right = (_center - _eye).cross(_up).normalized();
			auto move = right * -delta.x() +  _up * delta.y();
			_eye += move;
			_center += move;
		}
		else
		{
			e->ignore();
			return;
		}

		e->accept();
		_moved();
	}

	void Camera::_onMouseWheeled(QGraphicsSceneWheelEvent* e)
	{
		auto dir = _center - _eye;
		auto distance = dir.length();
		dir = dir.normalized() * (e->delta() > 0? 1.0f : -1.0f);
		auto newEye = _eye + dir * MOVE_SPEED * distance;
		if((_center - newEye).length_squared() > 1.0f)
		{
			_eye = newEye;
			e->accept();
			_moved();
		}
		else
		{
			e->ignore();
		}
	}

	void Camera::_onModelLoaded()
	{
		reset();
	}

	void Camera::_onTimerStopped()
	{
		Global::renderer->setDrawableVisible(_pivotDrawable, false);
		_timer.stop();
	}

	void Camera::_moved()
	{
		_dirty = true;
		Global::renderer->setDrawableVisible(_pivotDrawable, true);
		Global::renderer->setDrawableTransform(_pivotDrawable, mat4::translation(_center));
		_timer.start();
	}
}
