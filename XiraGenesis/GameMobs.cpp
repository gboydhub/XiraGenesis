#include "Game.h"

void Scene::CreateMobs()
{
	cMob New;

	New.Name = "Slime";
	New.Type = 0;
	New.Index = 0;
	GameMobs.push_back(New);
}