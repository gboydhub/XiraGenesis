//Particle system base
//9/30/13 Gary Boyd

#ifndef _CPARTICLE_H_
#define _CPARTICLE_H_

#include "Game.h"

class cParticle
{
public:
	cParticle(){};
	cParticle(float X, float Y, float XOff, float YOff, int SourceX, int SourceY, int Width, int Height, int Life, int StartDelay, ALLEGRO_BITMAP *Image, int MapID);

	void Draw(int XOffset, int YOffset);
	virtual void Tick() = 0;

	bool IsDead() { if(Life > 0) return false; return true;}
	int GetMapID() { return MapID; }

	void SetColorMask(float Red, float Green, float Blue, float Alpha);

protected:
	float X;
	float Y;
	float XOff;
	float YOff;
	float CenterX;
	float CenterY;

	float XScale;
	float YScale;

	float Angle;
	int MapID; 

	int SourceX;
	int SourceY;
	int Width;
	int Height;

	int InitialLife;
	int Life;
	int StartDelay;

	float AlphaMod;
	float RedMod;
	float GreenMod;
	float BlueMod;

	ALLEGRO_BITMAP *Image;
};

#endif