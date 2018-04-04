#include "ParticleTypes.h"

void PE_Fader::Tick()
{
	if(StartDelay > 0)
	{
		StartDelay--;
		return;
	}
	if(Life > 0)
	{
		Life--;
		X += XOff;
		Y += YOff;

		YOff += Gravity;
	}

	AlphaMod = (float)Life / (float)InitialLife;
}

void PE_Changer::Tick()
{
	if(StartDelay > 0)
	{
		StartDelay--;
		return;
	}

	DelayCounter--;
	if(DelayCounter == 0)
	{
		DelayCounter = FrameDelay;
		SourceX += XOff;
		SourceY += YOff;
	}

	Life--;
}