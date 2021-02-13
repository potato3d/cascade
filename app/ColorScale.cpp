#include <app/ColorScale.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

namespace app
{
	QWidget* createTickMark(int w)
	{
		QFrame* line = new QFrame();
		line->setMinimumHeight(1);
		line->setMaximumHeight(1);
		line->setMinimumWidth(w*1.25f);
		line->setMaximumWidth(w*1.25f);
		line->setStyleSheet("QFrame{background-color: black;}");
		return line;
	}

	QLayout* pushRight(QWidget* widget, int w)
	{
		QHBoxLayout* l = new QHBoxLayout();
		l->addSpacerItem(new QSpacerItem(w*0.25f, 1));
		l->addWidget(widget);
		return l;
	}

	// ------------------------------------------------------------------------------------------------------------

	ColorQuad::ColorQuad(int idx)
	{
		_idx = idx;
	}

	void ColorQuad::mouseDoubleClickEvent(QMouseEvent* e)
	{
		emit mouseDoubleClicked(_idx);
	}

	// ------------------------------------------------------------------------------------------------------------

	void ColorScale::init(int w, int h, int numColors)
	{
		_title = new QLabel("title");
		_title->setAlignment(Qt::AlignCenter);
		_title->setStyleSheet("QLabel{font: 10pt; background-color:rgba(255,255,255,128);}");

		for(int i = 0; i < numColors; ++i)
		{
			auto f = new ColorQuad(i);
			f->setMinimumHeight(h);
			f->setMaximumHeight(h);
			f->setMinimumWidth(w);
			f->setMaximumWidth(w);
			connect(f, SIGNAL(mouseDoubleClicked(int)), this, SLOT(_onQuadMouseDoubleClicked(int)));
			_quads.push_back(f);
		}

		auto vl1 = new QVBoxLayout();
		vl1->setSpacing(0);
		vl1->setSizeConstraint(QLayout::SetFixedSize);
		vl1->addSpacerItem(new QSpacerItem(1, 10));
		vl1->addWidget(createTickMark(w));
		for(int i = 0; i < numColors; ++i)
		{
			vl1->addLayout(pushRight(_quads[i], w));
			vl1->addWidget(createTickMark(w));
		}

		auto vl2 = new QVBoxLayout();
		vl2->setSizeConstraint(QLayout::SetFixedSize);
		vl2->setMargin(0);
		vl2->setSpacing(h*0.5);
		for(int i = 0; i < numColors+1; ++i)
		{
			auto l = new QLabel("Feb 19 1985");
			l->setStyleSheet("QLabel{font: 9pt; background-color:rgba(255,255,255,128);"
							 "border: 2px solid lightgrey;border-radius: 6px;}");
			vl2->addWidget(l);
			_labels.push_back(l);
		}

		auto hl = new QHBoxLayout();
		hl->setSizeConstraint(QLayout::SetFixedSize);
		hl->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding));
		hl->addLayout(vl2);
		hl->addLayout(vl1);
		hl->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding));

		auto thl = new QHBoxLayout();
		thl->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding));
		thl->addWidget(_title);
		thl->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::MinimumExpanding));

		auto vll = new QVBoxLayout();
		vll->setSizeConstraint(QLayout::SetFixedSize);
		vll->addLayout(thl);
		vll->addLayout(hl);

		_widget = new QFrame();
		_widget->setLayout(vll);
		_widget->setStyleSheet("background-color:transparent");
	}

	unsigned int ColorScale::getNumColors()
	{
		return _quads.size();
	}

	void ColorScale::setColor(int idx, const QColor& color)
	{
		_quads.at(idx)->setStyleSheet(QString("QFrame{background-color: rgb(%1,%2,%3);").arg(color.red()).arg(color.green()).arg(color.blue()) +
									  "border-left: 1px solid black;"
									  "border-right: 1px solid black;}");
	}

	void ColorScale::setLabel(int idx, const QString& text)
	{
		_labels.at(idx)->setText(text);
		_labels.at(idx)->adjustSize();
	}

	void ColorScale::setTitle(const QString& title)
	{
		_title->setText(title);
		_title->adjustSize();
	}

	QWidget* ColorScale::getWidget()
	{
		return _widget;
	}

	void ColorScale::_onQuadMouseDoubleClicked(int idx)
	{
		emit colorClicked(idx);
	}
}
