#pragma once
#include <QObject>
#include <QAction>
#include <QTreeWidget>
#include <app/CollapsableDialog.h>
#include <QTreeWidgetItem>
#include <bl/bl.h>
#include <QSpinBox>
#include <app/AABB.h>
#include <functional>
using std::function;

class QGraphicsSceneMouseEvent;

namespace app
{
	class Hierarchy : public QObject
	{
		Q_OBJECT
	public:
		void init();
		void addTree(QTreeWidgetItem* tree);
		QList<QTreeWidgetItem*> getSelection();
		void getRenderData(QTreeWidgetItem* item, vector<unsigned int>& drawables, AABB& bound);
		void getRenderData(const QString& name, vector<unsigned int>& drawables, AABB& bound);
		QTreeWidgetItem* getTreeItemFromDrawable(int drawableID);

	signals:
		void selectionChanged();


	private slots:
		void _onModelLoaded();
		void _onHierarchyActionTriggered(bool checked);
		void _onTreeWidgetSelectionChanged();
		void _onTreeWidgetItemDoubleClicked(QTreeWidgetItem* item, int column);

	private:
		void _collectDrawablesRecursive(QTreeWidgetItem* item, vector<unsigned int>& drawables);

		QAction* _action = nullptr;
		CollapsableDialog* _dialog = nullptr;
		QTreeWidget* _treeWidget = nullptr;
		QHash<QString, QTreeWidgetItem*> _nameToTreeItem;
		QHash<unsigned int, QTreeWidgetItem*> _drawableToTreeItem;
	};
}
