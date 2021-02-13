#include <app/Hierarchy.h>
#include <app/Global.h>
#include <QLayout>
#include <QHeaderView>
#include <QApplication>

Q_DECLARE_METATYPE(app::AABB)

namespace app
{
	void Hierarchy::init()
	{
		_action = Global::toolbar->addAction("Hierarchy", this, SLOT(_onHierarchyActionTriggered(bool)));
		_action->setCheckable(true);
		connect(Global::loader, SIGNAL(modelLoaded()), this, SLOT(_onModelLoaded()));

		Global::selection->setCustomSelection([=](QGraphicsSceneMouseEvent* e)
		{
			QSignalBlocker sb(_treeWidget);

			vector<unsigned int> drawablesToSelect;
			QTreeWidgetItem* itemToSelect = nullptr;

			auto drawableID = Global::renderer->pickDrawable(e->scenePos().x(), Global::glscene->height() - e->scenePos().y());

			if(drawableID >= Global::renderer->getFirstSceneDrawable())
			{
				itemToSelect = _drawableToTreeItem.value(Global::renderer->getOriginalDrawableID(drawableID));
				if(e->button() == Qt::LeftButton)
				{
					const auto& taskObjs = Global::schedule1->getTaskObjectNames();
					auto newItem = itemToSelect;
					while(newItem != nullptr && !taskObjs.contains(newItem->text(0)))
					{
						newItem = newItem->parent();
					}
					if(newItem != nullptr)
					{
						itemToSelect = newItem;
					}
				}

				vector<unsigned int> drawables;
				AABB bound;
				getRenderData(itemToSelect, drawables, bound);

				if(Global::renderer->isDuplicateDrawable(drawableID))
				{
					for(auto dr : drawables)
					{
						drawablesToSelect.push_back(Global::renderer->getDuplicateDrawableID(dr));
					}
					itemToSelect = nullptr;
				}
				else
				{
					for(auto dr : drawables)
					{
						drawablesToSelect.push_back(dr);
					}
				}
			}
			else
			{
				_treeWidget->clearSelection();
			}

			if(e->modifiers() & Qt::ControlModifier)
			{
				auto wasSelected = Global::selection->toggleSelection(drawablesToSelect);
				if(itemToSelect)
				{
					itemToSelect->setSelected(!wasSelected);
				}
			}
			else
			{
				_treeWidget->clearSelection();
				Global::selection->setSelection(drawablesToSelect);
				if(itemToSelect)
				{
					auto parent = itemToSelect->parent();
					while(parent)
					{
						parent->setExpanded(true);
						parent = parent->parent();
					}
					itemToSelect->setSelected(true);
					_treeWidget->scrollToItem(itemToSelect);
				}
			}
			Global::selection->onHierarchySelectionChanged();

//			emit selectionChanged();
		});
	}

	void Hierarchy::addTree(QTreeWidgetItem* tree)
	{
		if(!_treeWidget)
		{
			_treeWidget = new QTreeWidget();
			_treeWidget->setHeaderHidden(true);
			_treeWidget->setAnimated(false);
			_treeWidget->setIndentation(7);
			_treeWidget->setSelectionBehavior(QTreeView::SelectRows);
			_treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
			_treeWidget->setDragDropMode(QAbstractItemView::DragOnly);
			_treeWidget->setExpandsOnDoubleClick(false);
			_treeWidget->setAlternatingRowColors(true);

			_treeWidget->setUniformRowHeights(true);
			_treeWidget->header()->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
			_treeWidget->header()->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

	#ifdef _WIN32
			_treeWidget->setStyleSheet("QTreeWidget::item:pressed,QTreeWidget::item:selected{background-color : rgb(120,180,255);}‌");
	#endif

			connect(_treeWidget, SIGNAL(itemSelectionChanged()), this, SLOT(_onTreeWidgetSelectionChanged()));
			connect(_treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)), this, SLOT(_onTreeWidgetItemDoubleClicked(QTreeWidgetItem*,int)));
		}
		_treeWidget->addTopLevelItem(tree);
	}

	QList<QTreeWidgetItem*> Hierarchy::getSelection()
	{
		return _treeWidget->selectedItems();
	}

	void Hierarchy::getRenderData(QTreeWidgetItem* item, vector<unsigned int>& drawables, AABB& bound)
	{
		drawables = item->data(0, Qt::UserRole).value<vector<unsigned int>>();
		bound = item->data(0, Qt::UserRole+1).value<app::AABB>();
	}

	void Hierarchy::getRenderData(const QString& name, vector<unsigned int>& drawables, AABB& bound)
	{
		auto item = _nameToTreeItem.value(name);
		if(item == nullptr)
		{
//			auto nname = name;
//			nname.remove(0, 1);
//			auto itr = QTreeWidgetItemIterator(_treeWidget);
//			while(*itr)
//			{
//				if((*itr)->text(0).contains(name, Qt::CaseInsensitive) || (*itr)->text(0).contains(nname, Qt::CaseInsensitive))
//				{
//					io::print(name.toStdString() + " not found, closest match: " + (*itr)->text(0).toStdString());
//				}
//				++itr;
//			}
//			io::print("item does not exist: " + name.toStdString());
		}
		else
		{
			getRenderData(item, drawables, bound);
		}
	}

	QTreeWidgetItem* Hierarchy::getTreeItemFromDrawable(int drawableID)
	{
		return _drawableToTreeItem.value(drawableID);
	}

	void Hierarchy::_onModelLoaded()
	{
		for(int i = 0; i < _treeWidget->topLevelItemCount(); ++i)
		{
			vector<unsigned int> v;
			_collectDrawablesRecursive(_treeWidget->topLevelItem(i), v);
		}


//		vector<QString> prefixes = {"/p-", "/c-", "/v-", "/t-", "/f-", "/mm-", "/b-", "/fl-", "/sc-", "/sealgas-panel", "/demister-heater", "/tq-", "/z-",
//								   "/r-", "/sl-", "/seal_gas_booster", "/sealgas", "/ponte_rolante", "/a-", "/ae-"};
//		vector<QString> found;

//		QTreeWidgetItemIterator itr(_treeWidget);
//		while(*itr)
//		{
//			for(auto p : prefixes)
//			{
//				auto name = (*itr)->text(0);
//				if(name.startsWith(p, Qt::CaseInsensitive))
//				{
//					found.push_back(name);
//				}
//			}
//			++itr;
//		}

//		for(auto n : found)
//		{
//			io::print(n.toStdString());
//		}
	}

	void Hierarchy::_onHierarchyActionTriggered(bool checked)
	{
		if(!_dialog)
		{
			_dialog = new CollapsableDialog("Hierarchy");
			_dialog->layout()->addWidget(_treeWidget);
			Global::glscene->addDialog(_dialog);
		}
		_dialog->setVisible(checked);
	}

	void Hierarchy::_onTreeWidgetSelectionChanged()
	{
		vector<unsigned int> selectedDrawables;

		auto selectedItems = _treeWidget->selectedItems();

		for(auto item : selectedItems)
		{
			auto drawables = item->data(0, Qt::UserRole).value<vector<unsigned int>>();
			selectedDrawables.insert(selectedDrawables.end(), drawables.begin(), drawables.end());
		}

		Global::selection->setSelection(selectedDrawables);

//		emit selectionChanged();
	}

	void Hierarchy::_onTreeWidgetItemDoubleClicked(QTreeWidgetItem* /*item*/, int /*column*/)
	{
		Global::selection->focusOnSelection();
	}

	void Hierarchy::_collectDrawablesRecursive(QTreeWidgetItem* item, vector<unsigned int>& drawables)
	{
		_nameToTreeItem.insert(item->text(0), item);

		auto data = item->data(0, Qt::UserRole);
		if(data.isValid())
		{
			auto itemDrawables = data.value<vector<unsigned int>>();
			if(itemDrawables.size() == 1)
			{
				if(item->childCount() > 0)
				{
					throw std::exception();
				}
				_drawableToTreeItem.insert(itemDrawables.at(0), item);
				drawables.push_back(itemDrawables.at(0));
				return;
			}
			else
			{
				throw std::exception();
			}
		}

		vector<unsigned int> myDrawables;
		for(int i = 0; i < item->childCount(); ++i)
		{
			_collectDrawablesRecursive(item->child(i), myDrawables);
		}

		AABB bound;
		for(auto d : myDrawables)
		{
			bound.expand(Global::renderer->getDrawableBounds(d));
		}

		item->setData(0, Qt::UserRole, QVariant::fromValue(myDrawables));
		item->setData(0, Qt::UserRole+1, QVariant::fromValue(bound));

		drawables.insert(drawables.end(), myDrawables.begin(), myDrawables.end());
	}
}
