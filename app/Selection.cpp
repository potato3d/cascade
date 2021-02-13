#include <app/Selection.h>
#include <app/Global.h>
#include <QApplication>
#include <QLabel>
#include <QLayout>
#include <QFrame>
#include <QHeaderView>


namespace app
{
	static const int COL_KEY = 0;
	static const int COL_VALUE = 1;
	static const int ROW_ID = 0;
	static const int ROW_TASK_NAME = 1;
	static const int ROW_OBJ_NAME = 2;
	static const int ROW_BASE_START = 3;
	static const int ROW_BASE_FINISH = 4;
	static const int ROW_ACT_START = 5;
	static const int ROW_ACT_FINISH = 6;
	static const int ROW_FREE_SLACK = 7;
	static const int ROW_TOTAL_SLACK = 8;
	static const int ROW_OS = 9;
	static const int ROW_CRITICAL = 10;
	static const int ROW_CUSTOM_METRIC = 11;

	static const vector<QString> INDICATORS = {"Task Name", "Object Name", "OS", "Critical", "Baseline Start", "Baseline Finish", "Actual Start", "Actual Finish",
											   "Baseline Duration", "Actual Duration", "Delta Duration", "Delta Start", "Delta Finish",
											   "Free Slack", "Total Slack", "Custom Metric"};

	QTableWidgetItem* createTableKey(const QString& text)
	{
		auto item = new QTableWidgetItem(text);
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		return item;
	}

	QTableWidgetItem* createTableValue(const QString& text, bool italicText = false)
	{
		auto item = new QTableWidgetItem(text);
		item->setTextAlignment(Qt::AlignRight);
		if(italicText)
		{
			auto f = item->font();
			f.setItalic(true);
			item->setFont(f);
		}
		item->setFlags(item->flags() ^ Qt::ItemIsEditable);
		return item;
	}

	void Selection::init()
	{
		_actSelection = Global::toolbar->addAction("Selection", this, SLOT(_onActionSelectionTriggered(bool)));
		_actSelection->setCheckable(true);

		_actMetadata = Global::toolbar->addAction("Metadata", this, SLOT(_onActionMetadataTriggered(bool)));
		_actMetadata->setCheckable(true);

		for(int i = 0; i < 1e3; ++i)
		{
			auto l = new QLabel();
			l->setStyleSheet(Global::getLabelStyle());
			_drawableIndicators.push_back(l);
			Global::overlay->addWidget(l, vec3::ZERO);
			l->setVisible(false);
		}

		addSelectionCallback("Selection Highlight", [=](bool checked)
		{
			_showSelectionHighlight = checked;

			vector<unsigned int> drawables;
			for(auto s : _selection)
			{
				drawables.push_back(s);
			}
			Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightCyan, false);

			if(!_showSelectionHighlight)
			{
				return;
			}

			Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightCyan, true);
		}, _showSelectionHighlight);

		addSelectionCallback("Others Invisible", [=](bool checked)
		{
			if(checked)
			{
				_setOthersInvisible();
			}
			else
			{
				_setAllVisible();
			}
		}, false);

		addSelectionCallback("Others Transparent", [=](bool checked)
		{
			if(checked)
			{
				_setOthersTransparent();
			}
			else
			{
				_setAllOpaque();
			}
		}, false);

		addSelectionCallback("Metadata", [=](bool checked)
		{
			_showDrawableIndicators = checked;
			_updateDrawableIndicators();
		}, _showDrawableIndicators);

		connect(Global::glscene, SIGNAL(mousePressed(QGraphicsSceneMouseEvent*)), this, SLOT(_onMousePressed(QGraphicsSceneMouseEvent*)));
		connect(Global::glscene, SIGNAL(mouseReleased(QGraphicsSceneMouseEvent*)), this, SLOT(_onMouseReleased(QGraphicsSceneMouseEvent*)));
		connect(Global::glscene, SIGNAL(mouseDoubleClicked(QGraphicsSceneMouseEvent*)), this, SLOT(_onMouseDoubleClicked(QGraphicsSceneMouseEvent*)));

		connect(Global::explodedview, SIGNAL(plotChanged()), this, SLOT(_onPlotChanged()));
	}

	void Selection::saveState()
	{
		for(auto w : _callbackWidgets)
		{
			Global::settings->setValue("Selection/" + w.widget->text(), QVariant::fromValue(w.widget->isChecked()));
		}
	}

	void Selection::restoreState()
	{
		auto keys = Global::settings->allKeys();
		for(auto k : keys)
		{
			if(k.startsWith("Selection/"))
			{
				for(auto w : _callbackWidgets)
				{
					if(k.contains(w.widget->text()))
					{
						w.widget->setChecked(Global::settings->value(k).toBool());
					}
				}
			}
		}
	}

	void Selection::setCustomSelection(function<void(QGraphicsSceneMouseEvent*)> customFunc)
	{
		_customSelection = customFunc;
	}

	void Selection::pivotOnSelection()
	{
		auto bounds = _getSelectionBounds();
		if(bounds.isValid())
		{
			Global::camera->setPivot(bounds);
			_justFocused = true;
		}
	}

	void Selection::focusOnSelection()
	{
		auto bounds = _getSelectionBounds();
		if(bounds.isValid())
		{
			Global::camera->setFocus(bounds);
			_justFocused = true;
		}
	}

	void Selection::setSelection(const vector<unsigned int>& drawables)
	{
		if(_showSelectionHighlight)
		{
			vector<unsigned int> prevDrawables;
			for(const auto& item : _selection)
			{
				prevDrawables.push_back(item);
			}
			Global::renderer->setDrawablesHighlight(prevDrawables.data(), prevDrawables.size(), Renderer::HighlightCyan, false);
		}
		_selection.clear();

		for(auto d : drawables)
		{
			_selection.insert(d);
		}
		if(_showSelectionHighlight)
		{
			Global::renderer->setDrawablesHighlight(drawables.data(), drawables.size(), Renderer::HighlightCyan, true);
		}
		_updateDlgSelection();
		emit selectionChanged();
	}

	bool Selection::toggleSelection(const vector<unsigned int>& drawables)
	{
		bool anySelected = false;
		vector<unsigned int> oldDrawables;
		vector<unsigned int> newDrawables;
		for(auto d : drawables)
		{
			bool wasSelected = _selection.remove(d);
			if(!wasSelected)
			{
				_selection.insert(d);
				newDrawables.push_back(d);
			}
			else if(_showSelectionHighlight)
			{
				oldDrawables.push_back(d);
			}
			anySelected |= wasSelected;
		}
		if(_showSelectionHighlight)
		{
			if(!oldDrawables.empty())
			{
				Global::renderer->setDrawablesHighlight(oldDrawables.data(), oldDrawables.size(), Renderer::HighlightCyan, false);
			}
			if(!newDrawables.empty())
			{
				Global::renderer->setDrawablesHighlight(newDrawables.data(), newDrawables.size(), Renderer::HighlightCyan, true);
			}
		}
		_updateDlgSelection();
		emit selectionChanged();
		return anySelected;
	}

	const QSet<unsigned int>& Selection::getSelection()
	{
		return _selection;
	}

	void Selection::addSelectionCallback(const string& name, std::function<void(bool)> callback, bool enabled /*= true*/)
	{
		auto ck = new QCheckBox(QString::fromStdString(name));
		ck->setChecked(enabled);
		connect(ck, &QCheckBox::toggled, [=](bool checked)
		{
			callback(checked);
		});
		_callbackWidgets.push_back({ck, callback});
		if(_dlgSelection)
		{
			_dlgSelection->layout()->addWidget(ck);
		}
	}

	void Selection::onHierarchySelectionChanged()
	{
		_onHierarchySelectionChanged();
	}

	void Selection::_onPlotChanged()
	{
		_updateDrawableIndicators();
	}

	void Selection::_onMousePressed(QGraphicsSceneMouseEvent* e)
	{
		_mousePressPos = e->scenePos();
		e->ignore();
	}

	void Selection::_onMouseReleased(QGraphicsSceneMouseEvent* e)
	{
		if((e->scenePos() - _mousePressPos).manhattanLength() > 2)
		{
			e->ignore();
			return;
		}

		if(_justFocused)
		{
			_justFocused = false;
			e->ignore();
			return;
		}

		if(e->modifiers() & Qt::ShiftModifier || e->modifiers() & Qt::AltModifier)
		{
			e->ignore();
			return;
		}

		if(_customSelection)
		{
			_customSelection(e);
			return;
		}

//		auto drawableID = Global::renderer->pickDrawable(e->scenePos().x(), Global::glscene->height() - e->scenePos().y());

//		if(drawableID >= Global::renderer->getFirstSceneDrawable())
//		{
//			if(e->modifiers() & Qt::ControlModifier)
//			{
//				// toggle selection
//				auto itr = _selection.find(drawableID);
//				if(itr != _selection.end())
//				{
//					// deselect
//					if(_showSelectionHighlight)
//					{
//						Global::renderer->setDrawableHighlight(*itr, Renderer::HighlightCyan, false);
//					}
//					_selection.erase(itr);
//				}
//				else
//				{
//					// select
//					_select(drawableID);
//				}
//			}
//			else
//			{
//				// deselect all and select new
//				_deselectAll();
//				_select(drawableID);
//			}
//		}
//		else
//		{
//			// deselect all
//			_deselectAll();
//		}

//		_updateDlgSelection();

//		emit selectionChanged();
//		e->accept();
	}

	void Selection::_onMouseDoubleClicked(QGraphicsSceneMouseEvent* e)
	{
		if(_selection.isEmpty())
		{
			e->ignore();
			return;
		}

		if(e->buttons() & Qt::LeftButton)
		{
			focusOnSelection();
			e->accept();
		}
		else if(e->buttons() & Qt::RightButton)
		{
			pivotOnSelection();
			e->accept();
		}
		else
		{
			e->ignore();
		}
	}

	void Selection::_onActionSelectionTriggered(bool checked)
	{
		if(!_dlgSelection)
		{
			_ckVisible = new QCheckBox("Visible");
			_ckVisible->setEnabled(!_selection.isEmpty());
			_ckOpaque = new QCheckBox("Opaque");
			_ckOpaque->setEnabled(!_selection.isEmpty());
			_pbOthersInvisible = new QPushButton("Others Invisible");
			_pbOthersInvisible->setEnabled(!_selection.isEmpty());
			_pbOthersTransparent = new QPushButton("Others Transparent");
			_pbOthersTransparent->setEnabled(!_selection.isEmpty());
			_pbAllVisible = new QPushButton("All Visible");
			_pbAllOpaque = new QPushButton("All Opaque");
			_slTransparency = new QSlider(Qt::Horizontal);
			_slTransparency->setMinimum(10);
			_slTransparency->setMaximum(99);
			_slTransparency->setValue(Global::renderer->getTransparencyLevel()*100);

			const auto& selection = Global::selection->getSelection();
			bool anyInvisible = false;
			bool anyTransparent = false;
			for(auto d : selection)
			{
				anyInvisible |= !Global::renderer->isDrawableVisible(d);
				anyTransparent |= Global::renderer->isDrawableTransparent(d);
				if(anyInvisible && anyTransparent)
				{
					break;
				}
			}
			_ckVisible->setChecked(!anyInvisible);
			_ckOpaque->setChecked(!anyTransparent);

			connect(_ckVisible, &QCheckBox::toggled, [=](bool checked)
			{
				for(auto s : _selection)
				{
					Global::renderer->setDrawableVisible(s, checked);
				}
			});

			connect(_ckOpaque, &QCheckBox::toggled, [=](bool checked)
			{
				for(auto s : _selection)
				{
					Global::renderer->setDrawableTransparent(s, !checked);
				}
			});

			connect(_pbOthersInvisible, &QPushButton::clicked, [=]
			{
				_setOthersInvisible();
			});

			connect(_pbOthersTransparent, &QPushButton::clicked, [=]
			{
				_setOthersTransparent();
			});

			connect(_pbAllVisible, &QPushButton::clicked, [=]
			{
				_setAllVisible();
			});

			connect(_pbAllOpaque, &QPushButton::clicked, [=]
			{
				_setAllOpaque();
			});

			connect(_slTransparency, &QSlider::valueChanged, [=](int value)
			{
				Global::renderer->setTransparencyLevel((float)value/100.0f);
			});

			_dlgSelection = new CollapsableDialog("Selection");

			_dlgSelection->layout()->addWidget(_ckVisible);
			_dlgSelection->layout()->addWidget(_pbOthersInvisible);
			_dlgSelection->layout()->addWidget(_pbAllVisible);
			_dlgSelection->layout()->addWidget(Global::createSeparator());
			_dlgSelection->layout()->addWidget(_ckOpaque);
			_dlgSelection->layout()->addWidget(_pbOthersTransparent);
			_dlgSelection->layout()->addWidget(_pbAllOpaque);
			_dlgSelection->layout()->addWidget(Global::createSeparator());

			_dlgSelection->layout()->addWidget(_slTransparency);

			_dlgSelection->layout()->addWidget(Global::createSeparator());

			for(auto w : _callbackWidgets)
			{
				_dlgSelection->layout()->addWidget(w.widget);
			}

			Global::glscene->addDialog(_dlgSelection);
		}
		_dlgSelection->setVisible(checked);
	}

	void Selection::_onActionMetadataTriggered(bool checked)
	{
		if(!_dlgMetadata)
		{
			_twMetadata = new QTableWidget();
			_twMetadata->setColumnCount(2);
			_twMetadata->setRowCount(12);
			_twMetadata->setHorizontalHeaderLabels({"Key", "Value"});
			_twMetadata->horizontalHeader()->setStretchLastSection(true);
			_twMetadata->verticalHeader()->hide();

			_twMetadata->setItem(ROW_ID, COL_KEY, createTableKey("ID"));
			_twMetadata->setItem(ROW_TASK_NAME, COL_KEY, createTableKey("Task Name"));
			_twMetadata->setItem(ROW_OBJ_NAME, COL_KEY, createTableKey("Object Name"));
			_twMetadata->setItem(ROW_BASE_START, COL_KEY, createTableKey("Baseline Start"));
			_twMetadata->setItem(ROW_BASE_FINISH, COL_KEY, createTableKey("Baseline Finish"));
			_twMetadata->setItem(ROW_ACT_START, COL_KEY, createTableKey("Actual Start"));
			_twMetadata->setItem(ROW_ACT_FINISH, COL_KEY, createTableKey("Actual Finish"));
			_twMetadata->setItem(ROW_FREE_SLACK, COL_KEY, createTableKey("Free Slack"));
			_twMetadata->setItem(ROW_TOTAL_SLACK, COL_KEY, createTableKey("Total Slack"));
			_twMetadata->setItem(ROW_OS, COL_KEY, createTableKey("Operating System"));
			_twMetadata->setItem(ROW_CRITICAL, COL_KEY, createTableKey("Critical"));
			_twMetadata->setItem(ROW_CUSTOM_METRIC, COL_KEY, createTableKey("Custom Metric"));

			_twMetadata->resizeColumnsToContents();
			_twMetadata->resizeRowsToContents();

			_twMetadata->setEnabled(false);

			_cbIndicator = new QComboBox();
			for(unsigned int i = 0; i < INDICATORS.size(); ++i)
			{
				_cbIndicator->addItem(INDICATORS.at(i));
			}

			connect(_cbIndicator, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=]
			{
				_updateDrawableIndicators();
			});

			_dlgMetadata = new CollapsableDialog("Metadata");
			_dlgMetadata->layout()->addWidget(_cbIndicator);
			_dlgMetadata->layout()->addWidget(_twMetadata);
			_dlgMetadata->resize(_twMetadata->columnWidth(0) + _twMetadata->columnWidth(1) + 10, (_twMetadata->rowHeight(0)+1)*14);

			Global::glscene->addDialog(_dlgMetadata);

//			connect(Global::hierarchy, SIGNAL(selectionChanged()), this, SLOT(_onHierarchySelectionChanged()));
			_onHierarchySelectionChanged();
		}
		_dlgMetadata->setVisible(checked);
	}

	void Selection::_onHierarchySelectionChanged()
	{
		if(!_dlgMetadata)
		{
			return;
		}

		auto selection = Global::hierarchy->getSelection();
		if(selection.isEmpty())
		{
			_clearMetadata();
			return;
		}
		else if(selection.size() == 1)
		{
			auto task = Global::schedule1->getTaskByObjectName(selection.at(0)->text(0));
			if(!task)
			{
				_clearMetadata();
				return;
			}

			_twMetadata->setItem(ROW_ID, COL_VALUE, createTableValue(QString::number(task->id)));
			_twMetadata->setItem(ROW_TASK_NAME, COL_VALUE, createTableValue(task->taskName));
			_twMetadata->setItem(ROW_OBJ_NAME, COL_VALUE, createTableValue(task->objectName));
			_twMetadata->setItem(ROW_BASE_START, COL_VALUE, createTableValue(task->baselineStart.toString("MMM dd yyyy")));
			_twMetadata->setItem(ROW_BASE_FINISH, COL_VALUE, createTableValue(task->baselineFinish.toString("MMM dd yyyy")));
			_twMetadata->setItem(ROW_ACT_START, COL_VALUE, createTableValue(task->actualStart.toString("MMM dd yyyy")));
			_twMetadata->setItem(ROW_ACT_FINISH, COL_VALUE, createTableValue(task->actualFinish.toString("MMM dd yyyy")));
			_twMetadata->setItem(ROW_FREE_SLACK, COL_VALUE, createTableValue(QString::number(task->freeSlack)));
			_twMetadata->setItem(ROW_TOTAL_SLACK, COL_VALUE, createTableValue(QString::number(task->totalSlack)));
			_twMetadata->setItem(ROW_OS, COL_VALUE, createTableValue(task->os));
			_twMetadata->setItem(ROW_CRITICAL, COL_VALUE, createTableValue(task->isCritical? "Yes" : "No"));
			_twMetadata->setItem(ROW_CUSTOM_METRIC, COL_VALUE, createTableValue(QString::number(task->getCustomMetric())));
		}
		else
		{
			bool sameID = true;
			bool sameTaskName = true;
			bool sameObjName = true;
			bool sameBaseStart = true;
			bool sameBaseFinish = true;
			bool sameActualStart = true;
			bool sameActualFinish = true;
			bool sameFreeSlack = true;
			bool sameTotalSlack = true;
			bool sameOS = true;
			bool sameCritical = true;
			bool sameCustomMetric = true;

			auto task = Global::schedule1->getTaskByObjectName((*selection.begin())->text(0));

			for(auto itr = selection.begin()+1; itr != selection.end(); ++itr)
			{
				auto t = Global::schedule1->getTaskByObjectName((*itr)->text(0));
				sameID &= !t? false : (!sameID? false : task->id == t->id);
				sameTaskName &= !t? false : (!sameTaskName? false : task->taskName == t->taskName);
				sameObjName &= !t? false : (!sameObjName? false : task->objectName == t->objectName);
				sameBaseStart &= !t? false : (!sameBaseStart? false : task->baselineStart == t->baselineStart);
				sameBaseFinish &= !t? false : (!sameBaseFinish? false : task->baselineFinish == t->baselineFinish);
				sameActualStart &= !t? false : (!sameActualStart? false : task->actualStart == t->actualStart);
				sameActualFinish &= !t? false : (!sameActualFinish? false : task->actualFinish == t->actualFinish);
				sameFreeSlack &= !t? false : (!sameFreeSlack? false : task->freeSlack == t->freeSlack);
				sameTotalSlack &= !t? false : (!sameTotalSlack? false : task->totalSlack == t->totalSlack);
				sameOS &= !t? false : (!sameOS? false : task->os == t->os);
				sameCritical &= !t? false : (!sameCritical? false : task->isCritical == t->isCritical);
				sameCustomMetric &= !t? false : (!sameCustomMetric? false : task->getCustomMetric() == t->getCustomMetric());
			}

			_twMetadata->setItem(ROW_ID, COL_VALUE, createTableValue(sameID? QString::number(task->id) : "...different...", !sameID));
			_twMetadata->setItem(ROW_TASK_NAME, COL_VALUE, createTableValue(sameTaskName? task->taskName : "...different...", !sameTaskName));
			_twMetadata->setItem(ROW_OBJ_NAME, COL_VALUE, createTableValue(sameObjName? task->objectName : "...different...", !sameObjName));
			_twMetadata->setItem(ROW_BASE_START, COL_VALUE, createTableValue(sameBaseStart? task->baselineStart.toString("MMM dd yyyy") : "...different...", !sameBaseStart));
			_twMetadata->setItem(ROW_BASE_FINISH, COL_VALUE, createTableValue(sameBaseFinish? task->baselineFinish.toString("MMM dd yyyy") : "...different...", !sameBaseFinish));
			_twMetadata->setItem(ROW_ACT_START, COL_VALUE, createTableValue(sameActualStart? task->actualStart.toString("MMM dd yyyy") : "...different...", !sameActualStart));
			_twMetadata->setItem(ROW_ACT_FINISH, COL_VALUE, createTableValue(sameActualFinish? task->actualFinish.toString("MMM dd yyyy") : "...different...", !sameActualFinish));
			_twMetadata->setItem(ROW_FREE_SLACK, COL_VALUE, createTableValue(sameFreeSlack? QString::number(task->freeSlack) : "...different...", !sameFreeSlack));
			_twMetadata->setItem(ROW_TOTAL_SLACK, COL_VALUE, createTableValue(sameTotalSlack? QString::number(task->totalSlack) : "...different...", !sameTotalSlack));
			_twMetadata->setItem(ROW_OS, COL_VALUE, createTableValue(sameOS? task->os : "...different...", !sameOS));
			_twMetadata->setItem(ROW_CRITICAL, COL_VALUE, createTableValue(sameCritical? (task->isCritical? "Yes" : "No") : "...different...", !sameCritical));
			_twMetadata->setItem(ROW_CUSTOM_METRIC, COL_VALUE, createTableValue(sameCustomMetric? QString::number(task->getCustomMetric()) : "...different...", !sameCustomMetric));
		}

		_twMetadata->setEnabled(true);
		_twMetadata->resizeColumnsToContents();
		_twMetadata->resizeRowsToContents();
		_dlgMetadata->resize(_twMetadata->columnWidth(0) + _twMetadata->columnWidth(1) + 10, (_twMetadata->rowHeight(0)+1)*14);
	}

	void Selection::_clearMetadata()
	{
		_twMetadata->setEnabled(false);
		for(int i = 0; i <= ROW_CUSTOM_METRIC; ++i)
		{
			_twMetadata->setItem(i, COL_VALUE, createTableValue(""));
		}
	}

	AABB Selection::_getSelectionBounds()
	{
		AABB bounds;
		for(const auto& item : _selection)
		{
			bounds.expand(Global::renderer->getDrawableBounds(item));
		}
		return bounds;
	}

	void Selection::_setOthersInvisible()
	{
		auto first = Global::renderer->getFirstSceneDrawable();
		auto last = Global::renderer->getLastSceneDrawable();
		if(_selection.isEmpty())
		{
			for(unsigned int i = first; i <= last; ++i)
			{
				Global::renderer->setDrawableVisible(i, true);
			}
		}
		else
		{
			for(unsigned int i = first; i <= last; ++i)
			{
				Global::renderer->setDrawableVisible(i, false);
			}
			for(auto s : _selection)
			{
				Global::renderer->setDrawableVisible(s, true);
			}
		}
	}

	void Selection::_setOthersTransparent()
	{
		auto first = Global::renderer->getFirstSceneDrawable();
		auto last = Global::renderer->getLastSceneDrawable();
		if(_selection.isEmpty())
		{
			for(unsigned int i = first; i <= last; ++i)
			{
				Global::renderer->setDrawableTransparent(i, false);
			}
		}
		else
		{
			for(unsigned int i = first; i <= last; ++i)
			{
				Global::renderer->setDrawableTransparent(i, true);
			}
			for(auto s : _selection)
			{
				Global::renderer->setDrawableTransparent(s, false);
			}
		}
	}

	void Selection::_setAllVisible()
	{
		auto first = Global::renderer->getFirstSceneDrawable();
		auto last = Global::renderer->getLastSceneDrawable();
		for(unsigned int i = first; i <= last; ++i)
		{
			Global::renderer->setDrawableVisible(i, true);
		}
	}

	void Selection::_setAllOpaque()
	{
		auto first = Global::renderer->getFirstSceneDrawable();
		auto last = Global::renderer->getLastSceneDrawable();
		for(unsigned int i = first; i <= last; ++i)
		{
			Global::renderer->setDrawableTransparent(i, false);
		}
	}

	void Selection::_updateDlgSelection()
	{
		if(_dlgSelection)
		{
			_ckVisible->setEnabled(!_selection.isEmpty());
			_ckOpaque->setEnabled(!_selection.isEmpty());
			_pbOthersInvisible->setEnabled(!_selection.isEmpty());
			_pbOthersTransparent->setEnabled(!_selection.isEmpty());

			if(!_selection.isEmpty())
			{
				QSignalBlocker sb(_ckVisible);
				QSignalBlocker sb2(_ckOpaque);
				_ckVisible->setChecked(true);
				_ckOpaque->setChecked(true);
				for(auto s : _selection)
				{
					if(!Global::renderer->isDrawableVisible(s))
					{
						_ckVisible->setChecked(false);
					}
					if(Global::renderer->isDrawableTransparent(s))
					{
						_ckOpaque->setChecked(false);
					}
					if(!_ckVisible->isChecked() && !_ckOpaque->isChecked())
					{
						break;
					}
				}
			}
		}

//		for(auto w : _callbackWidgets)
//		{
//			if(w.widget->isChecked())
//			{
//				w.callback(true);
//			}
//		}
	}

	void Selection::_updateDrawableIndicators()
	{
		for(auto l : _drawableIndicators)
		{
			l->setVisible(false);
		}

		const auto& selection = Global::selection->getSelection();
		if(!_showDrawableIndicators || selection.isEmpty())
		{
			return;
		}

		int labelIdx = 0;

		auto tasks = Global::schedule1->getTasksByTaskName(Global::explodedview->getActiveTaskName());
		auto tasks2 = Global::schedule2->getTasksByTaskName(Global::explodedview->getActiveTaskName());

		for(int i = 0; i < tasks.size(); ++i)
		{
			auto t = tasks.at(i);
			auto t2 = tasks2.at(i);

			vector<unsigned int> drawables;
			AABB bound;
			Global::hierarchy->getRenderData(t->objectName, drawables, bound);

			if(drawables.empty())
			{
				continue;
			}

			if(selection.contains(drawables.at(0)))
			{
				auto l = _drawableIndicators.at(labelIdx);

				switch(_cbIndicator->currentIndex())
				{
				case 0:
					l->setText(t->taskName);
					break;
				case 1:
					l->setText(t->objectName);
					break;
				case 2:
					l->setText(t->os);
					break;
				case 3:
				{
#ifdef _WIN32
//					QString path = "D:/develop/doctorate/sources/app_schedule3d/";
					QString path = "C:/develop/doctorate/sources/app_schedule3d/";
#else
					QString path = "/home/potato/Develop/doctorate/sources/app_schedule3d/";
#endif
					if(t->isCritical)
					{
						path += "Exclamation-icon.png";
					}
					else
					{
						path += "Check-icon.png";
					}
					QPixmap p(path);
					l->setPixmap(p.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
					l->setStyleSheet("QLabel{font: 9pt; background-color:transparent;}");
					break;
				}
				case 4:
					l->setText(t->baselineStart.toString("MMM dd yyyy"));
					break;
				case 5:
					l->setText(t->baselineFinish.toString("MMM dd yyyy"));
					break;
				case 6:
					l->setText(t->actualStart.toString("MMM dd yyyy"));
					break;
				case 7:
					l->setText(t->actualFinish.toString("MMM dd yyyy"));
					break;
				case 8:
					l->setText(QString::number(t->getBaselineDuration()));
					break;
				case 9:
					l->setText(QString::number(t->getActualDuration()));
					break;
				case 10:
					l->setText(QString::number(t->getDeltaDuration()));
					break;
				case 11:
					l->setText(QString::number(t->getDeltaStart()));
					break;
				case 12:
					l->setText(QString::number(t->getDeltaFinish()));
					break;
				case 13:
					l->setText(QString::number(t->freeSlack, 'f', 2));
					break;
				case 14:
					l->setText(QString::number(t->totalSlack, 'f', 2));
					break;
				case 15:
					l->setText(QString::number(t->getCustomMetric(), 'f', 2));
					break;
				default:
					throw std::exception();
				}

				if(_cbIndicator->currentIndex() != 3)
				{
					l->setStyleSheet(Global::getLabelStyle());
				}

				l->adjustSize();

				bound = Global::getUpdatedBound(drawables);
				auto s = bound.getSize();

				Global::overlay->setWidgetPosition(l, vec3(bound.max.x-s.x*0.25f, bound.max.y-s.y*0.25f, bound.min.z+s.z*0.25f));
				l->setVisible(true);

				++labelIdx;
			}

			// -----------------------------

			for(auto& d : drawables)
			{
				d = Global::renderer->getDuplicateDrawableID(d);
			}

			if(selection.contains(drawables.at(0)))
			{
				auto l = _drawableIndicators.at(labelIdx);

				switch(_cbIndicator->currentIndex())
				{
				case 0:
					l->setText(t2->taskName);
					break;
				case 1:
					l->setText(t2->objectName);
					break;
				case 2:
					l->setText(t2->os);
					break;
				case 3:
				{
#ifdef _WIN32
					QString path = "c:/develop/doctorate/sources/app_schedule3d/";
#else
					QString path = "/home/potato/Develop/doctorate/sources/app_schedule3d/";
#endif
					if(t2->isCritical)
					{
						path += "Exclamation-icon.png";
					}
					else
					{
						path += "Check-icon.png";
					}
					QPixmap p(path);
					l->setPixmap(p.scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
					l->setStyleSheet("QLabel{font: 9pt; background-color:transparent;}");
					break;
				}
				case 4:
					l->setText(t2->baselineStart.toString("MMM dd yyyy"));
					break;
				case 5:
					l->setText(t2->baselineFinish.toString("MMM dd yyyy"));
					break;
				case 6:
					l->setText(t2->actualStart.toString("MMM dd yyyy"));
					break;
				case 7:
					l->setText(t2->actualFinish.toString("MMM dd yyyy"));
					break;
				case 8:
					l->setText(QString::number(t2->getBaselineDuration()));
					break;
				case 9:
					l->setText(QString::number(t2->getActualDuration()));
					break;
				case 10:
					l->setText(QString::number(t2->getDeltaDuration()));
					break;
				case 11:
					l->setText(QString::number(t2->getDeltaStart()));
					break;
				case 12:
					l->setText(QString::number(t2->getDeltaFinish()));
					break;
				case 13:
					l->setText(QString::number(t2->freeSlack, 'f', 2));
					break;
				case 14:
					l->setText(QString::number(t2->totalSlack, 'f', 2));
					break;
				case 15:
					l->setText(QString::number(t2->getCustomMetric(), 'f', 2));
					break;
				default:
					throw std::exception();
				}

				if(_cbIndicator->currentIndex() != 3)
				{
					l->setStyleSheet(Global::getLabelStyle());
				}

				l->adjustSize();

				bound = Global::getUpdatedBound(drawables);
				auto s = bound.getSize();

				Global::overlay->setWidgetPosition(l, vec3(bound.max.x-s.x*0.25f, bound.max.y-s.y*0.25f, bound.min.z+s.z*0.25f));
				l->setVisible(true);

				++labelIdx;
			}
		}
	}
}
