#ifndef _TITLESTATE_H_
#define _TITLESTATE_H_

#include "Game.h"

class TitleState : public Scene
{
public:
	void Init();
	void ProcessInput(int KeyCode, bool KeyDown);
	bool DoLogic();
	void Draw();
	void ResetState();

private:
	ALLEGRO_BITMAP *Background;
	ALLEGRO_BITMAP *Buttons;
	cButton HostButton;
	cButton JoinButton;
};

#endif