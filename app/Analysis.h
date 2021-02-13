#pragma once
#include <QDialog>
#include <QObject>
#include <QAction>
#include <QCheckBox>
#include <QLabel>
#include <QComboBox>
#include <QGraphicsSceneMouseEvent>
#include <QColor>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <app/ColorScale.h>
#include <bl/bl.h>

namespace app
{
	class Task;
	struct SchemeColor {QColor color; unsigned int id;};

	class Analysis : public QObject
	{
		Q_OBJECT
	public:
		void init();
		void saveState();
		void restoreState();

	private slots:
		void _onPlotChanged();
		void _onActionAnalysisTriggered(bool checked);
		void _onColorDoubleClicked(int idx);

	private:
		void _createWidgets();
		void _updateCritLines();
		void _updatePredSucLines();
		void _updateColorScale();
		void _updateOSLines();

		bool _showPredSucLines = false;
		vector<unsigned int> _predLines;
		vector<unsigned int> _sucLines;
		vector<unsigned int> _predDrawables;
		vector<unsigned int> _sucDrawables;

		QDialog* _dlgAnalysis = nullptr;
		QAction* _actAnalysis = nullptr;

		QCheckBox* _ckCriticalPath = nullptr;
		vector<unsigned int> _critLines;
		vector<unsigned int> _critDrawables;

		bool _showOSLines = false;
		vector<unsigned int> _osLines;
		vector<unsigned int> _osDrawables;

		struct DrawableColor { unsigned int drawableID; unsigned int colorID; };
		vector<DrawableColor> _originalDrawableColors;
		QMap<QString, vector<SchemeColor>> _colorSchemes;
		QCheckBox* _ckColorScale = nullptr;
		QComboBox* _cbColorSchemes = nullptr;
		QComboBox* _cbColorIndicator = nullptr;
		QDoubleSpinBox* _dsColorMin = nullptr;
		QDoubleSpinBox* _dsColorMax = nullptr;
		QDateEdit* _deColorMin = nullptr;
		QDateEdit* _deColorMax = nullptr;
		ColorScale _colorScale;
	};
}
