#pragma once
#include <QDialog>

class QFrame;

namespace app
{
	class CollapsableDialog : public QDialog
	{
		Q_OBJECT

	public:
		CollapsableDialog(const QString& title);

		void addWidget(QWidget* widget);

	protected:
		virtual void mousePressEvent(QMouseEvent* e);
		virtual void mouseReleaseEvent(QMouseEvent* e);
		virtual void mouseMoveEvent(QMouseEvent* e);
		virtual void mouseDoubleClickEvent(QMouseEvent* e);
		virtual void wheelEvent(QWheelEvent* e);

	private:
		QFrame* _frame;
		int _previousHeight;
	};
} // namespace app
