#include <app/CollapsableDialog.h>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QVBoxLayout>
#include <QStyle>
#include <QDebug>

namespace app
{
	CollapsableDialog::CollapsableDialog(const QString& title)
	{
		setWindowTitle(title);
		setWindowOpacity(0.8);
		setLayout(new QVBoxLayout());
		layout()->setSpacing(2);
		layout()->setMargin(1);
		_previousHeight = height();
	}

	void CollapsableDialog::mousePressEvent(QMouseEvent* e)
	{
		e->accept();
	}

	void CollapsableDialog::mouseReleaseEvent(QMouseEvent* e)
	{
		e->accept();
	}

	void CollapsableDialog::mouseMoveEvent(QMouseEvent* e)
	{
		e->accept();
	}

	void CollapsableDialog::mouseDoubleClickEvent(QMouseEvent* e)
	{
		if(e->localPos().y() <= 8)
		{
			if(maximumHeight() == 0)
			{
				setMaximumHeight(QWIDGETSIZE_MAX);
				resize(width(), _previousHeight);
			}
			else
			{
				_previousHeight = height();
				setMaximumHeight(0);
			}
		}
		e->accept();
	}

	void CollapsableDialog::wheelEvent(QWheelEvent* e)
	{
		e->accept();
	}
} // namespace app
