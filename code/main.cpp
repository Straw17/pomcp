#include "battleship.h"
#include "mcts.h"
#include "network.h"
#include "pocman.h"
#include "rocksample.h"
#include "tag.h"
#include "experiment.h"
#include <string>
#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

int main(int argc, char* argv[])
{
	MCTS::PARAMS searchParams;
	EXPERIMENT::PARAMS expParams;
	SIMULATOR::KNOWLEDGE knowledge;
	string problem, policy, horizonString;
	string outputfile;

	po::options_description desc("Allowed options");
	desc.add_options()
		("problem,p", po::value<string>()->default_value("battleship"), "set problem type")
		("name,n", po::value<string>()->default_value("default"), "set test name")
		("size,s", po::value<int>()->default_value(10), "set size of problem")
		("number,num", po::value<int>()->default_value(10), "set number of objects in problem")
		("experiment_config", po::value<string>()->default_value("experiment_config"), "set experiment config file path")
		("knowledge_config", po::value<string>()->default_value("knowledge_config"), "set knowledge config file path")
		("mcts_config", po::value<string>()->default_value("mcts_config"), "set MCTS config file path");

	po::variables_map vm;
	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

    string testName;
	int size, number;
	problem = vm["problem"].as<string>();
	testName = vm["name"].as<string>();

	// set up parameters for search, initializing based on config files
	searchParams = MCTS::PARAMS("config/" + vm["mcts_config"].as<string>() + ".json");
	expParams = EXPERIMENT::PARAMS("config/" + vm["experiment_config"].as<string>() + ".json");
	knowledge = SIMULATOR::KNOWLEDGE("config/" + vm["knowledge_config"].as<string>() + ".json");

	// set up real and simulated version of problem
	SIMULATOR* real = 0;
	SIMULATOR* simulator = 0;
	if(problem == "battleship")
	{
		real = new BATTLESHIP(10, 10, 5);
		simulator = new BATTLESHIP(10, 10, 5);
	}
	else if(problem == "pocman")
	{
		real = new FULL_POCMAN();
		simulator = new FULL_POCMAN();
	}
	else if(problem == "network")
	{
		size = vm["size"].as<int>();
		number = vm["number"].as<int>();
		real = new NETWORK(size, number);
		simulator = new NETWORK(size, number);
	}
	else if(problem == "rocksample")
	{
        size = vm["size"].as<int>();
		number = vm["number"].as<int>();
		real = new ROCKSAMPLE(size, number);
		simulator = new ROCKSAMPLE(size, number);
	}
	else if(problem == "tag")
	{
		real = new TAG(1);
		simulator = new TAG(1);
	}
	else
	{
		cout << "Unknown problem" << endl;
		exit(1);
	}

	// add knowledge level
    simulator->SetKnowledge(knowledge);

	// configure output file
	if(testName == "default") {
		outputfile = "/dev/null";
	} else {
    	outputfile = "output/" + problem + "_" + testName + ".csv";
	}
    
	// run experiment with given problem and parameters
	EXPERIMENT experiment(*real,*simulator, outputfile, expParams, searchParams);
	experiment.DiscountedReturn();

	delete real;
	delete simulator;
	return 0;
}
