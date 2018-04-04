#include "Game.h"

cParticle::cParticle(float X, float Y, float XOff, float YOff, int SourceX, int SourceY, int Width, int Height, int Life, int StartDelay, ALLEGRO_BITMAP *Image, int MapID)
{
	this->X = X;
	this->Y = Y;
	this->XOff = XOff;
	this->YOff = YOff;
	this->SourceX = SourceX;
	this->SourceY = SourceY;
	this->Width = Width;
	this->Height = Height;
	this->Life = Life;
	this->InitialLife = Life;
	this->Image = Image;
	this->StartDelay = StartDelay;
	this->MapID = MapID;

	this->XScale = 1.0f;
	this->YScale = 1.0f;
	
	this->CenterX = Width/2;
	this->CenterY = Height/2;
	this->Angle = 0.0f;
	
	this->RedMod = 1.0f;
	this->GreenMod = 1.0f;
	this->BlueMod = 1.0f;
	this->AlphaMod = 1.0f;
}

void cParticle::Draw(int XOffset, int YOffset)
{
	if(StartDelay > 0)
		return;

	al_draw_tinted_scaled_rotated_bitmap_region(Image, SourceX, SourceY, Width, Height, al_map_rgba_f(RedMod * AlphaMod, GreenMod * AlphaMod, BlueMod * AlphaMod, AlphaMod),
		CenterX, CenterY, X + CenterX + XOffset, Y + CenterY + YOffset, XScale, YScale, Angle, 0);
}

void cParticle::SetColorMask(float Red, float Green, float Blue, float Alpha)
{
	RedMod = Red;
	BlueMod = Blue;
	GreenMod = Green;
	AlphaMod = Alpha;
}