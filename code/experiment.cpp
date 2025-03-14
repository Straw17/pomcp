#include "experiment.h"
#include "boost/timer.hpp"
#include <boost/property_tree/json_parser.hpp>

using namespace std;

EXPERIMENT::PARAMS::PARAMS()
	: NumRuns(100),
	NumSteps(100000),
	SimSteps(1000),
	TimeOut(12*3600),
	MinDoubles(1),
	MaxDoubles(14),
	TransformDoubles(-4),
	TransformAttempts(1000),
	Accuracy(0.01),
	UndiscountedHorizon(1000),
	AutoExploration(true)
{
}

EXPERIMENT::PARAMS::PARAMS(const std::string& filename) {
	boost::property_tree::ptree pt;
        
	// Read the JSON file
	boost::property_tree::read_json(filename, pt);
	
	// Load parameters from the property tree
	NumRuns = pt.get<int>("NumRuns");
	NumSteps = pt.get<int>("NumSteps");
	SimSteps = pt.get<int>("SimSteps");
	TimeOut = pt.get<int>("TimeOut");
	MinDoubles = pt.get<int>("MinDoubles");
	MaxDoubles = pt.get<int>("MaxDoubles");
	TransformDoubles = pt.get<int>("TransformDoubles");
	TransformAttempts = pt.get<int>("TransformAttempts");
	Accuracy = pt.get<double>("Accuracy");
	UndiscountedHorizon = pt.get<int>("UndiscountedHorizon");
	AutoExploration = pt.get<bool>("AutoExploration");
}

EXPERIMENT::EXPERIMENT(const SIMULATOR& real,
	const SIMULATOR& simulator, const string& outputFile,
	EXPERIMENT::PARAMS& expParams, MCTS::PARAMS& searchParams)
	: Real(real),
	Simulator(simulator),
	OutputFile(outputFile.c_str()),
	ExpParams(expParams),
	SearchParams(searchParams)
{
	if (ExpParams.AutoExploration)
	{
		if (SearchParams.UseRave)
			SearchParams.ExplorationConstant = 0;
		else
			SearchParams.ExplorationConstant = simulator.GetRewardRange();
	}
	MCTS::InitFastUCB(SearchParams.ExplorationConstant);
}

void EXPERIMENT::Run()
{
	boost::timer timer;

	MCTS* mcts = NULL;
	mcts = new MCTS(Simulator, SearchParams);
	double undiscountedReturn = 0.0;
	double discountedReturn = 0.0;
	double discount = 1.0;
	bool terminal = false;
	bool outOfParticles = false;
	int t;

	STATE* state = Real.CreateStartState();
	if (SearchParams.Verbose >= SearchParams.TREE)
		Real.DisplayState(*state, cout);

	for (t = 0; t < ExpParams.NumSteps; t++)
	{
		int observation;
		double reward;
        int action = mcts->SelectAction();
		terminal = Real.Step(*state, action, observation, reward);

		Results.Reward.Add(reward);
		undiscountedReturn += reward;
		discountedReturn += reward * discount;
		discount *= Real.GetDiscount();
		if (SearchParams.Verbose >= SearchParams.TREE)
		{
			Real.DisplayAction(action, cout);
			Real.DisplayState(*state, cout);
			Real.DisplayObservation(*state, observation, cout);
			Real.DisplayReward(reward, cout);
		}

		if (terminal)
		{
			cout << "Terminated" << endl;
			break;
		}
		outOfParticles = !mcts->Update(action, observation, reward);
		if (outOfParticles)
			break;

		if (timer.elapsed() > ExpParams.TimeOut)
		{
			cout << "Timed out after " << t << " steps in "
				<< Results.Time.GetTotal() << "seconds" << endl;
			break;
		}
	}

	if (outOfParticles)
	{
		cout << "Out of particles, finishing episode with SelectRandom" << endl;
		HISTORY history = mcts->GetHistory();
		while (++t < ExpParams.NumSteps)
		{
			int observation;
			double reward;

			// This passes real state into simulator!
			// SelectRandom must only use fully observable state
			// to avoid "cheating"
			int action = Simulator.SelectRandom(*state, history, mcts->GetStatus());
			terminal = Real.Step(*state, action, observation, reward);

			Results.Reward.Add(reward);
			undiscountedReturn += reward;
			discountedReturn += reward * discount;
			discount *= Real.GetDiscount();
			if (SearchParams.Verbose >= SearchParams.TREE)
			{
				Real.DisplayAction(action, cout);
				Real.DisplayState(*state, cout);
				Real.DisplayObservation(*state, observation, cout);
				Real.DisplayReward(reward, cout);
			}

			if (terminal)
			{
				cout << "Terminated" << endl;
				break;
			}

			history.Add(action, observation);
		}
	}

	Results.Time.Add(timer.elapsed());
	Results.UndiscountedReturn.Add(undiscountedReturn);
	Results.DiscountedReturn.Add(discountedReturn);
	cout << "Discounted return = " << discountedReturn
		<< ", average = " << Results.DiscountedReturn.GetMean() << endl;
	cout << "Undiscounted return = " << undiscountedReturn
		<< ", average = " << Results.UndiscountedReturn.GetMean() << endl;
	delete mcts;
}

void EXPERIMENT::MultiRun()
{
	int numberOfRuns = ExpParams.NumRuns;
	for (int n = 0; n < numberOfRuns; n++)
	{
		cout << "Starting run " << n + 1 << " with "
			<< SearchParams.NumSimulations << " simulations... " << endl;
		Run();
		if (Results.Time.GetTotal() > ExpParams.TimeOut)
		{
			cout << "Timed out after " << n << " runs in "
				<< Results.Time.GetTotal() << "seconds" << endl;
			break;
		}
	}
}

void EXPERIMENT::DiscountedReturn()
{
	cout << "Main runs" << endl;
	OutputFile << "Simulations,Runs,Undiscounted return,Undiscounted error,Discounted return,Discounted error,Time\n";

	ExpParams.SimSteps = Simulator.GetHorizon(ExpParams.Accuracy, ExpParams.UndiscountedHorizon);
	ExpParams.NumSteps = Real.GetHorizon(ExpParams.Accuracy, ExpParams.UndiscountedHorizon);

	for (int i = ExpParams.MinDoubles; i <= ExpParams.MaxDoubles; i++)
	{
		SearchParams.NumSimulations = 1 << i; // equivalent to 2**i
		SearchParams.NumStartStates = 1 << i; 

		// compute how many particles should be transformed
		if (i + ExpParams.TransformDoubles >= 0)
			SearchParams.NumTransforms = 1 << (i + ExpParams.TransformDoubles); // equivalent to 2**(i + ExpParams.TransformDoubles)
		else
			SearchParams.NumTransforms = 1;
		SearchParams.MaxAttempts = SearchParams.NumTransforms * ExpParams.TransformAttempts;

		Results.Clear();
		MultiRun();

		cout << "Simulations = " << SearchParams.NumSimulations << endl
			<< "Runs = " << Results.Time.GetCount() << endl
			<< "Undiscounted return = " << Results.UndiscountedReturn.GetMean()
			<< " +- " << Results.UndiscountedReturn.GetStdErr() << endl
			<< "Discounted return = " << Results.DiscountedReturn.GetMean()
			<< " +- " << Results.DiscountedReturn.GetStdErr() << endl
			<< "Time = " << Results.Time.GetMean() << endl;
		OutputFile << SearchParams.NumSimulations << ","
			<< Results.Time.GetCount() << ","
			<< Results.UndiscountedReturn.GetMean() << ","
			<< Results.UndiscountedReturn.GetStdErr() << ","
			<< Results.DiscountedReturn.GetMean() << ","
			<< Results.DiscountedReturn.GetStdErr() << ","
			<< Results.Time.GetMean() << endl;
	}
}

void EXPERIMENT::AverageReward()
{
	cout << "Main runs" << endl;
	OutputFile << "Simulations,Steps,Average reward,Average time\n";

	ExpParams.SimSteps = Simulator.GetHorizon(ExpParams.Accuracy, ExpParams.UndiscountedHorizon);

	for (int i = ExpParams.MinDoubles; i <= ExpParams.MaxDoubles; i++)
	{
		SearchParams.NumSimulations = 1 << i;
		SearchParams.NumStartStates = 1 << i;
		if (i + ExpParams.TransformDoubles >= 0)
			SearchParams.NumTransforms = 1 << (i + ExpParams.TransformDoubles);
		else
			SearchParams.NumTransforms = 1;
		SearchParams.MaxAttempts = SearchParams.NumTransforms * ExpParams.TransformAttempts;

		Results.Clear();
		Run();

		cout << "Simulations = " << SearchParams.NumSimulations << endl
			<< "Steps = " << Results.Reward.GetCount() << endl
			<< "Average reward = " << Results.Reward.GetMean()
			<< " +- " << Results.Reward.GetStdErr() << endl
			<< "Average time = " << Results.Time.GetMean() / Results.Reward.GetCount() << endl;
		OutputFile << SearchParams.NumSimulations << ","
			<< Results.Reward.GetCount() << ","
			<< Results.Reward.GetMean() << ","
			<< Results.Reward.GetStdErr() << ","
			<< Results.Time.GetMean() / Results.Reward.GetCount() << endl;
		OutputFile.flush();
	}
}

//----------------------------------------------------------------------------
