#pragma once
#include <app/AABB.h>
#include <bl/bl.h>
#include <QObject>
#include <QLabel>
#include <QPoint>
#include <QGraphicsSceneMouseEvent>
#include <QAction>
#include <QComboBox>
#include <QDialog>
#include <QPushButton>
#include <QCheckBox>
#include <functional>
#include <QSlider>
#include <QTableWidget>
#include <functional>

namespace app
{
	class Selection : public QObject
	{
		Q_OBJECT
	public:
		void init();

		void saveState();
		void restoreState();

		void setCustomSelection(std::function<void(QGraphicsSceneMouseEvent*)> customFunc);

		void pivotOnSelection();
		void focusOnSelection();
		void setSelection(const vector<unsigned int>& drawables);
		bool toggleSelection(const vector<unsigned int>& drawables);
		const QSet<unsigned int>& getSelection();

		void addSelectionCallback(const string& name, std::function<void(bool)> callback, bool enabled);

		void onHierarchySelectionChanged();

	signals:
		void selectionChanged();

	private slots:
		void _onPlotChanged();

		void _onMousePressed(QGraphicsSceneMouseEvent* e);
		void _onMouseReleased(QGraphicsSceneMouseEvent* e);
		void _onMouseDoubleClicked(QGraphicsSceneMouseEvent* e);

		void _onActionSelectionTriggered(bool checked);
		void _onActionMetadataTriggered(bool checked);

		void _onHierarchySelectionChanged();

	private:
		void _clearMetadata();
		AABB _getSelectionBounds();
		void _setOthersInvisible();
		void _setOthersTransparent();
		void _setAllVisible();
		void _setAllOpaque();
		void _updateDlgSelection();
		void _updateDrawableIndicators();

		std::function<void(QGraphicsSceneMouseEvent*)> _customSelection;

		QSet<unsigned int> _selection;
		bool _justFocused = false;
		QPointF _mousePressPos;

		QCheckBox* _ckVisible = nullptr;
		QCheckBox* _ckOpaque = nullptr;
		QPushButton* _pbOthersInvisible = nullptr;
		QPushButton* _pbOthersTransparent = nullptr;
		QPushButton* _pbAllVisible = nullptr;
		QPushButton* _pbAllOpaque = nullptr;
		QSlider* _slTransparency = nullptr;

		QAction* _actSelection = nullptr;
		QDialog* _dlgSelection = nullptr;
		struct CallbackWidget {QCheckBox* widget; std::function<void(bool)> callback;};
		vector<CallbackWidget> _callbackWidgets;

		bool _showSelectionHighlight = true;

		QAction* _actMetadata = nullptr;
		QDialog* _dlgMetadata = nullptr;
		QTableWidget* _twMetadata = nullptr;

		QComboBox* _cbIndicator = nullptr;
		bool _showDrawableIndicators = false;
		vector<QLabel*> _drawableIndicators;
	};
}
