#include <app/ExplodedView.h>
#include <app/Global.h>
#include <QLayout>

#include <tess/tessellator.h>

namespace app
{
	static const unsigned int MAX_HORIZONAL_BAR_COUNT = 365;
	static const QString FACE_POS_X = "Positive X";
	static const QString FACE_NEG_X = "Negative X";
	static const QString FACE_POS_Y = "Positive Y";
	static const QString FACE_NEG_Y = "Negative Y";

	static const QString LABEL_DAILY = "Daily";
	static const QString LABEL_WEEKLY = "Weekly";
	static const QString LABEL_MONTHLY = "Monthly";
	static const QString LABEL_YEARLY = "Yearly";

	static const vector<QString> START_FINISH_LIST = {"Baseline Start",
												"Baseline Finish",
												"Actual Start",
												"Actual Finish"};

	static const QString DATE_FORMAT = "MMM dd yyyy";

	bool collide(const AABB& a, const AABB& b)
	{
		return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
				 (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
				 (a.min.z <= b.max.z && a.max.z >= b.min.z);
	}

	void ExplodedView::init()
	{
		_actSchedule = Global::toolbar->addAction("Schedule", this, SLOT(_onScheduleActionTriggered(bool)));
		_actSchedule->setCheckable(true);

		// create 3D plot
		auto backColorID = Global::renderer->addColor(0.7f, 0.7f, 0.7f);
		auto floorColorID = Global::renderer->addColor(0.7f, 0.7f, 0.7f);
		auto axisColorID = Global::renderer->addColor(0.7f, 0.7f, 0.7f);
		_lineColorID = Global::renderer->addColor(0.0f, 0.9f, 0.9f);

		_freeTaskBoxColorID = Global::renderer->addColor(0.0f, 0.0f, 1.0f);
		_collidedTaskBoxColorID = Global::renderer->addColor(1.0f, 0.0f, 0.0f);

		// background (vertical quad)
		_plotBackground = Global::renderer->addExtraDrawable("plotBackground", backColorID, tess::tessellate_box(vec3(1.0f)));
		Global::renderer->setDrawableTransparent(_plotBackground, true);
		Global::renderer->setDrawableVisible(_plotBackground, false);

		// floor (horizontal quad)
		_plotFloor = Global::renderer->addExtraDrawable("plotFloor", floorColorID, tess::tessellate_box(vec3(1.0f)));
		Global::renderer->setDrawableTransparent(_plotFloor, true);
		Global::renderer->setDrawableVisible(_plotFloor, false);

		// horizontal bars (cylinders)
		for(unsigned int i = 0; i < MAX_HORIZONAL_BAR_COUNT; ++i)
		{
			unsigned int drawable = Global::renderer->addLineDrawable(axisColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(drawable, false);
			_plotHorizontalBars.push_back(drawable);

			auto label = new QLabel();
			label->setVisible(false);
			if(i == 0)
			{
				label->setStyleSheet("QLabel{font: 9pt; background-color:rgba(50, 200, 0, 50);"
									 "border: 2px solid rgba(25, 100, 0, 50);border-radius: 6px;}");
			}
			else
			{
				label->setStyleSheet(Global::getLabelStyle());
			}
			Global::overlay->addWidget(label, vec3::ZERO);
			_plotHorizontalLabels.push_back(label);
		}

		// other visual elements associated with drawables
		for(unsigned int i = 0; i < 1e3; ++i)
		{
			unsigned int verticalLineDrawable = Global::renderer->addLineDrawable(_lineColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(verticalLineDrawable, false);
			Global::renderer->setLineDrawableStipple(verticalLineDrawable, true);
			_plotVerticalLines.push_back(verticalLineDrawable);

			unsigned int verticalSphereDrawable = Global::renderer->addExtraDrawable("plotSphere", _lineColorID, tess::tessellate_sphere(0.75f));
//			Global::renderer->setDrawableTransparent(verticalSphereDrawable, true);
			Global::renderer->setDrawableVisible(verticalSphereDrawable, false);
			_plotVerticalSpheres.push_back(verticalSphereDrawable);

			auto label = new QLabel();
			label->setStyleSheet(Global::getLabelStyle());
			label->setVisible(false);
			Global::overlay->addWidget(label, vec3::ZERO);
			_plotSelectionDateLabels.push_back(label);

			unsigned int lineDrawable = Global::renderer->addLineDrawable(_lineColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(lineDrawable, false);
			_plotSelectionDateLines.push_back(lineDrawable);

			unsigned int dateDrawable = Global::renderer->addExtraDrawable("plotSphere", _lineColorID, tess::tessellate_sphere(0.5f));
			Global::renderer->setDrawableVisible(dateDrawable, false);
			_plotSelectionDateSpheres.push_back(dateDrawable);

			unsigned int taskBoxDrawable = Global::renderer->addExtraDrawable("taskBox", _freeTaskBoxColorID, tess::tessellate_box(vec3{1.0f}));
			Global::renderer->setDrawableVisible(taskBoxDrawable, false);
			Global::renderer->setDrawableTransparent(taskBoxDrawable, true);
			_taskBoxDrawables.push_back(taskBoxDrawable);
		}

		// need to create widgets here because they are used to store our state
		_createWidgets();

		_animationTimer.setInterval(500);
		connect(&_animationTimer, SIGNAL(timeout()), this, SLOT(_onAnimationTimeChanged()));

		Global::selection->addSelectionCallback("Schedule Date", [=](bool checked)
		{
			_showSelectionDates = checked;
			if(!_ck3DPlot->isChecked())
			{
				return;
			}
			_rebuildPlot();
		}, _showSelectionDates);

		Global::selection->addSelectionCallback("Animation Path", [=](bool checked)
		{
			_showVerticalLines = checked;
			if(!_ck3DPlot->isChecked())
			{
				return;
			}
			_rebuildPlot();
		}, _showVerticalLines);

		Global::selection->addSelectionCallback("Task Boxes", [=](bool checked)
		{
			_showTaskBoxes = checked;
			if(!_ck3DPlot->isChecked())
			{
				return;
			}
			_rebuildPlot();
		}, _showTaskBoxes);

		connect(Global::selection, SIGNAL(selectionChanged()), this, SLOT(_onSelectionChanged()));
	}

	void ExplodedView::saveState()
	{
		Global::settings->setValue(QString::fromStdString("ExplodedView/CheckPlot"), QVariant::fromValue(_ck3DPlot->isChecked()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/CheckHorizontalPlane"), QVariant::fromValue(_ckHorizontalPlane->isChecked()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/CheckHighlightOutOfRange"), QVariant::fromValue(_ckHighlightOutOfRange->isChecked()));
		if(_lastHidOrShown)
		{
			Global::settings->setValue(QString::fromStdString("ExplodedView/HideOutOfRange"), QVariant::fromValue(_lastHidOutOfRange));
		}
		Global::settings->setValue(QString::fromStdString("ExplodedView/CompareSchedules"), QVariant::fromValue(_ckCompareSchedules->isChecked()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/VerticalScale"), QVariant::fromValue(_dsbVerticalScale->value()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/XOffset"), QVariant::fromValue(_dsbXOffset->value()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/YOffset"), QVariant::fromValue(_dsbYOffset->value()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/ZOffset"), QVariant::fromValue(_dsbZOffset->value()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/VerticalLabels"), QVariant::fromValue(_cbVerticalLabels->currentIndex()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/VerticalCount"), QVariant::fromValue(_sbVerticalCount->value()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/BoundFace"), QVariant::fromValue(_cbBoundFace->currentIndex()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/TaskName"), QVariant::fromValue(_cbTaskName->currentIndex()));
		Global::settings->setValue(QString::fromStdString("ExplodedView/StartFinish"), QVariant::fromValue(_cbStartFinish->currentIndex()));
		Global::settings->setValue(QString::fromStdString("Animation/CurrDate"), QVariant::fromValue(_slCurrDate->value()));
		Global::settings->setValue(QString::fromStdString("Animation/Speed"), QVariant::fromValue(_dsbSpeed->value()));
	}

	void ExplodedView::restoreState()
	{
		if(Global::settings->contains("ExplodedView/CheckPlot"))
		{
			_ck3DPlot->setChecked(Global::settings->value("ExplodedView/CheckPlot").toBool());
		}
		if(Global::settings->contains("ExplodedView/CheckHorizontalPlane"))
		{
			_ckHorizontalPlane->setChecked(Global::settings->value("ExplodedView/CheckHorizontalPlane").toBool());
		}
		if(Global::settings->contains("ExplodedView/CheckHighlightOutOfRange"))
		{
			_ckHighlightOutOfRange->setChecked(Global::settings->value("ExplodedView/CheckHighlightOutOfRange").toBool());
		}
		if(Global::settings->contains("ExplodedView/HideOutOfRange"))
		{
			auto value = Global::settings->value("ExplodedView/HideOutOfRange").toBool();
			if(value)
			{
				_pbHideOutOfRange->click();
			}
			else
			{
				_pbShowOutOfRange->click();
			}
		}
		if(Global::settings->contains("ExplodedView/CompareSchedules"))
		{
			_ckCompareSchedules->setChecked(Global::settings->value("ExplodedView/CompareSchedules").toBool());
		}
		if(Global::settings->contains("ExplodedView/VerticalScale"))
		{
			_dsbVerticalScale->setValue(Global::settings->value("ExplodedView/VerticalScale").toDouble());
		}
		if(Global::settings->contains("ExplodedView/XOffset"))
		{
			_dsbXOffset->setValue(Global::settings->value("ExplodedView/XOffset").toDouble());
		}
		if(Global::settings->contains("ExplodedView/YOffset"))
		{
			_dsbYOffset->setValue(Global::settings->value("ExplodedView/YOffset").toDouble());
		}
		if(Global::settings->contains("ExplodedView/ZOffset"))
		{
			_dsbZOffset->setValue(Global::settings->value("ExplodedView/ZOffset").toDouble());
		}
		if(Global::settings->contains("ExplodedView/VerticalLabels"))
		{
			_cbVerticalLabels->setCurrentIndex(Global::settings->value("ExplodedView/VerticalLabels").toInt());
		}
		if(Global::settings->contains("ExplodedView/VerticalCount"))
		{
			_sbVerticalCount->setValue(Global::settings->value("ExplodedView/VerticalCount").toInt());
		}
		if(Global::settings->contains("ExplodedView/BoundFace"))
		{
			_cbBoundFace->setCurrentIndex(Global::settings->value("ExplodedView/BoundFace").toInt());
		}
		if(Global::settings->contains("ExplodedView/TaskName"))
		{
			_cbTaskName->setCurrentIndex(Global::settings->value("ExplodedView/TaskName").toInt());
		}
		if(Global::settings->contains("ExplodedView/StartFinish"))
		{
			_cbStartFinish->setCurrentIndex(Global::settings->value("ExplodedView/StartFinish").toInt());
		}
		if(Global::settings->contains("Animation/CurrDate"))
		{
			_slCurrDate->setValue(Global::settings->value("Animation/CurrDate").toInt());
		}
		if(Global::settings->contains("Animation/Speed"))
		{
			_dsbSpeed->setValue(Global::settings->value("Animation/Speed").toDouble());
		}
	}

	bool ExplodedView::isActive()
	{
		return _ck3DPlot->isChecked();
	}

	QString ExplodedView::getActiveTaskName()
	{
		return _cbTaskName->currentText();
	}

	void ExplodedView::_onScheduleActionTriggered(bool checked)
	{
		if(!_dlgSchedule)
		{
			_dlgSchedule = new CollapsableDialog("Schedule");
			_dlgSchedule->layout()->addWidget(_ck3DPlot);
			_dlgSchedule->layout()->addWidget(_ckHorizontalPlane);
			_dlgSchedule->layout()->addWidget(Global::createSeparator());
			_dlgSchedule->layout()->addWidget(_ckHighlightOutOfRange);
			auto hll = new QHBoxLayout();
			hll->addWidget(_pbHideOutOfRange);
			hll->addWidget(_pbShowOutOfRange);
			qobject_cast<QVBoxLayout*>(_dlgSchedule->layout())->addLayout(hll);
			_dlgSchedule->layout()->addWidget(Global::createSeparator());
			_dlgSchedule->layout()->addWidget(_ckCompareSchedules);
			_dlgSchedule->layout()->addWidget(_dsbVerticalScale);
			auto hl = new QHBoxLayout();
			hl->addWidget(_dsbXOffset);
			hl->addWidget(_dsbYOffset);
			hl->addWidget(_dsbZOffset);
			qobject_cast<QVBoxLayout*>(_dlgSchedule->layout())->addLayout(hl);
			_dlgSchedule->layout()->addWidget(_sbVerticalCount);
			_dlgSchedule->layout()->addWidget(_cbVerticalLabels);
			_dlgSchedule->layout()->addWidget(_cbBoundFace);
			_dlgSchedule->layout()->addWidget(_cbTaskName);
			_dlgSchedule->layout()->addWidget(_cbStartFinish);

			_dlgSchedule->layout()->addWidget(Global::createSeparator());

			auto hlDateSpeed = new QHBoxLayout();
			hlDateSpeed->addWidget(_deCurrDate);
			hlDateSpeed->addWidget(_dsbSpeed);

			auto hlControls = new QHBoxLayout();
			hlControls->addWidget(_pbFullBack);
			hlControls->addWidget(_pbBack);
			hlControls->addWidget(_pbPlayPause);
			hlControls->addWidget(_pbStop);
			hlControls->addWidget(_pbFwd);
			hlControls->addWidget(_pbFullFwd);

			_dlgSchedule->layout()->addWidget(_slCurrDate);
			qobject_cast<QVBoxLayout*>(_dlgSchedule->layout())->addLayout(hlDateSpeed);
			qobject_cast<QVBoxLayout*>(_dlgSchedule->layout())->addLayout(hlControls);

			Global::glscene->addDialog(_dlgSchedule);
		}
		_dlgSchedule->setVisible(checked);
	}

	void ExplodedView::_onAnimationTimeChanged()
	{
		_deCurrDate->setDate(_deCurrDate->date().addDays(1));
		if(_deCurrDate->date() == Global::schedule1->getLastDate())
		{
			_pbPlayPause->setChecked(false);
		}
	}

	void ExplodedView::_onSelectionChanged()
	{
		_rebuildPlot();
	}

	void ExplodedView::_createWidgets()
	{
		// -----------------------------------------------------------------------------------
		// schedule dialog
		// -----------------------------------------------------------------------------------

		// checkbox to toggle vertical plot
		_ck3DPlot = new QCheckBox("3D Plot");

		// checkbox to toggle horizontal plane
		_ckHorizontalPlane = new QCheckBox("Horizontal plane");
		_ckHorizontalPlane->setEnabled(false);

		// checkbox to choose whether to turn invisible drawables that fall out of plot range
		_ckHighlightOutOfRange = new QCheckBox("Highlight out of range");
		_ckHighlightOutOfRange->setEnabled(false);

		_pbHideOutOfRange = new QPushButton("Hide");
		_pbHideOutOfRange->setEnabled(false);

		_pbShowOutOfRange = new QPushButton("Show");
		_pbShowOutOfRange->setEnabled(false);

		// checkbox to compare two schedules
		_ckCompareSchedules = new QCheckBox("Compare with other schedule");
		_ckCompareSchedules->setEnabled(false);

		// slider to choose vertical scale
		_dsbVerticalScale = new QDoubleSpinBox();
		_dsbVerticalScale->setRange(1e-2, 1e2);
		_dsbVerticalScale->setSingleStep(0.1);
		_dsbVerticalScale->setDecimals(2);
		_dsbVerticalScale->setValue(1.0);
		_dsbVerticalScale->setEnabled(false);

		// spin box to choose offsets
		_dsbXOffset = new QDoubleSpinBox();
		_dsbXOffset->setRange(-1e6, 1e6);
		_dsbXOffset->setValue(0.0f);
		_dsbXOffset->setMaximumWidth(70);
		_dsbXOffset->setEnabled(false);

		_dsbYOffset = new QDoubleSpinBox();
		_dsbYOffset->setRange(-1e6, 1e6);
		_dsbYOffset->setValue(0.0f);
		_dsbYOffset->setMaximumWidth(70);
		_dsbYOffset->setEnabled(false);

		_dsbZOffset = new QDoubleSpinBox();
		_dsbZOffset->setRange(-1e6, 1e6);
		_dsbZOffset->setValue(0.0f);
		_dsbZOffset->setMaximumWidth(70);
		_dsbZOffset->setEnabled(false);

		// combo to choose spacing between horizontal bars and labels
		_cbVerticalLabels = new QComboBox();
		_cbVerticalLabels->addItem(LABEL_DAILY);
		_cbVerticalLabels->addItem(LABEL_WEEKLY);
		_cbVerticalLabels->addItem(LABEL_MONTHLY);
		_cbVerticalLabels->addItem(LABEL_YEARLY);
		_cbVerticalLabels->setEnabled(false);

		// spin box to choose how many horizontal bars and labels
		_sbVerticalCount = new QSpinBox();
		_sbVerticalCount->setRange(1, MAX_HORIZONAL_BAR_COUNT);
		_sbVerticalCount->setValue(10);
		_sbVerticalCount->setEnabled(false);

		// combo to choose scene bounding box face
		_cbBoundFace = new QComboBox();
		_cbBoundFace->addItem(FACE_POS_X);
		_cbBoundFace->addItem(FACE_POS_Y);
		_cbBoundFace->addItem(FACE_NEG_X);
		_cbBoundFace->addItem(FACE_NEG_Y);
		_cbBoundFace->setEnabled(false);

		// combo to choose task name
		_cbTaskName = new QComboBox();
		auto uniqueTaskNames = Global::schedule1->getUniqueTaskNames();
		for(const auto& n : uniqueTaskNames)
		{
			_cbTaskName->addItem(n);
		}
		_cbTaskName->setEnabled(false);

		// radio to choose start/finish
		_cbStartFinish = new QComboBox();

		for(unsigned int i = 0; i < START_FINISH_LIST.size(); ++i)
		{
			_cbStartFinish->addItem(START_FINISH_LIST.at(i));
		}
		_cbStartFinish->setCurrentIndex(1);
		_cbStartFinish->setEnabled(false);

		// connect actions to handle changes in all above widgets
		connect(_ck3DPlot, &QCheckBox::toggled, [=](bool checked)
		{
			_ckHorizontalPlane->setEnabled(checked);
			_ckHighlightOutOfRange->setEnabled(checked);
			_pbHideOutOfRange->setEnabled(checked);
			_pbShowOutOfRange->setEnabled(checked);
			_ckCompareSchedules->setEnabled(checked);
			_dsbVerticalScale->setEnabled(checked);
			_dsbXOffset->setEnabled(checked);
			_dsbYOffset->setEnabled(checked);
			_dsbZOffset->setEnabled(checked);
			_sbVerticalCount->setEnabled(checked);
			_cbVerticalLabels->setEnabled(checked);
			_cbBoundFace->setEnabled(checked);
			_cbTaskName->setEnabled(checked);
			_cbStartFinish->setEnabled(checked);

			_slCurrDate->setEnabled(checked);
			_deCurrDate->setEnabled(checked);
			_dsbSpeed->setEnabled(checked);

			_pbFullBack->setEnabled(checked);
			_pbBack->setEnabled(checked);
			_pbPlayPause->setEnabled(checked);
			_pbStop->setEnabled(checked);
			_pbFwd->setEnabled(checked);
			_pbFullFwd->setEnabled(checked);

			if(!checked)
			{
				_pbPlayPause->setChecked(false);
			}

			_rebuildPlot();
		});

		connect(_ckHorizontalPlane, &QCheckBox::toggled, [=](bool checked)
		{
			_rebuildPlot();
		});

		connect(_ckHighlightOutOfRange, &QCheckBox::toggled, [=](bool checked)
		{
			_rebuildPlot();
		});

		connect(_pbHideOutOfRange, &QPushButton::clicked, [=]
		{
			_hideOutOfRange = true;
			_rebuildPlot();
			_hideOutOfRange = false;
			_lastHidOrShown = true;
			_lastHidOutOfRange = true;
		});

		connect(_pbShowOutOfRange, &QPushButton::clicked, [=]
		{
			_showOutOfRange = true;
			_rebuildPlot();
			_showOutOfRange = false;
			_lastHidOrShown = true;
			_lastHidOutOfRange = false;
		});

		connect(_ckCompareSchedules, &QCheckBox::toggled, [=](bool checked)
		{
			_rebuildPlot();
		});

		connect(_dsbVerticalScale, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value)
		{
			_rebuildPlot();
		});

		connect(_dsbXOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value)
		{
			_rebuildPlot();
		});

		connect(_dsbYOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value)
		{
			_rebuildPlot();
		});

		connect(_dsbZOffset, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value)
		{
			_rebuildPlot();
		});

		connect(_cbVerticalLabels, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int value)
		{
			_rebuildPlot();
		});

		connect(_sbVerticalCount, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), [=](int value)
		{
			_rebuildPlot();
		});

		connect(_cbBoundFace, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int value)
		{
			_rebuildPlot();
		});

		connect(_cbTaskName, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int value)
		{
			_rebuildPlot();
		});

		connect(_cbStartFinish, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int value)
		{
			_rebuildPlot();
		});

		// -----------------------------------------------------------------------------------
		// animation dialog
		// -----------------------------------------------------------------------------------

		// slider to change current date
		_slCurrDate = new QSlider();
		_slCurrDate->setOrientation(Qt::Horizontal);
		_slCurrDate->setRange(0, Global::schedule1->getFirstDate().daysTo(Global::schedule1->getLastDate()));
		_slCurrDate->setTickInterval(1);
		_slCurrDate->setValue(0);
		_slCurrDate->setEnabled(false);

		// edits to change current date (day/month/year)
		_deCurrDate = new QDateEdit();
		_deCurrDate->setDateRange(Global::schedule1->getFirstDate(), Global::schedule1->getLastDate());
		_deCurrDate->setDate(Global::schedule1->getFirstDate());
		_deCurrDate->setDisplayFormat("MMM dd yyyy");
		_deCurrDate->setEnabled(false);

		// spinbox to choose playback speed
		_dsbSpeed = new QDoubleSpinBox();
		_dsbSpeed->setRange(0.5, 10.0);
		_dsbSpeed->setValue(1.0);
		_dsbSpeed->setSingleStep(0.5);
		_dsbSpeed->setEnabled(false);

		// full back, back, play/pause, stop, fwd, full fwd buttons
		// TODO: icons
		_pbFullBack = new QPushButton("<<"); _pbFullBack->setEnabled(false);
		_pbBack = new QPushButton("<"); _pbBack->setEnabled(false);
		_pbPlayPause = new QPushButton("PP"); _pbPlayPause->setEnabled(false);
		_pbStop = new QPushButton("S"); _pbStop->setEnabled(false);
		_pbFwd = new QPushButton(">"); _pbFwd->setEnabled(false);
		_pbFullFwd = new QPushButton(">>"); _pbFullFwd->setEnabled(false);

		_pbPlayPause->setCheckable(true);

		int w = 30;
		_pbFullBack->setMaximumWidth(w);
		_pbBack->setMaximumWidth(w);
		_pbPlayPause->setMaximumWidth(w);
		_pbStop->setMaximumWidth(w);
		_pbFwd->setMaximumWidth(w);
		_pbFullFwd->setMaximumWidth(w);

		// connect actions to handle changes in all above widgets

		connect(_slCurrDate, &QSlider::valueChanged, [=](int value)
		{
			_deCurrDate->setDate(Global::schedule1->getFirstDate().addDays(value));
		});

		connect(_deCurrDate, &QDateEdit::dateChanged, [=](const QDate& date)
		{
			_slCurrDate->setValue(Global::schedule1->getFirstDate().daysTo(date));
			_rebuildPlot();
		});

		connect(_dsbSpeed, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=](double value)
		{
			_animationTimer.setInterval(1000/value);
		});

		connect(_pbFullBack, &QPushButton::clicked, [=]
		{
			_deCurrDate->setDate(Global::schedule1->getFirstDate());
		});

		connect(_pbBack, &QPushButton::clicked, [=]
		{
			_deCurrDate->setDate(_deCurrDate->date().addDays(-1));
		});

		connect(_pbPlayPause, &QPushButton::toggled, [=](bool checked)
		{
			_pbFullBack->setEnabled(!checked);
			_pbBack->setEnabled(!checked);
			_pbFwd->setEnabled(!checked);
			_pbFullFwd->setEnabled(!checked);
			if(checked)
			{
				_animationTimer.start();
			}
			else
			{
				_animationTimer.stop();
			}
		});

		connect(_pbStop, &QPushButton::clicked, [=]
		{
			_pbPlayPause->setChecked(false);
			_deCurrDate->setDate(Global::schedule1->getFirstDate());
		});

		connect(_pbFwd, &QPushButton::clicked, [=]
		{
			_deCurrDate->setDate(_deCurrDate->date().addDays(1));
		});

		connect(_pbFullFwd, &QPushButton::clicked, [=]
		{
			_deCurrDate->setDate(Global::schedule1->getLastDate());
		});
	}

	void ExplodedView::_rebuildPlot()
	{
		Global::renderer->beginTransformBatch();

		// hide everything
		Global::renderer->setDrawableVisible(_plotBackground, false);
		Global::renderer->setDrawableVisible(_plotFloor, false);

		for(auto d : _plotHorizontalBars)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		for(auto l : _plotHorizontalLabels)
		{
			l->setVisible(false);
		}
		for(auto d : _plotVerticalLines)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		for(auto d : _plotVerticalSpheres)
		{
			Global::renderer->setDrawableVisible(d, false);
		}
		for(auto l : _plotSelectionDateLabels)
		{
			l->setVisible(false);
		}
		for(auto d : _plotSelectionDateLines)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		for(auto d : _plotSelectionDateSpheres)
		{
			Global::renderer->setDrawableVisible(d, false);
		}
		for(auto d : _taskBoxDrawables)
		{
			Global::renderer->setDrawableVisible(d, false);
		}
		_visibleTaskBoxes.clear();

		auto tasks = Global::schedule1->getTasksByTaskName(_cbTaskName->currentText());
		for(const auto& t : tasks)
		{
			vector<unsigned int> drawables;
			AABB bounds;
			Global::hierarchy->getRenderData(t->objectName, drawables, bounds);

			vector<unsigned int> duplicateDrawables;

			for(auto d : drawables)
			{
				Global::renderer->setBatchDrawableTransform(d, mat4::IDENTITY);
				auto d2 = Global::renderer->getDuplicateDrawableID(d);
				Global::renderer->setBatchDrawableTransform(d2, mat4::IDENTITY);
				duplicateDrawables.push_back(d2);
			}

			Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightMagenta, false);
			Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightBlue, false);

			Global::renderer->setDrawablesHighlight(duplicateDrawables.data(), duplicateDrawables.size(), Renderer::HighlightMagenta, false);
			Global::renderer->setDrawablesHighlight(duplicateDrawables.data(), duplicateDrawables.size(), Renderer::HighlightRed, false);
			Global::renderer->setDrawablesHighlight(duplicateDrawables.data(), duplicateDrawables.size(), Renderer::HighlightGreen, false);
		}

		// -------------------------------------------------------------------------------------------

		if(!_ck3DPlot->isChecked())
		{
			Global::renderer->endTransformBatch();
			emit plotChanged();
			return;
		}

		// rebuild everything

		auto sceneBound = Global::renderer->getSceneOriginalBounds();
		auto sceneSize = sceneBound.max - sceneBound.min;

		vec3 plotOrigin;
		vec3 plotBackgroundSize;
		vec3 plotBackgroundLocalOffset;
		vec3 plotHorizontalLocalOffset;
		float plotHeight = sceneSize.z * _dsbVerticalScale->value();
		int plotWidthAxis = 0;
		int plotDepthAxis = 1;
		float plotWidthAngle;
		float plotDepthAngle;

		vec3 axis[] = {vec3::UNIT_X, vec3::UNIT_Y, vec3::UNIT_Z};

		auto currFace = _cbBoundFace->currentText();
		if(currFace == FACE_POS_X)
		{
			plotOrigin = vec3(sceneBound.max.x, sceneBound.min.y, sceneBound.min.z);
			plotBackgroundSize = vec3(sceneSize.x*1e-6f, sceneSize.y, plotHeight);
			plotBackgroundLocalOffset = vec3(0.5f);
			plotHorizontalLocalOffset = vec3(-0.5f, 0.5f, 0.5f);
			plotWidthAxis = 1;
			plotDepthAxis = 0;
			plotWidthAngle = -math::HALF_PI;
			plotDepthAngle = math::HALF_PI;
		}
		else if(currFace == FACE_NEG_X)
		{
			plotOrigin = vec3(sceneBound.min.x, sceneBound.max.y, sceneBound.min.z);
			plotBackgroundSize = vec3(sceneSize.x*1e-6f, sceneSize.y, plotHeight);
			plotBackgroundLocalOffset = vec3(0.5f, -0.5f, 0.5f);
			plotHorizontalLocalOffset = vec3(0.5f, -0.5f, 0.5f);
			plotWidthAxis = 1;
			plotDepthAxis = 0;
			plotWidthAngle = math::HALF_PI;
			plotDepthAngle = -math::HALF_PI;
		}
		else if(currFace == FACE_POS_Y)
		{
			plotOrigin = vec3(sceneBound.max.x, sceneBound.max.y, sceneBound.min.z);
			plotBackgroundSize = vec3(sceneSize.x, sceneSize.y*1e-6f, plotHeight);
			plotBackgroundLocalOffset = vec3(-0.5f, 0.5f, 0.5f);
			plotHorizontalLocalOffset = vec3(-0.5f, -0.5f, 0.5f);
			plotWidthAxis = 0;
			plotDepthAxis = 1;
			plotWidthAngle = math::HALF_PI;
			plotDepthAngle = math::HALF_PI;
		}
		else if(currFace == FACE_NEG_Y)
		{
			plotOrigin = vec3(sceneBound.min.x, sceneBound.min.y, sceneBound.min.z);
			plotBackgroundSize = vec3(sceneSize.x, sceneSize.y*1e-6f, plotHeight);
			plotBackgroundLocalOffset = vec3(0.5f);
			plotHorizontalLocalOffset = vec3(0.5f);
			plotWidthAxis = 0;
			plotDepthAxis = 1;
			plotWidthAngle = -math::HALF_PI;
			plotDepthAngle = -math::HALF_PI;
		}
		else
		{
			throw std::exception();
		}

		plotOrigin.x += _dsbXOffset->value();
		plotOrigin.y += _dsbYOffset->value();
		plotOrigin.z += _dsbZOffset->value();

		// background
		Global::renderer->setBatchDrawableTransform(_plotBackground, mat4::translation(plotOrigin) * mat4::scale(plotBackgroundSize) * mat4::translation(plotBackgroundLocalOffset));
		Global::renderer->setDrawableVisible(_plotBackground, true);

		// floor
		if(_ckHorizontalPlane->isChecked())
		{
			Global::renderer->setBatchDrawableTransform(_plotFloor, mat4::translation(plotOrigin) * mat4::scale({sceneSize.x, sceneSize.y, sceneSize.z*1e-6f}) * mat4::translation(plotHorizontalLocalOffset));
			Global::renderer->setDrawableVisible(_plotFloor, true);
		}

		// horizontal bars and labels
		vector<QString> labelTexts;
		vector<vec3> labelPositions;

		function<int(const QDate&)> dayOffset;

		auto labelMode = _cbVerticalLabels->currentText();

		if(labelMode == LABEL_DAILY)
		{
			dayOffset = [](auto)
			{
				return 1;
			};
		}
		else if(labelMode == LABEL_WEEKLY)
		{
			dayOffset = [](auto)
			{
				return 7;
			};
		}
		else if(labelMode == LABEL_MONTHLY)
		{
			dayOffset = [](auto d)
			{
				return d.daysInMonth();
			};
		}
		else if(labelMode == LABEL_YEARLY)
		{
			dayOffset = [](auto d)
			{
				return d.daysInYear();
			};
		}
		else
		{
			throw std::exception();
		}

		unsigned int numLabels = _sbVerticalCount->value();
		float dz = plotHeight / (numLabels-1);
		vec3 pos = plotOrigin;
		auto date = _deCurrDate->date();
		auto lastLabelDate = date;
		auto lastLabelHeight = 0.0f;
		while(labelTexts.size() < numLabels && date <= Global::schedule1->getLastDate())
		{
			labelTexts.push_back(date.toString(DATE_FORMAT));
			labelPositions.push_back(pos);
			lastLabelDate = date;
			lastLabelHeight = pos.z;
			date = date.addDays(dayOffset(date));
			pos.z += dz;
		}
		int allLabelsDays = _deCurrDate->date().daysTo(lastLabelDate);
		float allLabelsHeight = lastLabelHeight - plotOrigin.z;

		for(unsigned int i = 0; i < labelTexts.size(); ++i)
		{
			auto pos = labelPositions.at(i);

			// horizontal bar
			auto drawable = _plotHorizontalBars.at(i);
			vec3 p2;
			p2[plotDepthAxis] = plotOrigin[plotDepthAxis];
			p2[plotWidthAxis] = plotOrigin[plotWidthAxis] + plotBackgroundSize[plotWidthAxis] * (plotWidthAngle < 0? 1.0f : -1.0f);
			p2.z = pos.z;
			Global::renderer->setLineDrawablePoints(drawable, pos, p2);
			Global::renderer->setLineDrawableVisible(drawable, true);

			// label
			pos[plotWidthAxis] -= plotBackgroundLocalOffset[plotWidthAxis] * 3.0f;
			auto label = _plotHorizontalLabels.at(i);
			label->setVisible(true);
			label->setText(labelTexts.at(i));
			label->adjustSize();
			Global::overlay->setWidgetPosition(label, pos);
		}

		// tasks to plot
		tasks = Global::schedule1->getTasksByTaskName(_cbTaskName->currentText());
		auto tasks2 = Global::schedule2->getTasksByTaskName(_cbTaskName->currentText());

		// re-position scene drawables, vertical lines, vertical spheres and selection dates
		const auto& selection = Global::selection->getSelection();

		int auxIdx = 0;

		for(int idx = 0; idx < tasks.size(); ++idx)
		{
			auto t = tasks.at(idx);
			auto t2 = tasks2.at(idx);

			QDate refDate;
			QDate refDate2;
			switch(_cbStartFinish->currentIndex())
			{
			case 0:
				refDate = t->baselineStart;
				refDate2 = t2->baselineStart;
				break;
			case 1:
				refDate = t->baselineFinish;
				refDate2 = t2->baselineFinish;
				break;
			case 2:
				refDate = t->actualStart;
				refDate2 = t2->actualStart;
				break;
			case 3:
				refDate = t->actualFinish;
				refDate2 = t2->actualFinish;
				break;
			default:
				throw std::exception();
			}

			vector<unsigned int> drawables;
			AABB bound;
			Global::hierarchy->getRenderData(t->objectName, drawables, bound);

			if(drawables.empty())
			{
				continue;
			}

			auto center = bound.getCenter();

			auto deltaZ = ((float)_deCurrDate->date().daysTo(refDate) / allLabelsDays) * allLabelsHeight;
			deltaZ += plotOrigin.z - bound.min.z; // offset by plotOrigin.z to make sure all objects use the same reference frame

			bool isOutOfRange = false;

			// clamp to (plotOrigin.z, highest label in plot)
			if(bound.max.z + deltaZ > lastLabelHeight)
			{
				deltaZ = lastLabelHeight - bound.min.z + deltaZ * 0.025f;
				isOutOfRange = true;
			}
			else if(bound.min.z + deltaZ < plotOrigin.z) // if beneath plot origin, reset to original position
			{
				deltaZ = 0;
			}

			// ---------------------------------

			auto deltaZ2 = ((float)_deCurrDate->date().daysTo(refDate2) / allLabelsDays) * allLabelsHeight;
			deltaZ2 += plotOrigin.z - bound.min.z; // offset by plotOrigin.z to make sure all objects use the same reference frame

			bool isOutOfRange2 = false;

			// clamp to (plotOrigin.z, highest label in plot)
			if(bound.max.z + deltaZ2 > lastLabelHeight)
			{
				deltaZ2 = lastLabelHeight - bound.min.z + deltaZ2 * 0.025f;
				isOutOfRange2 = true;
			}
			else if(bound.min.z + deltaZ2 < plotOrigin.z) // if beneath plot origin, reset to original position
			{
				deltaZ2 = 0;
			}

			// ---------------------------------

			vector<unsigned int> magentaHighlight;
			vector<unsigned int> greenHighlight;
			vector<unsigned int> redHighlight;
			vector<unsigned int> blueHighlight;

			for(auto d : drawables)
			{
				Global::renderer->setBatchDrawableTransform(d, mat4::translation({0,0,deltaZ}));
				if(isOutOfRange)
				{
					if(_hideOutOfRange)
					{
						Global::renderer->setDrawableVisible(d, false);
					}
					else if(_showOutOfRange)
					{
						Global::renderer->setDrawableVisible(d, true);
					}
					if(_ckHighlightOutOfRange->isChecked() && !selection.contains(d) && !Global::renderer->isDrawableTransparent(d))
					{
						magentaHighlight.push_back(d);
					}
				}

				if(_ckCompareSchedules->isChecked())
				{
					if(refDate2 != refDate)
					{
						auto d2 = Global::renderer->getDuplicateDrawableID(d);

						Global::renderer->setDrawableVisible(d2, true);

						if(!selection.contains(d2))
						{
							if(refDate2 < refDate)
							{
								greenHighlight.push_back(d);
							}
							else
							{
								redHighlight.push_back(d2);
							}
						}

						if(!selection.contains(d))
						{
							blueHighlight.push_back(d);
						}

						// change size of duplicate drawable to avoid z-fights
						Global::renderer->setBatchDrawableTransform(d2, mat4::translation({0,0,deltaZ2}) * mat4::translation(center) * mat4::scale(vec3(1.05f)) * mat4::translation(-center));

						if(isOutOfRange2)
						{
							if(_hideOutOfRange)
							{
								Global::renderer->setDrawableVisible(d2, false);
							}
							else if(_showOutOfRange)
							{
								Global::renderer->setDrawableVisible(d2, true);
							}
							if(_ckHighlightOutOfRange->isChecked() && !selection.contains(d2) && !Global::renderer->isDrawableTransparent(d2))
							{
								magentaHighlight.push_back(d2);
							}
						}
					}
				}
			}

			Global::renderer->setDrawablesHighlight(magentaHighlight.data(), magentaHighlight.size(), Renderer::HighlightMagenta, true);
			Global::renderer->setDrawablesHighlight(greenHighlight.data(), greenHighlight.size(), Renderer::HighlightGreen, true);
			Global::renderer->setDrawablesHighlight(redHighlight.data(), redHighlight.size(), Renderer::HighlightRed, true);
			Global::renderer->setDrawablesHighlight(blueHighlight.data(), blueHighlight.size(), Renderer::HighlightBlue, true);

			center.z = bound.min.z + deltaZ;

			// vertical line
			if(_showVerticalLines && deltaZ != 0.0f && selection.contains(drawables.at(0)) && Global::renderer->isDrawableVisible(drawables.at(0))  && !Global::renderer->isDrawableTransparent(drawables.at(0)))
			{
				float bottomZ = _ckHorizontalPlane->isChecked()? plotOrigin.z : bound.min.z;

				auto lineDrawable = _plotVerticalLines.at(auxIdx);
				Global::renderer->setLineDrawablePoints(lineDrawable, center, vec3(center.x, center.y, bottomZ));
				Global::renderer->setLineDrawableVisible(lineDrawable, true);

				// vertical sphere
				auto sphereDrawable = _plotVerticalSpheres.at(auxIdx);
				Global::renderer->setBatchDrawableTransform(sphereDrawable, mat4::translation({center.x, center.y, bottomZ}));
				Global::renderer->setDrawableVisible(sphereDrawable, true);
			}

			// selection dates
			if(_showSelectionDates && deltaZ != 0.0f && selection.contains(drawables.at(0)) && Global::renderer->isDrawableVisible(drawables.at(0)))
			{
				vec3 plotProjectionPoint;
				plotProjectionPoint[plotDepthAxis] = plotOrigin[plotDepthAxis];
				plotProjectionPoint[plotWidthAxis] = center[plotWidthAxis];
				plotProjectionPoint.z = center.z;

				// sphere
				auto sphereDrawable = _plotSelectionDateSpheres.at(auxIdx);
				Global::renderer->setBatchDrawableTransform(sphereDrawable, mat4::translation(plotProjectionPoint));
				Global::renderer->setDrawableVisible(sphereDrawable, true);

				// line
				auto lineDrawable = _plotSelectionDateLines.at(auxIdx);
				Global::renderer->setLineDrawablePoints(lineDrawable, center, plotProjectionPoint);
				Global::renderer->setLineDrawableVisible(lineDrawable, true);

				// label
				auto l = _plotSelectionDateLabels.at(auxIdx);
				l->setText(refDate.toString(DATE_FORMAT));
				l->adjustSize();
				l->setVisible(true);
				plotProjectionPoint[plotWidthAxis] += plotBackgroundLocalOffset[plotWidthAxis] * 5.0f;
				plotProjectionPoint.z += 2.0f;
				Global::overlay->setWidgetPosition(l, plotProjectionPoint);
			}

			// selection task boxes
			if(_showTaskBoxes && deltaZ != 0.0f && selection.contains(drawables.at(0)) && Global::renderer->isDrawableVisible(drawables.at(0)))
			{
				auto startDeltaZ = ((float)_deCurrDate->date().daysTo(t->baselineStart) / allLabelsDays) * allLabelsHeight;
				startDeltaZ += plotOrigin.z - bound.min.z; // offset by plotOrigin.z to make sure all objects use the same reference frame

				if(bound.max.z + startDeltaZ > lastLabelHeight)
				{
					startDeltaZ = lastLabelHeight - bound.min.z + startDeltaZ * 0.025f;
				}
				else if(bound.min.z + startDeltaZ < plotOrigin.z) // if beneath plot origin, reset to original position
				{
					startDeltaZ = 0;
				}

				auto boxHeight = deltaZ - startDeltaZ;
				auto boundSize = bound.getSize();
				auto boxSize = vec3(boundSize.x, boundSize.y, boxHeight);

				auto translation = center;
				translation.z -= boxSize.z * 0.5f;

				auto m = mat4::translation(translation) * mat4::scale(boxSize);

				// box
				auto taskBoxDrawable = _taskBoxDrawables.at(auxIdx);
				Global::renderer->setBatchDrawableTransform(taskBoxDrawable, m);
				Global::renderer->setDrawableVisible(taskBoxDrawable, true);

				AABB tbound;
				tbound.min = m * vec3(-0.5f);
				tbound.max = m * vec3(0.5f);

				TaskBoxInfo tbi;
				tbi.drawable = taskBoxDrawable;
				tbi.bound = tbound;
				_visibleTaskBoxes.push_back(tbi);
			}

			++auxIdx;

			// ---------

			if(_ckCompareSchedules->isChecked())
			{
				center.z = bound.min.z + deltaZ2;

				auto dd = Global::renderer->getDuplicateDrawableID(drawables.at(0));

				// vertical line
				if(_showVerticalLines && deltaZ2 != 0.0f && Global::renderer->isDrawableVisible(dd) && !Global::renderer->isDrawableTransparent(dd))
				{
					float bottomZ = _ckHorizontalPlane->isChecked()? plotOrigin.z : bound.min.z;

					auto lineDrawable = _plotVerticalLines.at(auxIdx);
					Global::renderer->setLineDrawablePoints(lineDrawable, center, vec3(center.x, center.y, bottomZ));
					Global::renderer->setLineDrawableVisible(lineDrawable, true);

					// vertical sphere
					auto sphereDrawable = _plotVerticalSpheres.at(auxIdx);
					Global::renderer->setBatchDrawableTransform(sphereDrawable, mat4::translation({center.x, center.y, bottomZ}));
					Global::renderer->setDrawableVisible(sphereDrawable, true);
				}

				// selection dates
				if(_showSelectionDates && deltaZ2 != 0.0f && selection.contains(dd) && Global::renderer->isDrawableVisible(dd))
				{
					vec3 plotProjectionPoint;
					plotProjectionPoint[plotDepthAxis] = plotOrigin[plotDepthAxis];
					plotProjectionPoint[plotWidthAxis] = center[plotWidthAxis];
					plotProjectionPoint.z = center.z;

					// sphere
					auto sphereDrawable = _plotSelectionDateSpheres.at(auxIdx);
					Global::renderer->setBatchDrawableTransform(sphereDrawable, mat4::translation(plotProjectionPoint));
					Global::renderer->setDrawableVisible(sphereDrawable, true);

					// line
					auto lineDrawable = _plotSelectionDateLines.at(auxIdx);
					Global::renderer->setLineDrawablePoints(lineDrawable, center, plotProjectionPoint);
					Global::renderer->setLineDrawableVisible(lineDrawable, true);

					// label
					auto l = _plotSelectionDateLabels.at(auxIdx);
					l->setText(refDate2.toString(DATE_FORMAT));
					l->adjustSize();
					l->setVisible(true);
					plotProjectionPoint[plotWidthAxis] += plotBackgroundLocalOffset[plotWidthAxis] * 10.0f;
					plotProjectionPoint.z += 2.0f;
					Global::overlay->setWidgetPosition(l, plotProjectionPoint);
				}

				++auxIdx;
			}
		}

		// change task box color if they are colliding
		// TODO: this is quadratic :(
		for(unsigned int i = 0; i < _visibleTaskBoxes.size(); ++i)
		{
			for(unsigned int j = i+1; j < _visibleTaskBoxes.size(); ++j)
			{
				auto b1 = _visibleTaskBoxes.at(i);
				auto b2 = _visibleTaskBoxes.at(j);

				if(collide(b1.bound, b2.bound))
				{
					Global::renderer->setDrawableColorID(b1.drawable, _collidedTaskBoxColorID);
					Global::renderer->setDrawableColorID(b2.drawable, _collidedTaskBoxColorID);
				}
				else
				{
					Global::renderer->setDrawableColorID(b1.drawable, _freeTaskBoxColorID);
					Global::renderer->setDrawableColorID(b2.drawable, _freeTaskBoxColorID);
				}
			}
		}

		Global::renderer->endTransformBatch();
		emit plotChanged();
	}
}
