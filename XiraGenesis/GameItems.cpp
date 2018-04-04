#include "Game.h"

void Scene::CreateItems()
{
	cObj NewO;
	
	NewO.Quantity = 1;
	NewO.Index = 0;
	NewO.ID = 0;

	NewO.Type = OBJTYPE_ARMOR;
	NewO.Index = 1;
	NewO.Name = "Steel Armor";
	GameItems.push_back(NewO);
	
	NewO.ID ++;
	NewO.Index = 2;
	NewO.Type = OBJTYPE_LEGS;
	NewO.Name = "Steel Leggings";
	GameItems.push_back(NewO);
	
	NewO.ID ++;
	NewO.Index = 3;
	NewO.Type = OBJTYPE_BOOTS;
	NewO.Name = "Steel Boots";
	GameItems.push_back(NewO);
	
	NewO.ID ++;
	NewO.Index = 0;
	NewO.Type = OBJTYPE_HELM;
	NewO.Name = "Steel Helmet";
	GameItems.push_back(NewO);
}