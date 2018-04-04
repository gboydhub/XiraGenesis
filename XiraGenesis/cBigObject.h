#ifndef _CBIGOBJECT_H_
#define _CBIGOBJECT_H_

#include "Game.h"

#define BIGOBJ_HOUSE	0
#define BIGOBJ_CAVE		1

class cBigObject
{
public:
	int X;
	int Y;
	int XOff;
	int YOff;
	int Type;
};

#endif