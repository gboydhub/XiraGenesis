#ifndef _CCOMMAND_H_
#define _CCOMMAND_H_

#include "Game.h"
class ServerState;

#define CMDTYPE_GENERAL 0
#define CMDTYPE_SKILL	1

typedef void (ServerState::*PFN)(int);

class cCommand
{
public:
	int				Type;
	string			Name;
	string			ShortDesc;
	vector<string>	LongDesc;
	PFN				Function;
};

#endif