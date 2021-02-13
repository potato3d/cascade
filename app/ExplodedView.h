#pragma once
#include <bl/bl.h>
#include <QObject>
#include <QAction>
#include <QDialog>
#include <QLabel>
#include <QCheckBox>
#include <QSlider>
#include <QDateEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QTimer>
#include <app/AABB.h>

class QGraphicsSceneMouseEvent;

namespace app
{
	class ExplodedView : public QObject
	{
		Q_OBJECT
	public:
		void init();
		void saveState();
		void restoreState();
		bool isActive();
		QString getActiveTaskName();

	signals:
		void plotChanged();

	private slots:
		void _onScheduleActionTriggered(bool checked);
		void _onAnimationTimeChanged();
		void _onSelectionChanged();

	private:
		void _createWidgets();
		void _rebuildPlot();

		// ------------------------------------------------------------------

		QAction* _actSchedule = nullptr;
		QDialog* _dlgSchedule = nullptr;

		QCheckBox* _ck3DPlot;
		QCheckBox* _ckHorizontalPlane;
		QCheckBox* _ckHighlightOutOfRange;
		QPushButton* _pbHideOutOfRange;
		QPushButton* _pbShowOutOfRange;
		QCheckBox* _ckCompareSchedules;
		QDoubleSpinBox* _dsbVerticalScale;
		QDoubleSpinBox* _dsbXOffset;
		QDoubleSpinBox* _dsbYOffset;
		QDoubleSpinBox* _dsbZOffset;
		QComboBox* _cbVerticalLabels;
		QSpinBox* _sbVerticalCount;
		QComboBox* _cbBoundFace;
		QComboBox* _cbTaskName;
		QComboBox* _cbStartFinish;

		// ------------------------------------------------------------------

		QSlider* _slCurrDate;
		QDateEdit* _deCurrDate;
		QDoubleSpinBox* _dsbSpeed;
		QPushButton* _pbFullBack;
		QPushButton* _pbBack;
		QPushButton* _pbPlayPause;
		QPushButton* _pbStop;
		QPushButton* _pbFwd;
		QPushButton* _pbFullFwd;

		// ------------------------------------------------------------------

		QTimer _animationTimer;

		// ------------------------------------------------------------------

		unsigned int _plotBackground = 0; // vertical quad
		unsigned int _plotFloor = 0; // horizontal quad
		vector<unsigned int> _plotHorizontalBars; // thin cylinders
		vector<QLabel*> _plotHorizontalLabels; // axis labels
		vector<unsigned int> _plotVerticalLines; // lines beneath scene drawables
		vector<unsigned int> _plotVerticalSpheres; // spheres beneath scene drawables
		unsigned int _lineColorID;

		bool _showSelectionDates = false;
		vector<QLabel*> _plotSelectionDateLabels;
		vector<unsigned int> _plotSelectionDateLines;
		vector<unsigned int> _plotSelectionDateSpheres;

		bool _hideOutOfRange = false;
		bool _showOutOfRange = false;

		bool _showVerticalLines = false;

		bool _lastHidOrShown = false;
		bool _lastHidOutOfRange = false;

		struct TaskBoxInfo {unsigned int drawable; AABB bound;};
		bool _showTaskBoxes = false;
		vector<unsigned int> _taskBoxDrawables;
		vector<TaskBoxInfo> _visibleTaskBoxes;
		unsigned int _freeTaskBoxColorID = 0;
		unsigned int _collidedTaskBoxColorID = 0;
	};
}
