#include "Game.h"

//9/30/13 TODO: Broadcast these to everyone in a 2-3 block radius, and add a flag to display event only if you walk onto its proper map

class cGfxEvent
{
public:
	cGfxEvent() {EventID = 0; X = 0; Y = 0; IsDone = false; StartX = 0; StartY = 0; Ticks = 0; Counter = 0; Param = 0;};

	void Tick();
	void Draw();
	int EventID;
	int X;
	int Y;
	int StartX;
	int StartY;
	int Ticks;
	int Counter;
	int Param;
	float AlphaMod;
	bool IsDone;

	ALLEGRO_BITMAP *Image;
};