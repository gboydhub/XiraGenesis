#include "Game.h"

class cMob
{
public:
	cMob();

	int Index;
	int Type;

	string Name;

	int X;
	int Y;
	int XOff;
	int YOff;
	int TakingStep;
	int StepFreq;
	int NeedsStep;
	bool NeedsBlockCheck;
	int ID;

	int TargetPlayerID;
	bool Aggressive;

	void Tick();
	void HSTick();
};