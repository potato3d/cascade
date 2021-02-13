#pragma once
#include <bl/bl.h>
#include <QString>
#include <QDate>
#include <QSet>

namespace app
{
	struct Task;

	struct TaskRef
	{
		TaskRef(Task* t) : task(t), lag(0) {}
		TaskRef(Task* t, int l) : task(t), lag(l) {}
		Task* task = nullptr;
		int lag = 0;
	};

	struct Task
	{
		int id = 0;
		QString taskName;
		QString objectName;
		QDate baselineStart;
		QDate baselineFinish;
		QDate actualStart;
		QDate actualFinish;
		float freeSlack = 0.0f;
		float totalSlack = 0.0f;
		QString os;
		bool isCritical = false;
		vector<TaskRef> predecessors;
		vector<TaskRef> successors;

		int getBaselineDuration()
		{
			return baselineStart.daysTo(baselineFinish) + 1;
		}
		int getActualDuration()
		{
			return actualStart.daysTo(actualFinish) + 1;
		}
		int getDeltaDuration()
		{
			return getActualDuration() - getBaselineDuration();
		}
		int getDeltaStart()
		{
			return baselineStart.daysTo(actualStart);
		}
		int getDeltaFinish()
		{
			return baselineFinish.daysTo(actualFinish);
		}
		float getCustomMetric()
		{
			return (float)((predecessors.size() + successors.size() + 1.0f) * getBaselineDuration()) / (float)(freeSlack * math::sqrt(totalSlack) + 1.0f);
		}

		// no. predecessores(>) vs latencia(<) vs folga total(<)
		// no. sucessores(>) vs folga total(<)
		// variacao inicio / duracao
		// duracao real / duracao estimada
		// probabilidade de atraso de tarefas que ainda nao comecaram baseado no historico dos predecessores
		// porcentagem do custo individual sobre o custo total
		// taxa de custo por tempo: custo individual real / duracao real
		// quanto vou ter gasto ate o fim do mes? pegar taxa de custo por tempo (definir como escolher) e multiplicar pela duracao estimada das tarefas futuras ponderada pelo peso da eac da tarefa futura e criar escala de cores baseada no novo custo estimado total (novo custom acumulado previsto ate cada tarefa)
	};

	class Schedule
	{
	public:
		Schedule(const QString& filename);
		void init();

		QDate getFirstDate();
		QDate getLastDate();
		QList<QString> getUniqueTaskNames();
		QList<Task*> getTasksByTaskName(const QString& name);
		Task* getTaskByObjectName(const QString& name);
		const QHash<QString, Task*>& getTasksByOS();
		const QSet<QString>& getTaskObjectNames();

		QDate getMinBaselineStart();
		QDate getMaxBaselineStart();

		QDate getMinBaselineFinish();
		QDate getMaxBaselineFinish();

		QDate getMinActualStart();
		QDate getMaxActualStart();

		QDate getMinActualFinish();
		QDate getMaxActualFinish();

		int getMinBaselineDuration();
		int getMaxBaselineDuration();

		int getMinActualDuration();
		int getMaxActualDuration();

		int getMinDeltaDuration();
		int getMaxDeltaDuration();

		int getMinDeltaStart();
		int getMaxDeltaStart();

		int getMinDeltaFinish();
		int getMaxDeltaFinish();

		float getMinFreeSlack();
		float getMaxFreeSlack();

		float getMinTotalSlack();
		float getMaxTotalSlack();

		float getMinCustomMetric();
		float getMaxCustomMetric();

	private:
		QString _filename;

		vector<Task*> _tasks;
		vector<Task*> _criticalPathTasks;
		QHash<int, Task*> _byTaskID;
		QHash<QString, Task*> _byTaskName;
		QHash<QString, Task*> _byObjectName;
		QHash<QString, Task*> _byOS;
		QSet<QString> _objectNames;

		QDate _minBaselineStart = QDate(2100, 1, 1);
		QDate _maxBaselineStart = QDate(1900, 1, 1);

		QDate _minBaselineFinish = QDate(2100, 1, 1);
		QDate _maxBaselineFinish = QDate(1900, 1, 1);

		QDate _minActualStart = QDate(2100, 1, 1);
		QDate _maxActualStart = QDate(1900, 1, 1);

		QDate _minActualFinish = QDate(2100, 1, 1);
		QDate _maxActualFinish = QDate(1900, 1, 1);

		int _minBaselineDuration = std::numeric_limits<int>::max();
		int _maxBaselineDuration = -std::numeric_limits<int>::max();

		int _minActualDuration = std::numeric_limits<int>::max();
		int _maxActualDuration = -std::numeric_limits<int>::max();

		int _minDeltaDuration = std::numeric_limits<int>::max();
		int _maxDeltaDuration = -std::numeric_limits<int>::max();

		int _minDeltaStart = std::numeric_limits<int>::max();
		int _maxDeltaStart = -std::numeric_limits<int>::max();

		int _minDeltaFinish = std::numeric_limits<int>::max();
		int _maxDeltaFinish = -std::numeric_limits<int>::max();

		float _minFreeSlack = std::numeric_limits<float>::max();
		float _maxFreeSlack = -std::numeric_limits<float>::max();

		float _minTotalSlack = std::numeric_limits<float>::max();
		float _maxTotalSlack = -std::numeric_limits<float>::max();

		float _minCustomMetric = std::numeric_limits<float>::max();
		float _maxCustomMetric = -std::numeric_limits<float>::max();
	};
}
