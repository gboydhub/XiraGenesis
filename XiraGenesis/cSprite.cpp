#include "cSprite.h"

cSprite::cSprite()
{
	X = 0;
	Y = 0;
	CurFrame = 0;
	FrameWidth = 0;
	FrameHeight = 0;

	Animated = false;
	AnimSpeed = 0;
	StartFrame = 0;
	EndFrame = 0;
}

cSprite::cSprite(ALLEGRO_BITMAP *Image, int X, int Y, int FrameW, int FrameH, int CurFrame)
{
	this->Image = Image;
	this->X = X;
	this->Y = Y;
	this->CurFrame = CurFrame;
	FrameWidth = FrameW;
	FrameHeight = FrameH;

	FramesAcross = al_get_bitmap_width(Image) / FrameWidth;
	FramesDown = al_get_bitmap_height(Image) / FrameHeight;

	Animated = false;
	AnimSpeed = 0;
	StartFrame = 0;
	EndFrame = 0;
}

cSprite::cSprite(ALLEGRO_BITMAP *Image, int X, int Y, int FrameW, int FrameH, int CurFrame, int AnimStart, int AnimEnd, int AnimSpeed)
{
	this->Image = Image;
	this->X = X;
	this->Y = Y;
	this->CurFrame = CurFrame;
	FrameWidth = FrameW;
	FrameHeight = FrameH;

	FramesAcross = al_get_bitmap_width(Image) / FrameWidth;
	FramesDown = al_get_bitmap_height(Image) / FrameHeight;

	Animated = false;
	this->AnimSpeed = AnimSpeed;
	StartFrame = AnimStart;
	EndFrame = AnimEnd;
}

void cSprite::AddOverlay(ALLEGRO_BITMAP *Overlay, int SourceX, int SourceY, int DestX, int DestY, int Width, int Height)
{
	al_set_target_bitmap(Image);
	al_draw_bitmap_region(Overlay, SourceX, SourceY, Width, Height, DestX, DestY, NULL);
	al_set_target_backbuffer(al_get_current_display());
}

void cSprite::AddOverlay(ALLEGRO_BITMAP *Overlay, int SourceX, int SourceY, int DestX, int DestY, int Width, int Height, ALLEGRO_COLOR Mask)
{
	al_set_target_bitmap(Image);
	al_draw_tinted_bitmap_region(Overlay, Mask, SourceX, SourceY, Width, Height, DestX, DestY, NULL);
	al_set_target_backbuffer(al_get_current_display());
}

void cSprite::Draw()
{
	int CurFX = (CurFrame % FramesAcross) * FrameWidth;
	int CurFY = (CurFrame / FramesAcross) * FrameHeight;
	al_draw_bitmap_region(Image, CurFX, CurFY, FrameWidth, FrameHeight, X, Y, NULL);
}
