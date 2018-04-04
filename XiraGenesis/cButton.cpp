#include "Game.h"

cButton::cButton()
{
	Width = 0;
	Height = 0;
	X = 0;
	Y = 0;
	Image = NULL;
	ID = 0;
	WasPressed = false;
	NewClick = false;
	ClickedIn = false;
	ButtonState = BSTATE_DISABLED;
}

cButton::~cButton()
{
	Image = NULL;
}

void cButton::Draw()
{
	al_draw_bitmap_region(Image, Width * ButtonState, ID * Height, Width, Height, X, Y, NULL);
}

bool cButton::Init(ALLEGRO_BITMAP *Image, int w, int h, int x, int y, int State, int ID)
{
	ButtonState = State;
	Width = w;
	Height = h;
	this->Image = Image;
	this->ID = ID;
	this->X = x;
	this->Y = y;

	return true;
}

void cButton::Update()
{
	ALLEGRO_MOUSE_STATE state;
	al_get_mouse_state(&state);

	if((state.buttons & 1))
	{
		if(NewClick == false || ClickedIn == true)
		{
			NewClick = true;
			if(state.x > X && state.x < X + Width && state.y > Y && state.y < Y + Height)
			{
				if(ButtonState == BSTATE_UP)
				{
					ButtonState = BSTATE_DOWN;
					ClickedIn = true;
				}
			}
		}
		if(state.x < X || state.x > X + Width || state.y < Y || state.y > Y + Height)
		{
			ButtonState = BSTATE_UP;
		}
	}
	else
	{
		NewClick = false;
		ClickedIn = false;
		if(state.x > X && state.x < X + Width && state.y > Y && state.y < Y + Height)
		{
			if(ButtonState == BSTATE_DOWN)
			{
				WasPressed = true;
				ButtonState = BSTATE_UP;
			}
		}
		else
		{
			ButtonState = BSTATE_UP;
		}
	}
}