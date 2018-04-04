#ifndef _CSPRITE_H_
#define _CSPRITE_H_

#include "Game.h"

class cSprite
{
public:
	cSprite();
	cSprite(ALLEGRO_BITMAP *Image, int X, int Y, int FrameW, int FrameH, int CurFrame);
	cSprite(ALLEGRO_BITMAP *Image, int X, int Y, int FrameW, int FrameH, int CurFrame, int AnimStart, int AnimEnd, int AnimSpeed);

	void AddOverlay(ALLEGRO_BITMAP *Overlay, int SourceX, int SourceY, int DestX, int DestY, int Width, int Height);
	void AddOverlay(ALLEGRO_BITMAP *Overlay, int SourceX, int SourceY, int DestX, int DestY, int Width, int Height, ALLEGRO_COLOR Mask);
	
	void Draw();

	void StartAnimation()						{ Animated = true; }
	void StopAnimation()						{ Animated = false; }
	void SetAnimationSpeed(int Speed)			{ AnimSpeed = Speed; }

	void SetLoc(int X, int Y)					{ this->X = X; this->Y = Y; }
	void SetFrameInfo(int Width, int Height)	{ FrameWidth = Width; FrameHeight = Height; }
	void SetFrame(int Frame)					{ CurFrame = Frame; }
	~cSprite()									{ al_destroy_bitmap(Image); }

private:

	ALLEGRO_BITMAP *Image;

	int X;
	int Y;
	int FrameWidth;
	int FrameHeight;
	int CurFrame;

	int FramesAcross;
	int FramesDown;

	bool Animated;
	int AnimSpeed;
	int StartFrame;
	int EndFrame;
};

#endif