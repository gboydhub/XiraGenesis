#include "Game.h"

cObj::cObj()
{
	Index = 0;
	Type = OBJTYPE_NONE;
	ID = -1;
	Quantity = 0;
	X = 0;
	Y = 0;
	Name = "NullObj";
	Equipped = false;
}