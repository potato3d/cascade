#include <app/Schedule.h>
#include <QFile>
#include <QStringList>

namespace app
{
	struct TaskRefID
	{
		int id = -1;
		int lag = 0;
	};

	void linkTasks(Task* t1, Task* t2)
	{
		t1->successors.push_back(t2);
		t2->predecessors.push_back(t1);
	}

	QDate columnToDate(QString& col)
	{
		return QDate::fromString(col.remove(0, 4), "dd/MM/yy").addYears(100); // skip day of the week, add 100 years to get to 2000's
	}

	Schedule::Schedule(const QString& filename)
	{
		_filename = filename;
	}

	void Schedule::init()
	{
		QFile file(_filename);

		if(!file.exists())
		{
			throw std::exception();
		}

		if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			throw std::exception();
		}

#define columnToInt(t, c) bool ok = false; (t) = (c).toInt(&ok); if(!ok) throw std::exception();
#define columnToFloat(t, c) bool ok = false; (t) = (c.replace(",", ".")).toDouble(&ok); if(!ok) throw std::exception();

		QHash<Task*, TaskRefID> predecessors;
		QHash<Task*, TaskRefID> successors;

		while(!file.atEnd())
		{
			QString line = file.readLine();
			line = line.trimmed();
			if(line.isEmpty() || line.startsWith(';'))
			{
				continue;
			}

			// TODO: critical path column

			// id # sop # critical # object name # baseline start # baseline finish # actual start # actual finish # free slack # total slack # predecessors # successors # ... (ignored)

			auto task = new Task();
			_tasks.push_back(task);

			task->taskName = "Assembly in Field"; // example based only in this task

			auto columns = line.split("#");
			int idx = 0;
			for(const auto& c : columns)
			{
				QString col = c.trimmed();
				switch(idx++) // dont forget to increment idx
				{
				case 0: // id
				{
					columnToInt(task->id, col);
					break;
				}
				case 1: // sop
					task->os = col;
					break;
				case 2: // is critical?
					task->isCritical = col.compare("yes", Qt::CaseInsensitive) == 0;
					break;
				case 3: // object name
					task->objectName = "/" + col;
					break;
				case 4: // baseline start
					task->baselineStart = columnToDate(col);
					break;
				case 5: // baseline finish
					task->baselineFinish = columnToDate(col);
					break;
				case 6: // actual start
					if(col.toLower() == "nd")
					{
						task->actualStart = task->baselineStart;
					}
					else
					{
						task->actualStart = columnToDate(col);
					}
					break;
				case 7: // actual finish
					if(col.compare("nd", Qt::CaseInsensitive) == 0)
					{
						task->actualFinish = task->baselineFinish;
					}
					else
					{
						task->actualFinish = columnToDate(col);
					}
					break;
				case 8: // free slack
				{
					columnToFloat(task->freeSlack, col);
					break;
				}
				case 9: // total slack
				{
					columnToFloat(task->totalSlack, col);
					break;
				}
				case 10: // predecessors
				{
					auto preds = col.split(";");
					for(const auto& p : preds)
					{
						TaskRefID predRef;

						auto idPlusLag = p.split("+");

						if(idPlusLag.isEmpty() || idPlusLag.size() > 2) throw std::exception();

						bool ok = false;
						predRef.id = idPlusLag.takeFirst().toInt(&ok);
						if(!ok) throw std::exception();

						if(!idPlusLag.isEmpty())
						{
							auto list = idPlusLag.takeFirst().split(" ");
							if(list.empty()) throw std::exception();
							ok = false;
							predRef.lag = list.takeFirst().toInt(&ok);
							if(!ok) throw std::exception();
						}

						predecessors.insertMulti(task, predRef);
					}
					break;
				}
				case 11: // successors
				{
					auto sucs = col.split(";");
					for(const auto& s : sucs)
					{
						TaskRefID sucRef;

						auto idPlusLag = s.split("+");

						if(idPlusLag.isEmpty() || idPlusLag.size() > 2) throw std::exception();

						bool ok = false;
						sucRef.id = idPlusLag.takeFirst().toInt(&ok);
						if(!ok) throw std::exception();

						if(!idPlusLag.isEmpty())
						{
							auto list = idPlusLag.takeFirst().split(" ");
							if(list.empty()) throw std::exception();
							ok = false;
							sucRef.lag = list.takeFirst().toInt(&ok);
							if(!ok) throw std::exception();
						}

						successors.insertMulti(task, sucRef);
					}
					break;
				}
				default: // (ignored)
					break;
				}
			}
		}

//		auto task1 = new Task();
//		task1->taskName = "task1";
//		task1->objectName = "/P-2300011";
//		task1->baselineStart = QDate(2013, 1, 1);
//		task1->baselineFinish = QDate(2013, 1, 15);
//		task1->actualStart = QDate(2013, 1, 1);
//		task1->actualFinish = QDate(2013, 1, 15);
//		task1->freeSlack = 0;
//		task1->totalSlack = 0;
//		task1->os = "SOP 1";
//		task1->isCritical = false;

//		auto task2 = new Task();
//		task2->taskName = "task1";
//		task2->objectName = "/P-2300010";
//		task2->baselineStart = QDate(2013, 2, 1);
//		task2->baselineFinish = QDate(2013, 2, 15);
//		task2->actualStart = QDate(2013, 2, 1);
//		task2->actualFinish = QDate(2013, 2, 15);
//		task2->freeSlack = 0;
//		task2->totalSlack = 0;
//		task2->os = "SOP 1";
//		task2->isCritical = false;

//		auto task3 = new Task();
//		task3->taskName = "task1";
//		task3->objectName = "/P-2300009#C";
//		task3->baselineStart = QDate(2013, 3, 1);
//		task3->baselineFinish = QDate(2013, 3, 15);
//		task3->actualStart = QDate(2013, 3, 1);
//		task3->actualFinish = QDate(2013, 3, 15);
//		task3->freeSlack = 0;
//		task3->totalSlack = 0;
//		task3->os = "SOP 1";
//		task3->isCritical = false;

//		auto task4 = new Task();
//		task4->taskName = "task1";
//		task4->objectName = "/P-2300007B#C";
//		task4->baselineStart = QDate(2013, 4, 1);
//		task4->baselineFinish = QDate(2013, 4, 15);
//		task4->actualStart = QDate(2013, 4, 1);
//		task4->actualFinish = QDate(2013, 4, 15);
//		task4->freeSlack = 0;
//		task4->totalSlack = 0;
//		task4->os = "SOP 1";
//		task4->isCritical = false;

//		auto task5 = new Task();
//		task5->taskName = "task1";
//		task5->objectName = "/P-2300007A#C";
//		task5->baselineStart = QDate(2013, 5, 1);
//		task5->baselineFinish = QDate(2013, 5, 15);
//		task5->actualStart = QDate(2013, 5, 1);
//		task5->actualFinish = QDate(2013, 5, 15);
//		task5->freeSlack = 0;
//		task5->totalSlack = 0;
//		task5->os = "SOP 1";
//		task5->isCritical = false;

//		auto task6 = new Task();
//		task6->taskName = "task1";
//		task6->objectName = "/P-2300014";
//		task6->baselineStart = QDate(2013, 6, 1);
//		task6->baselineFinish = QDate(2013, 6, 15);
//		task6->actualStart = QDate(2013, 6, 1);
//		task6->actualFinish = QDate(2013, 6, 15);
//		task6->freeSlack = 0;
//		task6->totalSlack = 0;
//		task6->os = "SOP 1";
//		task6->isCritical = false;

//		auto task1a = new Task();
//		task1a->taskName = "task1";
//		task1a->objectName = "/B-2300001B";
//		task1a->baselineStart = QDate(2013, 3, 1);
//		task1a->baselineFinish = QDate(2013, 3, 7);
//		task1a->actualStart = QDate(2013, 3, 1);
//		task1a->actualFinish = QDate(2013, 3, 7);
//		task1a->freeSlack = 0;
//		task1a->totalSlack = 0;
//		task1a->os = "SOP 2";
//		task1a->isCritical = true;

//		auto task2a = new Task();
//		task2a->taskName = "task1";
//		task2a->objectName = "/B-2300001A";
//		task2a->baselineStart = QDate(2013, 4, 1);
//		task2a->baselineFinish = QDate(2013, 4, 20);
//		task2a->actualStart = QDate(2013, 4, 1);
//		task2a->actualFinish = QDate(2013, 4, 20);
//		task2a->freeSlack = 0;
//		task2a->totalSlack = 0;
//		task2a->os = "SOP 2";
//		task2a->isCritical = true;

//		auto task3a = new Task();
//		task3a->taskName = "task1";
//		task3a->objectName = "/B-2300005A#C";
//		task3a->baselineStart = QDate(2013, 5, 1);
//		task3a->baselineFinish = QDate(2013, 5, 7);
//		task3a->actualStart = QDate(2013, 5, 1);
//		task3a->actualFinish = QDate(2013, 5, 7);
//		task3a->freeSlack = 0;
//		task3a->totalSlack = 0;
//		task3a->os = "SOP 2";
//		task3a->isCritical = true;

//		auto task4a = new Task();
//		task4a->taskName = "task1";
//		task4a->objectName = "/B-2300005B#C";
//		task4a->baselineStart = QDate(2013, 5, 1);
//		task4a->baselineFinish = QDate(2013, 5, 20);
//		task4a->actualStart = QDate(2013, 5, 1);
//		task4a->actualFinish = QDate(2013, 5, 20);
//		task4a->freeSlack = 0;
//		task4a->totalSlack = 0;
//		task4a->os = "SOP 2";
//		task4a->isCritical = true;

//		_tasks.push_back(task1);
//		_tasks.push_back(task2);
//		_tasks.push_back(task3);
//		_tasks.push_back(task4);
//		_tasks.push_back(task5);
//		_tasks.push_back(task6);
//		_tasks.push_back(task1a);
//		_tasks.push_back(task2a);
//		_tasks.push_back(task3a);
//		_tasks.push_back(task4a);

//		linkTasks(task1, task2);
//		linkTasks(task1, task3);
//		linkTasks(task2, task4);
//		linkTasks(task3, task4);
//		linkTasks(task2, task5);
//		linkTasks(task4, task5);
//		linkTasks(task5, task6);

//		linkTasks(task1a, task2a);
//		linkTasks(task2a, task3a);
//		linkTasks(task2a, task4a);

		for(auto t : _tasks)
		{
			_byTaskID.insert(t->id, t);
			_byTaskName.insertMulti(t->taskName, t);
			_byObjectName.insert(t->objectName, t);
			_byOS.insertMulti(t->os, t);

			if(_objectNames.contains(t->objectName))
			{
				throw std::exception();
			}
			_objectNames.insert(t->objectName);

			_minBaselineStart = math::min(_minBaselineStart, t->baselineStart);
			_maxBaselineStart = math::max(_maxBaselineStart, t->baselineStart);

			_minBaselineFinish = math::min(_minBaselineFinish, t->baselineFinish);
			_maxBaselineFinish = math::max(_maxBaselineFinish, t->baselineFinish);

			_minActualStart = math::min(_minActualStart, t->actualStart);
			_maxActualStart = math::max(_maxActualStart, t->actualStart);

			_minActualFinish = math::min(_minActualFinish, t->actualFinish);
			_maxActualFinish = math::max(_maxActualFinish, t->actualFinish);

			_minBaselineDuration = math::min(_minBaselineDuration, t->getBaselineDuration());
			_maxBaselineDuration = math::max(_maxBaselineDuration, t->getBaselineDuration());

			_minActualDuration = math::min(_minActualDuration, t->getActualDuration());
			_maxActualDuration = math::max(_maxActualDuration, t->getActualDuration());

			_minDeltaDuration = math::min(_minDeltaDuration, t->getDeltaDuration());
			_maxDeltaDuration = math::max(_maxDeltaDuration, t->getDeltaDuration());

			_minDeltaStart = math::min(_minDeltaStart, t->getDeltaStart());
			_maxDeltaStart = math::max(_maxDeltaStart, t->getDeltaStart());

			_minDeltaFinish = math::min(_minDeltaFinish, t->getDeltaFinish());
			_maxDeltaFinish = math::max(_maxDeltaFinish, t->getDeltaFinish());

			_minFreeSlack = math::min(_minFreeSlack, t->freeSlack);
			_maxFreeSlack = math::max(_maxFreeSlack, t->freeSlack);

			_minTotalSlack = math::min(_minTotalSlack, t->totalSlack);
			_maxTotalSlack = math::max(_maxTotalSlack, t->totalSlack);
		}

		auto predKeys = predecessors.uniqueKeys();
		for(auto pk : predKeys)
		{
			auto preds = predecessors.values(pk);
			for(const auto& p : preds)
			{
				if(_byTaskID.contains(p.id))
				{
					pk->predecessors.push_back({_byTaskID.value(p.id), p.lag});
				}
			}
		}

		auto sucKeys = successors.uniqueKeys();
		for(auto sk : sucKeys)
		{
			auto sucs = successors.values(sk);
			for(const auto& s : sucs)
			{
				if(_byTaskID.contains(s.id))
				{
					sk->successors.push_back({_byTaskID.value(s.id), s.lag});
				}
			}
		}

		for(auto t : _tasks)
		{
			_minCustomMetric = math::min(_minCustomMetric, t->getCustomMetric());
			_maxCustomMetric = math::max(_maxCustomMetric, t->getCustomMetric());
		}
	}

	QDate Schedule::getFirstDate()
	{
		return math::min(math::min(_minBaselineStart, _minBaselineFinish), math::min(_minActualStart, _minActualFinish));
	}

	QDate Schedule::getLastDate()
	{
		return math::max(math::max(_maxBaselineStart, _maxBaselineFinish), math::max(_maxActualStart, _maxActualFinish));
	}

	QList<QString> Schedule::getUniqueTaskNames()
	{
		return _byTaskName.uniqueKeys();
	}

	QList<Task*> Schedule::getTasksByTaskName(const QString& name)
	{
		return _byTaskName.values(name);
	}

	Task* Schedule::getTaskByObjectName(const QString& name)
	{
		return _byObjectName.value(name);
	}

	const QHash<QString, Task*>& Schedule::getTasksByOS()
	{
		return _byOS;
	}

	const QSet<QString>& Schedule::getTaskObjectNames()
	{
		return _objectNames;
	}

	QDate Schedule::getMinBaselineStart()
	{
		return _minBaselineStart;
	}

	QDate Schedule::getMaxBaselineStart()
	{
		return _maxBaselineStart;
	}

	QDate Schedule::getMinBaselineFinish()
	{
		return _minBaselineFinish;
	}

	QDate Schedule::getMaxBaselineFinish()
	{
		return _maxBaselineFinish;
	}

	QDate Schedule::getMinActualStart()
	{
		return _minActualStart;
	}

	QDate Schedule::getMaxActualStart()
	{
		return _maxActualStart;
	}

	QDate Schedule::getMinActualFinish()
	{
		return _minActualFinish;
	}

	QDate Schedule::getMaxActualFinish()
	{
		return _maxActualFinish;
	}

	int Schedule::getMinBaselineDuration()
	{
		return _minBaselineDuration;
	}

	int Schedule::getMaxBaselineDuration()
	{
		return _maxBaselineDuration;
	}

	int Schedule::getMinActualDuration()
	{
		return _minActualDuration;
	}

	int Schedule::getMaxActualDuration()
	{
		return _maxActualDuration;
	}

	int Schedule::getMinDeltaDuration()
	{
		return _minDeltaDuration;
	}

	int Schedule::getMaxDeltaDuration()
	{
		return _maxDeltaDuration;
	}

	int Schedule::getMinDeltaStart()
	{
		return _minDeltaStart;
	}

	int Schedule::getMaxDeltaStart()
	{
		return _maxDeltaStart;
	}

	int Schedule::getMinDeltaFinish()
	{
		return _minDeltaFinish;
	}

	int Schedule::getMaxDeltaFinish()
	{
		return _maxDeltaFinish;
	}

	float Schedule::getMinFreeSlack()
	{
		return _minFreeSlack;
	}

	float Schedule::getMaxFreeSlack()
	{
		return _maxFreeSlack;
	}

	float Schedule::getMinTotalSlack()
	{
		return _minTotalSlack;
	}

	float Schedule::getMaxTotalSlack()
	{
		return _maxTotalSlack;
	}

	float Schedule::getMinCustomMetric()
	{
		return _minCustomMetric;
	}

	float Schedule::getMaxCustomMetric()
	{
		return _maxCustomMetric;
	}

}
