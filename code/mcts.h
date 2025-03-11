#ifndef MCTS_H
#define MCTS_H

#include "simulator.h"
#include "node.h"
#include "statistic.h"

class MCTS
{
public:

	struct PARAMS
	{
		// construct with defaults
		PARAMS();

		// construct based on file
		PARAMS(const std::string& filename);

		int Verbose;
		int MaxDepth; // maximum tree depth (search horizon)
		int NumSimulations; // number of simulations to perform
		int NumStartStates; // number of initial start states to sample
		bool UseTransforms; // whether or not to use transforms
		int NumTransforms; // number of transformed particles we want to have
		int MaxAttempts; // maximum number of times we try to transform particles
		int ExpandCount;
		int EnsembleSize; // NEVER USED
		double ExplorationConstant;
		bool UseRave;
		double RaveDiscount;
		double RaveConstant;
		bool DisableTree; // whether or not to use a basic monte carlo strategy (PO-rollout in the POMCP paper)
	};

	MCTS(const SIMULATOR& simulator, const PARAMS& params);
	virtual ~MCTS();

	virtual int SelectAction();
	bool Update(int action, int observation, double reward);

	void UCTSearch();
	void RolloutSearch();

	double Rollout(STATE& state);

	const BELIEF_STATE& BeliefState() const { return Root->Beliefs(); }
	const HISTORY& GetHistory() const { return History; }
	const SIMULATOR::STATUS& GetStatus() const { return Status; }
	void ClearStatistics();
	void DisplayStatistics(std::ostream& ostr) const;
	void DisplayValue(int depth, std::ostream& ostr) const;
	void DisplayPolicy(int depth, std::ostream& ostr) const;

	static void UnitTest();
	static void InitFastUCB(double exploration);

	int GreedyUCB(VNODE* vnode, bool ucb) const;
	int SelectRandom() const;
	double SimulateV(STATE& state, VNODE* vnode);
	double SimulateQ(STATE& state, QNODE& qnode, int action);
	void AddRave(VNODE* vnode, double totalReward);
	VNODE* ExpandNode(const STATE* state);
	void AddSample(VNODE* node, const STATE& state);
	void AddTransforms(VNODE* root, BELIEF_STATE& beliefs);
	STATE* CreateTransform() const;
	void Resample(BELIEF_STATE& beliefs);

	// Fast lookup table for UCB
	static const int UCB_N = 10000, UCB_n = 100;
	static double UCB[UCB_N][UCB_n];
	static bool InitialisedFastUCB;

	double FastUCB(int N, int n, double logN) const;
	const SIMULATOR& Simulator;
	int TreeDepth, PeakTreeDepth;
	PARAMS Params;
	VNODE* Root;
	HISTORY History;
	SIMULATOR::STATUS Status;
	STATISTIC StatTreeDepth;
	STATISTIC StatRolloutDepth;
	STATISTIC StatTotalReward;
private:
	static void UnitTestGreedy();
	static void UnitTestUCB();
	static void UnitTestRollout();
	static void UnitTestSearch(int depth);
};

#endif // MCTS_H
