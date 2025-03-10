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
using namespace boost::program_options;

int main(int argc, char* argv[])
{
	MCTS::PARAMS searchParams;
	EXPERIMENT::PARAMS expParams;
	SIMULATOR::KNOWLEDGE knowledge;
	knowledge.RolloutLevel = SIMULATOR::KNOWLEDGE::LEGAL; // LEGAL to use no extra knowledge, SMART to use preferred actions
	string problem, policy, horizonString;
	string outputfile;

    string banditArmCapacity, banditBetaPriorString, banditConvergenceEpsilonString;
	int size, number, treeknowledge = 1, rolloutknowledge = 1, smarttreecount = 10;
	problem = argv[1];
    horizonString = argv[2];
	banditBetaPriorString = argv[3];

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
		real = new NETWORK(size, number);
		simulator = new NETWORK(size, number);
	}
	else if(problem == "rocksample")
	{
        string problemSize = argv[4];
		size = 7;
		number = 8;
		string arg;
		arg = problemSize;
		if(arg == "11")
		{
			size = 11;
			number = 11;
		}
		else if(arg == "15")
		{
			size = 15;
			number = 15;
		}
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
    
	// set up parameters for search
	searchParams = MCTS::PARAMS();
	expParams = EXPERIMENT::PARAMS();
	searchParams.MaxDepth = stoi(horizonString); // maximum search depth
    simulator->SetKnowledge(knowledge);

	// configure output file
    outputfile = "output/" + problem;
	if(problem == "rocksample") 
	{
        string problemSize = argv[4];
		outputfile += problemSize;
	}
	outputfile += "_POSTS_legal_prior-"+banditBetaPriorString+"_horizon-"+horizonString+".txt";
    
	// run experiment with given problem and parameters
	EXPERIMENT experiment(*real,*simulator, outputfile, expParams, searchParams);
	experiment.DiscountedReturn();

	delete real;
	delete simulator;
	return 0;
}
