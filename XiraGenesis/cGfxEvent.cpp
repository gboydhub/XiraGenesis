#include "Game.h"

void cGfxEvent::Tick()
{
	if(EventID == GFXEVENT_TFE)
	{
		Y--;
		AlphaMod -= 0.02f;
		if(Y < StartY - 50)
			IsDone = true;
	}
	else if(EventID == GFXEVENT_TFPATH)
	{
		Y--;
		AlphaMod -= 0.02f;
		if(Y < StartY - 50)
			IsDone = true;
	}
	else if(EventID == GFXEVENT_TFCAVE)
	{
		Ticks++;
		if(Ticks > 5)
		{
			Counter = 5;
		}
		if(Ticks > 50)
		{
			Counter = 6;
		}
		if(Ticks >= 90)
		{
			IsDone = true;
		}
	}
	else if(EventID == GFXEVENT_BUHS)
	{
		Ticks++;
		if(Ticks > 5)
		{
			Counter = 9;
		}
		if(Ticks > 50)
		{
			Counter = 10;
		}
		if(Ticks >= 90)
		{
			IsDone = true;
		}
	}
	else if(EventID == GFXEVENT_BUFO)
	{
		Ticks++;
		if(Ticks > 5)
		{
			Counter = 12;
		}
		if(Ticks > 50)
		{
			Counter = 13;
		}
		if(Ticks >= 90)
		{
			IsDone = true;
		}
	}
}

void cGfxEvent::Draw()
{
	if(EventID == GFXEVENT_TFE)
	{
		al_draw_tinted_bitmap_region(Image, al_map_rgba_f(1 * AlphaMod, 1 * AlphaMod, 1 * AlphaMod, AlphaMod), 64, 0, 32 - 10, 32 - 10, X + 5, Y + 5, NULL);
	}
	else if(EventID == GFXEVENT_TFPATH)
	{
		al_draw_tinted_bitmap_region(Image, al_map_rgba_f(1 * AlphaMod, 1 * AlphaMod, 1 * AlphaMod, AlphaMod), 64, 0, 32 - 10, 32 - 10, X + 5, Y + 5, NULL);
	}
	else if(EventID == GFXEVENT_TFCAVE)
	{
		al_draw_bitmap_region(Image, Counter * 32, 0, 32, 32, X, Y, 0);
	}
	else if(EventID == GFXEVENT_BUHS)
	{
		al_draw_bitmap_region(Image, Counter * 32, 0, 32, 32, X, Y, 0);
	}
	else if(EventID == GFXEVENT_BUFO)
	{
		al_draw_bitmap_region(Image, Param * 32, 0, 32, 32, X, Y, 0);
		al_draw_bitmap_region(Image, Counter * 32, 0, 32, 32, X, Y, 0);
	}
}