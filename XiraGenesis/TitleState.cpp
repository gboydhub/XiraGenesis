#include "Game.h"

void TitleState::Init()
{
	Background = al_load_bitmap("TitleBG.png");
	Buttons = al_load_bitmap("TitleButtons.png");

	HostButton.Init(Buttons, 102, 82, 250, 250, BSTATE_UP, 0);
	JoinButton.Init(Buttons, 102, 82, 450, 250, BSTATE_UP, 1);
}

void TitleState::ProcessInput(int KeyCode, bool KeyDown)
{
}

bool TitleState::DoLogic()
{
	HostButton.Update();
	JoinButton.Update();

	if(HostButton.Pressed())
	{
		NextScene = SCENE_SERVERSCENE;
	}
	if(JoinButton.Pressed())
	{
		NextScene = SCENE_GAMESCENE;
	}
	return true;
}

void TitleState::Draw()
{
	al_draw_bitmap(Background, 0, 0, NULL);
	HostButton.Draw();
	JoinButton.Draw();
}

void TitleState::ResetState()
{
}