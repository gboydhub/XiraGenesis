#ifndef _PARTICLETYPES_H_
#define _PARTICLETYPES_H_

#include "Game.h"

class PE_Fader : public cParticle
{
public:
	PE_Fader(float X, float Y, float XOff, float YOff, int SourceX, int SourceY, int Width, int Height, int Life, int StartDelay, ALLEGRO_BITMAP *Image, int MapID, float Gravity) : cParticle(X, Y, XOff,  YOff, SourceX, SourceY, Width, Height, Life, StartDelay, Image, MapID) { this->Gravity = Gravity; };
	void Tick();

	float Gravity;
};

class PE_Changer : public cParticle
{
public:
	PE_Changer(float X, float Y, float XOff, float YOff, int SourceX, int SourceY, int Width, int Height, int Life, int StartDelay, ALLEGRO_BITMAP *Image, int MapID, float Frames) : cParticle(X, Y, XOff, YOff, SourceX, SourceY, Width, Height, Life, StartDelay, Image, MapID) { this->Frames = Frames; this->FrameDelay = Life / Frames; this->DelayCounter = this->FrameDelay;}
	void Tick();

	int Frames;

protected:
	int CurFrame;
	int FrameDelay;
	int DelayCounter;
};

#endif