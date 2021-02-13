#include <app/Analysis.h>
#include <app/Global.h>
#include <app/CollapsableDialog.h>
#include <QLayout>

namespace app
{
	static const vector<QString> COLOR_INDICATORS = {"Baseline Start", "Baseline Finish", "Actual Start", "Actual Finish",
													 "Baseline Duration", "Actual Duration", "Delta Duration", "Delta Start", "Delta Finish",
													 "Free Slack", "Total Slack", "Custom Metric"};

	static const int NUM_COLORS = 5;

	void highlight(const AABB& tbound, const vector<TaskRef>& connected, const vector<unsigned int>& lineDrawables, int& lineIdx, float mode,
				   vector<unsigned int>& rdrawables)
	{
		vector<unsigned int> allDrawablesToHighlight;
		for(const auto& c : connected)
		{
			vector<unsigned int> cdrawables;
			AABB cbound;
			Global::hierarchy->getRenderData(c.task->objectName, cdrawables, cbound);

			if(cdrawables.empty() || !Global::renderer->isDrawableVisible(cdrawables.at(0)))
			{
				continue;
			}

			cbound = Global::getUpdatedBound(cdrawables);

			auto d = lineDrawables.at(lineIdx);
			Global::renderer->setLineDrawablePoints(d, tbound.getCenter(), cbound.getCenter());
			Global::renderer->setLineDrawableVisible(d, true);

			++lineIdx;

			allDrawablesToHighlight.insert(allDrawablesToHighlight.end(), cdrawables.begin(), cdrawables.end());

			rdrawables.insert(rdrawables.end(), cdrawables.begin(), cdrawables.end());
		}
		Global::renderer->setDrawablesHighlight(allDrawablesToHighlight.data(), allDrawablesToHighlight.size(), mode, true);
	}

	void createColorSchemes(QMap<QString, vector<SchemeColor>>& schemes)
	{
		vector<SchemeColor> scheme(5);
		scheme.at(0).color.setRgb(255, 0, 0);
		scheme.at(1).color.setRgb(255, 80, 0);
		scheme.at(2).color.setRgb(255, 200, 0);
		scheme.at(3).color.setRgb(0, 255, 0);
		scheme.at(4).color.setRgb(0, 0, 255);
		schemes.insert("Rainbow", scheme);

		// from http://colorbrewer2.org/

		// multi hue

		scheme.at(4).color.setRgb(237,248,251);
		scheme.at(3).color.setRgb(178,226,226);
		scheme.at(2).color.setRgb(102,194,164);
		scheme.at(1).color.setRgb(44,162,95 );
		scheme.at(0).color.setRgb(0,109,44  );
		schemes.insert("BuGn", scheme);

		scheme.at(4).color.setRgb(237,248,251);
		scheme.at(3).color.setRgb(179,205,227);
		scheme.at(2).color.setRgb(140,150,198);
		scheme.at(1).color.setRgb(136,86,167 );
		scheme.at(0).color.setRgb(129,15,124   );
		schemes.insert("BuPu", scheme);

		scheme.at(4).color.setRgb(240,249,232);
		scheme.at(3).color.setRgb(186,228,188);
		scheme.at(2).color.setRgb(123,204,196);
		scheme.at(1).color.setRgb(67,162,202 );
		scheme.at(0).color.setRgb(8,104,172  );
		schemes.insert("GnBu", scheme);

		scheme.at(4).color.setRgb(254,240,217);
		scheme.at(3).color.setRgb(253,204,138);
		scheme.at(2).color.setRgb(252,141,89);
		scheme.at(1).color.setRgb(227,74,51  );
		scheme.at(0).color.setRgb(179,0,0   );
		schemes.insert("OrRd", scheme);

		scheme.at(4).color.setRgb(241,238,246);
		scheme.at(3).color.setRgb(189,201,225);
		scheme.at(2).color.setRgb(116,169,207 );
		scheme.at(1).color.setRgb(43,140,190 );
		scheme.at(0).color.setRgb(4,90,141  );
		schemes.insert("PuBu", scheme);

		scheme.at(4).color.setRgb(246,239,247);
		scheme.at(3).color.setRgb(189,201,225);
		scheme.at(2).color.setRgb(103,169,207);
		scheme.at(1).color.setRgb(28,144,153 );
		scheme.at(0).color.setRgb(1,108,89   );
		schemes.insert("PuBuGn", scheme);

		scheme.at(4).color.setRgb(241,238,246);
		scheme.at(3).color.setRgb(215,181,216);
		scheme.at(2).color.setRgb(223,101,176);
		scheme.at(1).color.setRgb(221,28,119  );
		scheme.at(0).color.setRgb(152,0,67   );
		schemes.insert("PuRd", scheme);

		scheme.at(4).color.setRgb(254,235,226);
		scheme.at(3).color.setRgb(251,180,185);
		scheme.at(2).color.setRgb(247,104,161);
		scheme.at(1).color.setRgb(197,27,138);
		scheme.at(0).color.setRgb(122,1,119);
		schemes.insert("RdPu", scheme);

		scheme.at(4).color.setRgb(255,255,204);
		scheme.at(3).color.setRgb(194,230,153);
		scheme.at(2).color.setRgb(120,198,121);
		scheme.at(1).color.setRgb(49,163,84);
		scheme.at(0).color.setRgb(0,104,55);
		schemes.insert("YlGn", scheme);

		scheme.at(4).color.setRgb(255,255,204);
		scheme.at(3).color.setRgb(161,218,180);
		scheme.at(2).color.setRgb(65,182,196);
		scheme.at(1).color.setRgb(44,127,184);
		scheme.at(0).color.setRgb(37,52,148  );
		schemes.insert("YlGnBu", scheme);

		scheme.at(4).color.setRgb(255,255,212);
		scheme.at(3).color.setRgb(254,217,142);
		scheme.at(2).color.setRgb(254,153,41);
		scheme.at(1).color.setRgb(217,95,14);
		scheme.at(0).color.setRgb(153,52,4  );
		schemes.insert("YlOrBr", scheme);

		scheme.at(4).color.setRgb(255,255,178);
		scheme.at(3).color.setRgb(254,204,92);
		scheme.at(2).color.setRgb(253,141,60);
		scheme.at(1).color.setRgb(240,59,32);
		scheme.at(0).color.setRgb(189,0,38  );
		schemes.insert("YlOrRd", scheme);

		// single hue

		scheme.at(4).color.setRgb(239,243,255);
		scheme.at(3).color.setRgb(189,215,231);
		scheme.at(2).color.setRgb(107,174,214);
		scheme.at(1).color.setRgb(49,130,189);
		scheme.at(0).color.setRgb(8,81,156  );
		schemes.insert("Blues", scheme);

		scheme.at(4).color.setRgb(237,248,233);
		scheme.at(3).color.setRgb(186,228,179);
		scheme.at(2).color.setRgb(116,196,118);
		scheme.at(1).color.setRgb(49,163,84);
		scheme.at(0).color.setRgb(0,109,44  );
		schemes.insert("Greens", scheme);

		scheme.at(4).color.setRgb(254,237,222);
		scheme.at(3).color.setRgb(253,190,133);
		scheme.at(2).color.setRgb(253,141,60);
		scheme.at(1).color.setRgb(230,85,13);
		scheme.at(0).color.setRgb(166,54,3  );
		schemes.insert("Oranges", scheme);

		scheme.at(4).color.setRgb(242,240,247);
		scheme.at(3).color.setRgb(203,201,226);
		scheme.at(2).color.setRgb(158,154,200);
		scheme.at(1).color.setRgb(117,107,177);
		scheme.at(0).color.setRgb(84,39,143  );
		schemes.insert("Purples", scheme);

		scheme.at(4).color.setRgb(254,229,217);
		scheme.at(3).color.setRgb(252,174,145);
		scheme.at(2).color.setRgb(251,106,74);
		scheme.at(1).color.setRgb(222,45,38);
		scheme.at(0).color.setRgb(165,15,21  );
		schemes.insert("Reds", scheme);

		// diverging

		scheme.at(0).color.setRgb(166,97,26);
		scheme.at(1).color.setRgb(223,194,125);
		scheme.at(2).color.setRgb(245,245,245);
		scheme.at(3).color.setRgb(128,205,193);
		scheme.at(4).color.setRgb(1,133,113  );
		schemes.insert("BrBg", scheme);

		scheme.at(0).color.setRgb(208,28,139);
		scheme.at(1).color.setRgb(241,182,218);
		scheme.at(2).color.setRgb(247,247,247);
		scheme.at(3).color.setRgb(184,225,134);
		scheme.at(4).color.setRgb(77,172,38  );
		schemes.insert("PiYG", scheme);

		scheme.at(0).color.setRgb(123,50,148);
		scheme.at(1).color.setRgb(194,165,207);
		scheme.at(2).color.setRgb(247,247,247);
		scheme.at(3).color.setRgb(166,219,160);
		scheme.at(4).color.setRgb(0,136,55  );
		schemes.insert("PRGn", scheme);

		scheme.at(0).color.setRgb(230,97,1);
		scheme.at(1).color.setRgb(253,184,99);
		scheme.at(2).color.setRgb(247,247,247);
		scheme.at(3).color.setRgb(178,171,210);
		scheme.at(4).color.setRgb(94,60,153  );
		schemes.insert("PuOr", scheme);

		scheme.at(0).color.setRgb(202,0,32);
		scheme.at(1).color.setRgb(244,165,130);
		scheme.at(2).color.setRgb(247,247,247);
		scheme.at(3).color.setRgb(146,197,222);
		scheme.at(4).color.setRgb(5,113,176  );
		schemes.insert("RdBu", scheme);

		scheme.at(0).color.setRgb(215,25,28);
		scheme.at(1).color.setRgb(253,174,97);
		scheme.at(2).color.setRgb(255,255,191);
		scheme.at(3).color.setRgb(171,217,233);
		scheme.at(4).color.setRgb(44,123,182  );
		schemes.insert("RdYlBu", scheme);

		scheme.at(0).color.setRgb(215,25,28);
		scheme.at(1).color.setRgb(253,174,97);
		scheme.at(2).color.setRgb(255,255,191);
		scheme.at(3).color.setRgb(166,217,106);
		scheme.at(4).color.setRgb(26,150,65  );
		schemes.insert("RdYlGn", scheme);

		scheme.at(0).color.setRgb(215,25,28);
		scheme.at(1).color.setRgb(253,174,97);
		scheme.at(2).color.setRgb(255,255,191);
		scheme.at(3).color.setRgb(171,221,164);
		scheme.at(4).color.setRgb(43,131,186  );
		schemes.insert("Spectral", scheme);
	}

	// --------------------------------------------------------------------------------------------------------------------------------------------------------

	void Analysis::init()
	{
		_actAnalysis = Global::toolbar->addAction("Analysis", this, SLOT(_onActionAnalysisTriggered(bool)));
		_actAnalysis->setCheckable(true);

		auto predLineColorID = Global::renderer->addColor(0.0f, 1.0f, 0.0f);
		auto sucLineColorID = Global::renderer->addColor(0.0f, 0.0f, 1.0f);
		auto critLineColorID = Global::renderer->addColor(1.0f, 0.0f, 0.0f);
		auto osLineColorID = Global::renderer->addColor(1.0f, 0.75f, 0.0f);

		for(int i = 0; i < 1e3; ++i)
		{
			auto d = Global::renderer->addLineDrawable(predLineColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(d, false);
			_predLines.push_back(d);

			d = Global::renderer->addLineDrawable(sucLineColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(d, false);
			_sucLines.push_back(d);

			d = Global::renderer->addLineDrawable(critLineColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(d, false);
			_critLines.push_back(d);

			d = Global::renderer->addLineDrawable(osLineColorID, vec3::ZERO, vec3::ZERO);
			Global::renderer->setLineDrawableVisible(d, false);
			_osLines.push_back(d);
		}

		Global::selection->addSelectionCallback("Predecessors/Successors", [=](bool checked)
		{
			_showPredSucLines = checked;
			_updatePredSucLines();
		}, _showPredSucLines);

		Global::selection->addSelectionCallback("Operating System", [=](bool checked)
		{
			_showOSLines = checked;
			_updateOSLines();
		}, _showOSLines);

		connect(Global::explodedview, SIGNAL(plotChanged()), this, SLOT(_onPlotChanged()));

		createColorSchemes(_colorSchemes);

		// create color ids in renderer
		for(auto& cs : _colorSchemes)
		{
			for(auto& c : cs)
			{
				c.id = Global::renderer->addColor(c.color.redF(), c.color.greenF(), c.color.blueF());
			}
		}

		_createWidgets();

		_colorScale.init(20, 40, NUM_COLORS);
		Global::glscene->addOverlayWidget(_colorScale.getWidget(), OpenGLScene::Corner_LowerRight);
		_colorScale.getWidget()->setVisible(false);

		connect(&_colorScale, SIGNAL(colorClicked(int)), this, SLOT(_onColorDoubleClicked(int)));
	}

	void Analysis::saveState()
	{
		Global::settings->setValue("Analysis/CheckCritical", QVariant::fromValue(_ckCriticalPath->isChecked()));
		Global::settings->setValue("Analysis/CheckColorScale", QVariant::fromValue(_ckColorScale->isChecked()));
		Global::settings->setValue("Analysis/ColorSchemes", QVariant::fromValue(_cbColorSchemes->currentIndex()));
		Global::settings->setValue("Analysis/ColorIndicator", QVariant::fromValue(_cbColorIndicator->currentIndex()));
		Global::settings->setValue("Analysis/DsColorMin", QVariant::fromValue(_dsColorMin->value()));
		Global::settings->setValue("Analysis/DsColorMax", QVariant::fromValue(_dsColorMax->value()));
		Global::settings->setValue("Analysis/DeColorMin", QVariant::fromValue(_deColorMin->dateTime()));
		Global::settings->setValue("Analysis/DeColorMax", QVariant::fromValue(_deColorMax->dateTime()));
	}

	void Analysis::restoreState()
	{
		if(Global::settings->contains("Analysis/CheckCritical"))
		{
			_ckCriticalPath->setChecked(Global::settings->value("Analysis/CheckCritical").toBool());
		}
		if(Global::settings->contains("Analysis/CheckColorScale"))
		{
			_ckColorScale->setChecked(Global::settings->value("Analysis/CheckColorScale").toBool());
		}
		if(Global::settings->contains("Analysis/ColorSchemes"))
		{
			_cbColorSchemes->setCurrentIndex(Global::settings->value("Analysis/ColorSchemes").toInt());
		}
		if(Global::settings->contains("Analysis/ColorIndicator"))
		{
			_cbColorIndicator->setCurrentIndex(Global::settings->value("Analysis/ColorIndicator").toInt());
		}
		if(Global::settings->contains("Analysis/DsColorMin"))
		{
			_dsColorMin->setValue(Global::settings->value("Analysis/DsColorMin").toDouble());
		}
		if(Global::settings->contains("Analysis/DsColorMax"))
		{
			_dsColorMax->setValue(Global::settings->value("Analysis/DsColorMax").toDouble());
		}
		if(Global::settings->contains("Analysis/DeColorMin"))
		{
			_deColorMin->setDateTime(Global::settings->value("Analysis/DeColorMin").toDateTime());
		}
		if(Global::settings->contains("Analysis/DeColorMax"))
		{
			_deColorMax->setDateTime(Global::settings->value("Analysis/DeColorMax").toDateTime());
		}
	}

	void Analysis::_onPlotChanged()
	{
		if(_dlgAnalysis)
		{
			_dlgAnalysis->setEnabled(Global::explodedview->isActive());
		}
		_updateCritLines();
		_updatePredSucLines();
		_updateColorScale();
		_updateOSLines();
	}

	void Analysis::_onActionAnalysisTriggered(bool checked)
	{
		if(!_dlgAnalysis)
		{
			_dlgAnalysis = new CollapsableDialog("Analysis");
			_dlgAnalysis->layout()->addWidget(_ckCriticalPath);
			_dlgAnalysis->layout()->addWidget(_ckColorScale);
			_dlgAnalysis->layout()->addWidget(_cbColorSchemes);
			_dlgAnalysis->layout()->addWidget(_cbColorIndicator);

			QHBoxLayout* hlds = new QHBoxLayout();
			hlds->addWidget(_dsColorMin);
			hlds->addWidget(_dsColorMax);

			QHBoxLayout* hlde = new QHBoxLayout();
			hlde->addWidget(_deColorMin);
			hlde->addWidget(_deColorMax);

			qobject_cast<QVBoxLayout*>(_dlgAnalysis->layout())->addLayout(hlds);
			qobject_cast<QVBoxLayout*>(_dlgAnalysis->layout())->addLayout(hlde);

			Global::glscene->addDialog(_dlgAnalysis);
			_dlgAnalysis->setEnabled(Global::explodedview->isActive());
		}
		_dlgAnalysis->setVisible(checked);
	}

	void Analysis::_onColorDoubleClicked(int idx)
	{
		AABB overallBound;
		vector<unsigned int> overallDrawables;

		auto tasks = Global::schedule1->getTasksByTaskName(Global::explodedview->getActiveTaskName());

		for(auto t : tasks)
		{
			int colorIdx = 0;
			switch(_cbColorIndicator->currentIndex())
			{
			case 0:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->baselineStart) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 1:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->baselineFinish) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 2:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->actualStart) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 3:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->actualFinish) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 4:
				colorIdx = _colorScale.getNumColors() * (float)(t->getBaselineDuration() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 5:
				colorIdx = _colorScale.getNumColors() * (float)(t->getActualDuration() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 6:
				colorIdx = _colorScale.getNumColors() * (float)(t->getDeltaDuration() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 7:
				colorIdx = _colorScale.getNumColors() * (float)(t->getDeltaStart() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 8:
				colorIdx = _colorScale.getNumColors() * (float)(t->getDeltaFinish() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 9:
				colorIdx = _colorScale.getNumColors() * (float)(t->freeSlack - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 10:
				colorIdx = _colorScale.getNumColors() * (float)(t->totalSlack - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 11:
				colorIdx = _colorScale.getNumColors() * (float)(t->getCustomMetric() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			default:
				throw std::exception();
			}
			colorIdx = colorIdx < 0? 0 : (colorIdx >= (int)_colorScale.getNumColors()? _colorScale.getNumColors() - 1 : colorIdx);
			colorIdx = _colorScale.getNumColors()-(colorIdx+1);

			if(colorIdx == idx)
			{
				vector<unsigned int> drawables;
				AABB bound;
				Global::hierarchy->getRenderData(t->objectName, drawables, bound);

				overallBound.expand(Global::getUpdatedBound(drawables));

				for(auto d : drawables)
				{
					if(Global::renderer->isDrawableVisible(d))
					{
						overallDrawables.push_back(d);
					}
				}
			}
		}

		if(overallDrawables.empty() || !overallBound.isValid())
		{
			return;
		}

		auto first = Global::renderer->getFirstSceneDrawable();
		auto last = Global::renderer->getLastSceneDrawable();
		for(unsigned int i = first; i <= last; ++i)
		{
			Global::renderer->setDrawableTransparent(i, true);
		}

		for(auto d : overallDrawables)
		{
			Global::renderer->setDrawableTransparent(d, false);
		}

		Global::selection->setSelection(overallDrawables);
		Global::camera->setFocus(overallBound);
	}

	void Analysis::_createWidgets()
	{
		_ckCriticalPath = new QCheckBox("Critical Path");

		_ckColorScale = new QCheckBox("Color Scale");

		_cbColorSchemes = new QComboBox();
		_cbColorSchemes->setEnabled(false);
		for(auto itr = _colorSchemes.begin(); itr != _colorSchemes.end(); ++itr)
		{
			_cbColorSchemes->addItem(itr.key());
		}
		_cbColorSchemes->setCurrentText("Rainbow");

		_cbColorIndicator = new QComboBox();
		for(unsigned int i = 0; i < COLOR_INDICATORS.size(); ++i)
		{
			_cbColorIndicator->addItem(COLOR_INDICATORS.at(i));
		}
		_cbColorIndicator->setEnabled(false);

		_dsColorMin = new QDoubleSpinBox();
		_dsColorMin->setRange(-1e6, 1e6);
		_dsColorMin->setVisible(false);

		_dsColorMax = new QDoubleSpinBox();
		_dsColorMax->setRange(-1e6, 1e6);
		_dsColorMax->setVisible(false);

		_deColorMin = new QDateEdit();
		_deColorMin->setDateRange(Global::schedule1->getFirstDate(), Global::schedule1->getLastDate());
		_deColorMin->setVisible(false);

		_deColorMax = new QDateEdit();
		_deColorMax->setDateRange(Global::schedule1->getFirstDate(), Global::schedule1->getLastDate());
		_deColorMax->setVisible(false);

		connect(_ckCriticalPath, &QCheckBox::toggled, [=](bool)
		{
			_updateCritLines();
		});

		connect(_ckColorScale, &QCheckBox::toggled, [=](bool checked)
		{
			_cbColorSchemes->setEnabled(checked);
			_cbColorIndicator->setEnabled(checked);
			_colorScale.getWidget()->setVisible(_ckColorScale->isChecked());
			_updateColorScale();
		});

		connect(_cbColorSchemes, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]
		{
			_updateColorScale();
		});

		connect(_cbColorIndicator, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]
		{
			_updateColorScale();
		});

		connect(_dsColorMin, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=]
		{
			_updateColorScale();
		});

		connect(_dsColorMax, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), [=]
		{
			_updateColorScale();
		});

		connect(_deColorMin, &QDateEdit::dateChanged, [=](QDate)
		{
			_updateColorScale();
		});

		connect(_deColorMax, &QDateEdit::dateChanged, [=](QDate)
		{
			_updateColorScale();
		});
	}

	void Analysis::_updateCritLines()
	{
		for(auto d : _critLines)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		Global::renderer->setDrawablesHighlight(_critDrawables.data(), _critDrawables.size(), Renderer::HighlightRed, false);
		_critDrawables.clear();

		if(!_ckCriticalPath->isChecked() || !Global::explodedview->isActive())
		{
			return;
		}

		const auto& selection = Global::selection->getSelection();
		auto tasks = Global::schedule1->getTasksByTaskName(Global::explodedview->getActiveTaskName());

		int lineIdx = 0;

		for(auto t : tasks)
		{
			if(!t->isCritical)
			{
				continue;
			}

			vector<unsigned int> drawables;
			AABB bound;
			Global::hierarchy->getRenderData(t->objectName, drawables, bound);

			if(drawables.empty() || !Global::renderer->isDrawableVisible(drawables.at(0)))
			{
				continue;
			}

			bound = Global::getUpdatedBound(drawables);

			// give priority to selection highlight
			if(!selection.contains(drawables.at(0)))
			{
				for(auto d : drawables)
				{
					_critDrawables.push_back(d);
				}
				Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightRed, true);
			}

			vector<TaskRef> critConnected;
			for(auto p : t->predecessors)
			{
				if(p.task->isCritical)
				{
					critConnected.push_back(p);
				}
			}
			for(auto s : t->successors)
			{
				if(s.task->isCritical)
				{
					critConnected.push_back(s);
				}
			}

			highlight(bound, critConnected, _critLines, lineIdx, Renderer::HighlightRed, _critDrawables);
		}
	}

	void Analysis::_updatePredSucLines()
	{
		for(auto d : _predLines)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		for(auto d : _sucLines)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		Global::renderer->setDrawablesHighlight(_predDrawables.data(), _predDrawables.size(), Renderer::HighlightGreen, false);
		Global::renderer->setDrawablesHighlight(_sucDrawables.data(), _sucDrawables.size(), Renderer::HighlightBlue, false);

		_predDrawables.clear();
		_sucDrawables.clear();

		if(!_showPredSucLines || !Global::explodedview->isActive())
		{
			return;
		}

		const auto& selection = Global::selection->getSelection();
		auto tasks = Global::schedule1->getTasksByTaskName(Global::explodedview->getActiveTaskName());

		int lineIdx = 0;

		for(auto t : tasks)
		{
			vector<unsigned int> drawables;
			AABB bound;
			Global::hierarchy->getRenderData(t->objectName, drawables, bound);

			if(!drawables.empty() && selection.contains(drawables.at(0)))
			{
				bound = Global::getUpdatedBound(drawables);
				highlight(bound, t->predecessors, _predLines, lineIdx, Renderer::HighlightGreen, _predDrawables);
				highlight(bound, t->successors, _sucLines, lineIdx, Renderer::HighlightBlue, _sucDrawables);
			}
		}
	}

	void Analysis::_updateColorScale()
	{
		_dsColorMin->setVisible(false);
		_dsColorMax->setVisible(false);
		_deColorMin->setVisible(false);
		_deColorMax->setVisible(false);

		// revert state
//		for(const auto& c : _originalDrawableColors)
//		{
//			Global::renderer->setDrawableColorID(c.drawableID, c.colorID);
//		}

		auto first = Global::renderer->getFirstSceneDrawable();
		auto last = Global::renderer->getLastSceneDrawable();
		for(unsigned int i = first; i <= last; ++i)
		{
			Global::renderer->setDrawableColorID(i, 0);
		}

//		_originalDrawableColors.clear();

		if(!_ckColorScale->isChecked())
		{
			return;
		}

		// apply current color scheme to color scale
		const auto& currScheme = _colorSchemes.value(_cbColorSchemes->currentText());
		int idx = 0;
		for(const auto& c : currScheme)
		{
			QColor correctedColor = c.color;
			correctedColor.setRedF(math::min(1.0, pow(correctedColor.redF() + 0.1f, 1.0/2.2)));
			correctedColor.setGreenF(math::min(1.0, pow(correctedColor.greenF() + 0.1f, 1.0/2.2)));
			correctedColor.setBlueF(math::min(1.0, pow(correctedColor.blueF() + 0.1f, 1.0/2.2)));
//			_colorScale.setColor(idx++, correctedColor);
			_colorScale.setColor(idx++, c.color);
		}
		_colorScale.setTitle(_cbColorIndicator->currentText());

		// get min, max range from schedule using current indicator
		// show appropriate min, max widgets, set their ranges and current values
		// set color scale labels
		if(_cbColorIndicator->currentIndex() <= 3) // if is date type
		{
			QDate min;
			QDate max;

			switch(_cbColorIndicator->currentIndex())
			{
			case 0:
				min = Global::schedule1->getMinBaselineStart();
				max = Global::schedule1->getMaxBaselineStart();
				break;
			case 1:
				min = Global::schedule1->getMinBaselineFinish();
				max = Global::schedule1->getMaxBaselineFinish();
				break;
			case 2:
				min = Global::schedule1->getMinActualStart();
				max = Global::schedule1->getMaxActualStart();
				break;
			case 3:
				min = Global::schedule1->getMinActualFinish();
				max = Global::schedule1->getMaxActualFinish();
				break;
			case 4:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
				break;
			default:
				throw std::exception();
			}

			_deColorMin->setDateRange(min, max);
			_deColorMin->setDate(min);

			_deColorMax->setDateRange(min, max);
			_deColorMax->setDate(max);

			QDate value = max;
			int delta = min.daysTo(max) / _colorScale.getNumColors();
			for(unsigned int i = 0; i < _colorScale.getNumColors()+1; ++i)
			{
				_colorScale.setLabel(i, value.toString("MMM dd yyyy"));
				value = value.addDays(-delta);
			}

			if(_dlgAnalysis && _dlgAnalysis->isVisible())
			{
				_deColorMin->setVisible(true);
				_deColorMax->setVisible(true);

				_dsColorMin->setVisible(false);
				_dsColorMax->setVisible(false);
			}
		}
		else
		{
			float min = 0;
			float max = 0;

			switch(_cbColorIndicator->currentIndex())
			{
			case 0:
			case 1:
			case 2:
			case 3:
				break;
			case 4:
				min = Global::schedule1->getMinBaselineDuration();
				max = Global::schedule1->getMaxBaselineDuration();
				break;
			case 5:
				min = Global::schedule1->getMinActualDuration();
				max = Global::schedule1->getMaxActualDuration();
				break;
			case 6:
				min = Global::schedule1->getMinDeltaDuration();
				max = Global::schedule1->getMaxDeltaDuration();
				break;
			case 7:
				min = Global::schedule1->getMinDeltaStart();
				max = Global::schedule1->getMaxDeltaStart();
				break;
			case 8:
				min = Global::schedule1->getMinDeltaFinish();
				max = Global::schedule1->getMaxDeltaFinish();
				break;
			case 9:
				min = Global::schedule1->getMinFreeSlack();
				max = Global::schedule1->getMaxFreeSlack();
				break;
			case 10:
				min = Global::schedule1->getMinTotalSlack();
				max = Global::schedule1->getMaxTotalSlack();
				break;
			case 11:
				min = Global::schedule1->getMinCustomMetric();
				max = Global::schedule1->getMaxCustomMetric();
				break;
			default:
				throw std::exception();
			}

			_dsColorMin->setRange(min, max);
			_dsColorMin->setValue(min);

			_dsColorMax->setRange(min, max);
			_dsColorMax->setValue(max);

			float value = max;
			float delta = (max-min)/_colorScale.getNumColors();
			for(unsigned int i = 0; i < _colorScale.getNumColors()+1; ++i)
			{
				auto str = QString::number(value, 'f', 2);
				if(str == "-0.00") str = "0.00";
				_colorScale.setLabel(i, str);
				value -= delta;
			}

			if(_dlgAnalysis && _dlgAnalysis->isVisible())
			{
				_dsColorMin->setVisible(true);
				_dsColorMax->setVisible(true);

				_deColorMin->setVisible(false);
				_deColorMax->setVisible(false);
			}
		}

		// adjust color scale position on screen
		Global::glscene->refreshOverlayWidgets();

		// for each task from schedule, get indicator value, map to [min,max] range, find appropriate color in current color scheme and set on corresponding drawables
		auto tasks = Global::schedule1->getTasksByTaskName(Global::explodedview->getActiveTaskName());

		for(auto t : tasks)
		{
			int colorIdx = 0;
			switch(_cbColorIndicator->currentIndex())
			{
			case 0:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->baselineStart) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 1:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->baselineFinish) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 2:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->actualStart) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 3:
				colorIdx = _colorScale.getNumColors() * (float)_deColorMin->date().daysTo(t->actualFinish) / (float)_deColorMin->date().daysTo(_deColorMax->date());
				break;
			case 4:
				colorIdx = _colorScale.getNumColors() * (float)(t->getBaselineDuration() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 5:
				colorIdx = _colorScale.getNumColors() * (float)(t->getActualDuration() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 6:
				colorIdx = _colorScale.getNumColors() * (float)(t->getDeltaDuration() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 7:
				colorIdx = _colorScale.getNumColors() * (float)(t->getDeltaStart() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 8:
				colorIdx = _colorScale.getNumColors() * (float)(t->getDeltaFinish() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 9:
				colorIdx = _colorScale.getNumColors() * (float)(t->freeSlack - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 10:
				colorIdx = _colorScale.getNumColors() * (float)(t->totalSlack - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			case 11:
				colorIdx = _colorScale.getNumColors() * (float)(t->getCustomMetric() - _dsColorMin->value()) / (float)(_dsColorMax->value() - _dsColorMin->value());
				break;
			default:
				throw std::exception();
			}
			colorIdx = colorIdx < 0? 0 : (colorIdx >= (int)_colorScale.getNumColors()? _colorScale.getNumColors() - 1 : colorIdx);
			auto colorID = currScheme.at(_colorScale.getNumColors()-(colorIdx+1)).id; // invert color idx: lower idx means higher value

			vector<unsigned int> drawables;
			AABB bound;
			Global::hierarchy->getRenderData(t->objectName, drawables, bound);

			for(auto d : drawables)
			{
//				auto c = Global::renderer->getDrawableColorID(d);
//				io::print(c);
//				_originalDrawableColors.push_back({d, c});
				Global::renderer->setDrawableColorID(d, colorID);
			}
		}
	}

	void Analysis::_updateOSLines()
	{
		for(auto d : _osLines)
		{
			Global::renderer->setLineDrawableVisible(d, false);
		}
		Global::renderer->setDrawablesHighlight(_osDrawables.data(), _osDrawables.size(), Renderer::HighlightOrange, false);
		_osDrawables.clear();

		if(!_showOSLines || !Global::explodedview->isActive())
		{
			return;
		}

		// gather task objects with the same os
		const auto& osTasks = Global::schedule1->getTasksByOS();

		const auto& selection = Global::selection->getSelection();

		// highlight objects of the same os and draw lines between them
		int lineIdx = 0;
		auto keys = osTasks.uniqueKeys();
		for(auto k : keys)
		{
			auto values = osTasks.values(k);
			for(int i = 0; i < values.size(); ++i)
			{
				auto v = values.at(i);

				AABB bound;
				vector<unsigned int> drawables;
				Global::hierarchy->getRenderData(v->objectName, drawables, bound);

				if(drawables.empty() || !Global::renderer->isDrawableVisible(drawables.at(0)) || !selection.contains(drawables.at(0)) || Global::renderer->isDuplicateDrawable(drawables.at(0)))
				{
					continue;
				}

				for(auto d : drawables)
				{
					_osDrawables.push_back(d);
				}
				Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightOrange, true);

				bound = Global::getUpdatedBound(drawables);

				vector<TaskRef> others;
				for(int k = 0; k < values.size(); ++k)
				{
					if(k != i)
					{
						others.push_back(values.at(k));
					}
				}

				highlight(bound, others, _osLines, lineIdx, Renderer::HighlightOrange, _osDrawables);
			}
		}
	}
}
