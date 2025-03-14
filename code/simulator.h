#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "history.h"
#include "node.h"
#include "utils.h"
#include <iostream>
#include <math.h>

class BELIEF_STATE;

class STATE : public MEMORY_OBJECT
{
};

class SIMULATOR
{
public:

	// how much knowledge the simulator uses
	struct KNOWLEDGE
	{
		enum
		{
			PURE = 0,
			LEGAL = 1, // only legal actions
			SMART = 2, // preferred actions
			NUM_LEVELS = 3
		};

		// construct with defaults
		KNOWLEDGE();

		// construct based on file
		KNOWLEDGE(const std::string& filename);

		int RolloutLevel; // knowledge level to use for rollouts
		int TreeLevel; // knowledge level to use within the tree
		int SmartTreeCount; // initial visit count for preferred nodes
		double SmartTreeValue; // initial value for preferred nodes

		int Level(int phase) const
		{
			assert(phase < STATUS::NUM_PHASES);
			if (phase == STATUS::TREE)
				return TreeLevel;
			else
				return RolloutLevel;
		}
	};

	struct STATUS
	{
		STATUS();

		enum
		{
			TREE,
			ROLLOUT,
			NUM_PHASES
		};

		enum
		{
			CONSISTENT,
			INCONSISTENT,
			RESAMPLED,
			OUT_OF_PARTICLES
		};

		int Phase;
		int Particles;
	};

	SIMULATOR();
	SIMULATOR(int numActions, int numObservations, double discount = 1.0);
	virtual ~SIMULATOR();

	// Create start start state (can be stochastic)
	virtual STATE* CreateStartState() const = 0;

	// Free memory for state
	virtual void FreeState(STATE* state) const = 0;

	// Update state according to action, and get observation and reward. 
	// Return value of true indicates termination of episode (if episodic)
	virtual bool Step(STATE& state, int action,
		int& observation, double& reward) const = 0;

	// Create new state and copy argument (must be same type)
	virtual STATE* Copy(const STATE& state) const = 0;

	// Sanity check
	virtual void Validate(const STATE& state) const;

	// Modify state stochastically to some related state
	virtual bool LocalMove(STATE& state, const HISTORY& history,
		int stepObs, const STATUS& status) const;

	// Use domain knowledge to assign prior value and confidence to actions
	// Should only use fully observable state variables
	void Prior(const STATE* state, const HISTORY& history, VNODE* vnode,
		const STATUS& status) const;

	// Use domain knowledge to select actions stochastically during rollouts
	// Should only use fully observable state variables
	int SelectRandom(const STATE& state, const HISTORY& history,
		const STATUS& status) const;

	// Generate set of legal actions
	virtual void GenerateLegal(const STATE& state, const HISTORY& history,
		std::vector<int>& actions, const STATUS& status) const;

	// Generate set of preferred actions
	virtual void GeneratePreferred(const STATE& state, const HISTORY& history,
		std::vector<int>& actions, const STATUS& status) const;

	// For explicit POMDP computation only
	virtual bool HasAlpha() const;
	virtual void AlphaValue(const QNODE& qnode, double& q, int& n) const;
	virtual void UpdateAlpha(QNODE& qnode, const STATE& state) const;

	// Textual display
	virtual void DisplayBeliefs(const BELIEF_STATE& beliefState,
		std::ostream& ostr) const;
	virtual void DisplayState(const STATE& state, std::ostream& ostr) const;
	virtual void DisplayAction(int action, std::ostream& ostr) const;
	virtual void DisplayObservation(const STATE& state, int observation, std::ostream& ostr) const;
	virtual void DisplayReward(double reward, std::ostream& ostr) const;

	// Accessors
	void SetKnowledge(const KNOWLEDGE& knowledge) { Knowledge = knowledge; }
	int GetNumActions() const { return NumActions; }
	int GetNumObservations() const { return NumObservations; }
	bool IsEpisodic() const { return false; }
	double GetDiscount() const { return Discount; }
	double GetRewardRange() const { return RewardRange; }
	double GetHorizon(double accuracy, int undiscountedHorizon = 100) const;
	void GenerateActionSpace(const STATE& state, const HISTORY& history,
		std::vector<int>& actions, const STATUS& status, const bool preferred) const;

protected:

	int NumActions, NumObservations;
	double Discount, RewardRange;
	KNOWLEDGE Knowledge;
};

#endif // SIMULATOR_H
