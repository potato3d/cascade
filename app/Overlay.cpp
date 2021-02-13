#include <app/Overlay.h>
#include <app/Global.h>
#include <QLabel>

namespace app
{
	void Overlay::init()
	{
		connect(Global::renderer, SIGNAL(beforeRender()), this, SLOT(_onBeforeRender()));
	}

	void Overlay::addWidget(QWidget* widget, const vec3& worldPos)
	{
		auto proxy = Global::glscene->addWidget(widget);
		proxy->setCacheMode(QGraphicsProxyWidget::DeviceCoordinateCache);
		_widgets.emplace(widget, Data{proxy, worldPos});
	}

	void Overlay::removeWidget(QWidget* widget)
	{
		auto itr = _widgets.find(widget);
		if(itr != _widgets.end())
		{
			_widgets.erase(itr);
		}
	}

	void Overlay::setWidgetPosition(QWidget* widget, const vec3& worldPos)
	{
		_widgets.at(widget).worldPos = worldPos;
	}

	void Overlay::_onBeforeRender()
	{
		auto w = Global::renderer->getScreenWidth();
		auto h = Global::renderer->getScreenHeight();

		auto projView = Global::renderer->getProjection().mul(Global::renderer->getView());

		for(auto& item : _widgets)
		{
			if(!item.first->isVisible())
			{
				continue;
			}
			float vw;
			auto ndcPos = projView.mul4x4(item.second.worldPos, vw);
			int screenX = -1;
			int screenY = -1;
			if(vw >= 0.0f)
			{
				ndcPos *= 1.0f / vw;
				screenX = (ndcPos.x * 0.5f + 0.5f) * w;
				screenY = (ndcPos.y * 0.5f + 0.5f) * h;
			}
			item.second.proxy->setPos(screenX, h - screenY); // invert y because qt's origin is at top
		}
	}
}
