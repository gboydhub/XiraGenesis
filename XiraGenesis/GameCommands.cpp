#include "Game.h"

void ServerState::CreateCommands()
{
	cCommand NewCommand;
	
	NewCommand.Name = "Help";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Shows help topics or help on a specific topic.";
	NewCommand.LongDesc.push_back("Shows a list of commands, or <C>help <W>on a specific <Y>topic<W>.");
	NewCommand.LongDesc.push_back("Example: <C>help <Y>h");
	NewCommand.Function = &ServerState::UserCmd_Help;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Time";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Shows you this worlds current time and date.";
	NewCommand.LongDesc.push_back("Shows you the current worlds date and time.");
	NewCommand.LongDesc.push_back("Format: <C>DD <G>HH<W>:<G>MM");
	NewCommand.Function = &ServerState::UserCmd_Time;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();
	
	NewCommand.Name = "Quit";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Disconnect from the server.";
	NewCommand.LongDesc.push_back("Log out of the server.");
	NewCommand.Function = &ServerState::UserCmd_Quit;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Say";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Say something to the world.";
	NewCommand.LongDesc.push_back("<C>Say <W>a <Y>message <W>into the chat window.");
	NewCommand.LongDesc.push_back("Example: <C>s <Y>Hello!");
	NewCommand.Function = &ServerState::UserCmd_Say;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Inventory";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "View the items in your inventory.";
	NewCommand.LongDesc.push_back("Use this command to see whats in your <C>inventory<W>.");
	NewCommand.Function = &ServerState::UserCmd_Inventory;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Drop";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Drop an item.";
	NewCommand.LongDesc.push_back("<C>Drop <W>an <Y>item <W>onto the ground.");
	NewCommand.LongDesc.push_back("Example: <C>dr <Y>potion <W>Will <C>drop <W>a <Y>potion <W>onto the ground.");
	NewCommand.LongDesc.push_back("Example; <C>drop <Y>all <W>Will <C>drop <W>everything in your inventory.");
	NewCommand.LongDesc.push_back("Example: <C>dr <Y>all pot <W>Will <C>drop <Y>all <W>of your <Y>potions <W>onto the ground.");
	NewCommand.Function = &ServerState::UserCmd_Drop;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Pickup";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Pickup an item.";
	NewCommand.LongDesc.push_back("<C>Pickup <W>an <Y>item <W>that is under you.");
	NewCommand.LongDesc.push_back("Example: <C>pickup <Y>potion <W>Will <C>pickup <W>a <Y>potion <W>off the ground.");
	NewCommand.LongDesc.push_back("Example; <C>pick <Y>all <W>Will <C>pickup <W>everything on the ground.");
	NewCommand.LongDesc.push_back("Example: <C>pick <Y>all pot <W>Will <C>pickup <Y>all <W>of the <Y>potions <W>on the ground.");
	NewCommand.Function = &ServerState::UserCmd_Pickup;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Equip";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Equip an item.";
	NewCommand.LongDesc.push_back("<C>Equip <W>an <Y>item <W>that is in your inventory.");
	NewCommand.LongDesc.push_back("Example: <C>eq <Y>st leg <W>Will <C>equip <Y>Steel Leggings <W>if you have them.");
	NewCommand.Function = &ServerState::UserCmd_Equip;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "UnEquip";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Remove an item.";
	NewCommand.LongDesc.push_back("<C>Unequip <W>an item that is currently equiped.");
	NewCommand.LongDesc.push_back("Available subcommands are: <Y>Armor<W>, <Y>Helmet<W>, <Y>Leggings<W>, <Y>Boots<W>, and <Y>All");
	NewCommand.LongDesc.push_back("Example: <C>unequip <Y>armor <W>Will <C>unequip <W>whatever armor you have equipped.");
	NewCommand.LongDesc.push_back("Example: <C>une <Y>all <W>Will <C>unequip <Y>everything <W>that you have equipped.");
	NewCommand.Function = &ServerState::UserCmd_UnEquip;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Join";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Join a party";
	NewCommand.LongDesc.push_back("<C>Join <W>the party of the <Y>person <W>standing beside you.");
	NewCommand.LongDesc.push_back("Example: <C>jo <Y>Gary <W>Would attempt to join a player named <Y>Gary<W>.");
	NewCommand.Function = &ServerState::UserCmd_Join;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Leave";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Leave a party.";
	NewCommand.LongDesc.push_back("<C>Leave <W>your current party.");
	NewCommand.LongDesc.push_back("Example: <C>leave <W>Will leave your current party.");
	NewCommand.Function = &ServerState::UserCmd_Leave;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "PartyInfo";
	NewCommand.Type = CMDTYPE_GENERAL;
	NewCommand.ShortDesc = "Shows party information.";
	NewCommand.LongDesc.push_back("Shows your current <C>party information<W>.");
	NewCommand.LongDesc.push_back("Example: <C>partyinfo <W>Will show the current members of your party, and some details.");
	NewCommand.Function = &ServerState::UserCmd_PartyInfo;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	

	NewCommand.Name = "TerraForm";
	NewCommand.Type = CMDTYPE_SKILL;
	NewCommand.ShortDesc = "Change the world around you.";
	NewCommand.LongDesc.push_back("You can form the world around you with this skill.");
	NewCommand.LongDesc.push_back("Available subcommands are: <Y>Cave<W>, and <Y>Exit<W>.");
	NewCommand.LongDesc.push_back("Example: <C>te <Y>cave east <W>Will raise a <Y>cave <W>from the ground to the east of you.");
	NewCommand.LongDesc.push_back("Example: <C>te <Y>ex <W>Will create a new <Y>exit <W>along the edge of the area you are at.");
	NewCommand.Function = &ServerState::UserCmd_Terraform;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Build";
	NewCommand.Type = CMDTYPE_SKILL;
	NewCommand.ShortDesc = "Build a structure.";
	NewCommand.LongDesc.push_back("This skill lets you <C>build <Y>structures <W>in the game.");
	NewCommand.LongDesc.push_back("Available subcommands are: <Y>House<W>, and <Y>Forge<W>.");
	NewCommand.LongDesc.push_back("Example: <C>build <Y>house east <W>Will build a <Y>house <W> to the east of you.");
	NewCommand.LongDesc.push_back("Example: <C>bu <Y>f s <W>Will <C>build <W>a <Y>forge <W>to the <Y>south <W>of you.");
	NewCommand.Function = &ServerState::UserCmd_Build;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Forge";
	NewCommand.Type = CMDTYPE_SKILL;
	NewCommand.ShortDesc = "Create a new item.";
	NewCommand.LongDesc.push_back("<C>Forge <W>a new <Y>item <W>and place it in your inventory.");
	NewCommand.LongDesc.push_back("Example: <C>forge <Y>list <W>Will show you a list of items you can create.");
	NewCommand.LongDesc.push_back("Example: <C>forge <Y>steel armor <W>Will create <W>a new set of <Y>Steel Armor <W>and place it in your inventory.");
	NewCommand.LongDesc.push_back("*<R>Note: <W>You must be standing beside a <C>forge <W>to use this skill.");
	NewCommand.Function = &ServerState::UserCmd_Forge;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Summon";
	NewCommand.Type = CMDTYPE_SKILL;
	NewCommand.ShortDesc = "Summon a creature.";
	NewCommand.LongDesc.push_back("<C>Summon <W>a <Y>creature <W>into existance.");
	NewCommand.LongDesc.push_back("Example: <C>summon <Y>slime <W>Will <C>summon <W>a <Y>Slime <W>creature that will attack other players.");
	NewCommand.Function = &ServerState::UserCmd_Summon;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();

	NewCommand.Name = "Battle";
	NewCommand.Type = 20;
	NewCommand.ShortDesc = "Debug Command - Jump into a battle";
	NewCommand.LongDesc.push_back("<P>Dont worry about it o.o");
	NewCommand.Function = &ServerState::UserCmd_Battle;
	CommandList.push_back(NewCommand);
	NewCommand.LongDesc.clear();
}