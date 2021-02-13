#include <app/ModelLoader.h>
#include <app/Global.h>
#include <rvm/FileReader.h>
#include <rvm/StatsCollector.h>
#include <app/rvm_observer.h>

namespace app
{
	void ModelLoader::init()
	{
#ifdef _WIN32
//		string baseDir = "d:/models/abast/COMPERJ/PDMS/"; // using this to test
//		string baseDir = "d:/models/DRGN/REFAP/PDMS/";
		string baseDir = "c:/models/REFAP/";
#else
//		string baseDir = "/home/potato/Models/abast/COMPERJ/PDMS/"; // using this to test
		string baseDir = "/home/potato/Models/abast/REFAP/PDMS/";
#endif

		vector<string> dirs;

		// models with correct bounds
//		dirs.push_back(baseDir + "SE-2400");
//		dirs.push_back(baseDir + "SE-4100");
//		dirs.push_back(baseDir + "SE-4500");
//		dirs.push_back(baseDir + "SE-5122");
//		dirs.push_back(baseDir + "SE-5142");
//		dirs.push_back(baseDir + "SE-5601");
//		dirs.push_back(baseDir + "SE-6300");
//		dirs.push_back(baseDir + "SE-6310");
//		dirs.push_back(baseDir + "U-2300"); // using this to test
//		dirs.push_back(baseDir + "U-2400");
//		dirs.push_back(baseDir + "U-2500");
//		dirs.push_back(baseDir + "U-2600");
//		dirs.push_back(baseDir + "U-4410");
//		dirs.push_back(baseDir + "U-4450");
//		dirs.push_back(baseDir + "U-5340");
//		dirs.push_back(baseDir + "U-5950");
//		dirs.push_back(baseDir + "U-6300");
//		dirs.push_back(baseDir + "U-54231");

		// small
//		dirs.push_back(baseDir + "U-4470");

		// medium
//		dirs.push_back(baseDir + "U-4710");

		// large
//		dirs.push_back(baseDir + "U-2400");

		// massive
//		dirs.push_back(baseDir + "U-2200");
//		dirs.push_back(baseDir + "U-2300");
//		dirs.push_back(baseDir + "U-2400");
//		dirs.push_back(baseDir + "U-2500");
//		dirs.push_back(baseDir + "U-2600");

		// refap
		dirs.push_back(baseDir + "UHDS");

		vector<string> files;

		while(!dirs.empty())
		{
			auto d = dirs.back();
			dirs.pop_back();

			auto contents = path::list_dir(d);
			for(const auto& c : contents)
			{
				if(path::is_dir(c))
				{
					dirs.push_back(c);
				}
				else if(str::ends_with(c, ".rvm") || str::ends_with(c, ".RVM"))
				{
					files.push_back(c);
				}
			}
		}

		if(files.empty())
		{
			throw std::exception();
		}

		rvm::FileReader reader;
		rvm_observer obs(Global::renderer);
		obs.addNodeToReject("SCTN 1 of FRMWORK 1 of STRUCTURE 1 of ZONE /U-0311_CAIXAS_JUNÇÃO_CJP");

		for(const auto& f : files)
		{
			rvm::StatsCollector stats;
			reader.readFile(f.data(), &stats);
			stats.getStats().print();

			reader.readFile(f.data(), &obs);

			Global::hierarchy->addTree(obs.getTree());
		}

		Global::renderer->endLoad();

		emit modelLoaded();
	}
}
