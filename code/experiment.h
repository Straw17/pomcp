#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "mcts.h"
#include "simulator.h"
#include "statistic.h"
#include <fstream>

//----------------------------------------------------------------------------

struct RESULTS
{
	void Clear();

	STATISTIC Time;
	STATISTIC Reward;
	STATISTIC DiscountedReturn;
	STATISTIC UndiscountedReturn;
};

inline void RESULTS::Clear()
{
	Time.Clear();
	Reward.Clear();
	DiscountedReturn.Clear();
	UndiscountedReturn.Clear();
}

//----------------------------------------------------------------------------

class EXPERIMENT
{
public:

	struct PARAMS
	{
		PARAMS();

		int NumRuns; // number of runs for each time setting
		int NumSteps; // maximum number of steps for each run
		int SimSteps; // NEVER USED
		double TimeOut; // maximum time allowed
		int MinDoubles; // lowest power of two # of simulations to test with (inclusive)
		int MaxDoubles; // highest power of two # of simulations to test with (inclusive)
		int TransformDoubles; // power of two fraction of particles that should be transformed (ex: -4 means 1/16 of particles are transformed)
		int TransformAttempts; // how many times to try to transform before failing
		double Accuracy; // smallest precision we care about - used to calculate horizon for discounted tasks
		int UndiscountedHorizon; // used to calculate the horizon for undiscounted tasks
		bool AutoExploration; // whether to set the exploration constant by finding a reward range
	};

	EXPERIMENT(const SIMULATOR& real, const SIMULATOR& simulator,
		const std::string& outputFile,
		EXPERIMENT::PARAMS& expParams, MCTS::PARAMS& searchParams);

	void Run();
	void MultiRun();
	void DiscountedReturn();
	void AverageReward();

private:

	const SIMULATOR& Real;
	const SIMULATOR& Simulator;
	EXPERIMENT::PARAMS& ExpParams;
	MCTS::PARAMS& SearchParams;
	RESULTS Results;

	std::ofstream OutputFile;
};

//----------------------------------------------------------------------------

#endif // EXPERIMENT_H
