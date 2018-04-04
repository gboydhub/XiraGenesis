#include "Game.h"

cMob::cMob()
{
	X = 0;
	XOff = 0;
	Y = 0;
	YOff = 0;
	Index = 0;
	Type = 0;
	TakingStep = 0;
	ID = 0;
	Name = "NA";
	Aggressive = false;
	TargetPlayerID = 0;

	StepFreq = 0;
	NeedsStep = 0;
	NeedsBlockCheck = false;
}

void cMob::Tick()
{
	if(TakingStep > 0)
	{
		/*switch(TakingStep)
		{
		}*/
	}
}

void cMob::HSTick()
{
	if(TakingStep == 0 && StepFreq > 0)
	{
		NeedsStep++;
		if(NeedsStep == StepFreq)
		{
			NeedsStep = 0;
			TakingStep = MOVEDIR_UP;
			NeedsBlockCheck = true;
		}
	}
}