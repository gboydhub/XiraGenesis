#include "Game.h"

#define OBJTYPE_NONE		0
#define OBJ_EQUIPSETS		1
#define OBJTYPE_BEGINEQUIP	10
#define OBJTYPE_ARMOR		11
#define OBJTYPE_LEGS		12
#define OBJTYPE_BOOTS		13
#define OBJTYPE_HELM		14
#define OBJTYPE_ENDEQUIP	15

class cObj
{
public:
	cObj();

	int Type;
	int Index;
	int ID;
	int X;
	int Y;
	int Quantity;
	bool Equipped;

	string Name;
private:
};