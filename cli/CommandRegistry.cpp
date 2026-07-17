#include <string.h>

#include "cli/CommandRegistry.h"
#include "cli/Testing.h"


// Implementation

CommandRegistry::CommandRegistry()
	: commands()
{
	commands.AddItem(new Testing());	
}


CommandRegistry::~CommandRegistry()
{
	for (int32 i = 0; i < commands.CountItems(); i++) {
		Command * command = (Command *)commands.ItemAt(i);
		delete command;
	}
}


Command * CommandRegistry::GetCommand(const char * cmd)
{
	for (int32 i = 0; i < commands.CountItems(); i++) {
		Command * command = (Command *)commands.ItemAt(i);
		if (strcmp(command->Command(), cmd) == 0)
			return command;
	}
	return NULL;
}


void CommandRegistry::PrintUsage()
{
	for (int32 i = 0; i < commands.CountItems(); i++) {
		Command * command = (Command *)commands.ItemAt(i);
		fprintf(stderr, "    %s\n", command->Usage());
	}
}