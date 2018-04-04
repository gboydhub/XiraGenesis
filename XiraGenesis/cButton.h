#ifndef _CBUTTON_H_
#define _CBUTTON_H_

#include "Game.h"

#define BSTATE_UP 0
#define BSTATE_DOWN 1
#define BSTATE_DISABLED 2

class cButton
{
public:
	cButton();
	~cButton();

	bool Init(ALLEGRO_BITMAP *Image, int w, int h, int x, int y, int State, int ID);
	void Draw();
	void Update();

	int GetState() { return ButtonState; };
	bool Pressed() { return WasPressed; }

private:
	int ButtonState;
	int Width;
	int Height;
	int X;
	int Y;
	int ID;
	bool WasPressed;
	ALLEGRO_BITMAP *Image;
	bool NewClick;
	bool ClickedIn;
};

#endif