#include <string.h>

#include "cli/Capabilities.h"
#include "cli/CommandRegistry.h"
#include "cli/Debug.h"
#include "cli/Devices.h"
#include "cli/Files.h"
#include "cli/Get.h"
#include "cli/Inquiry.h"
#include "cli/Scan.h"
#include "cli/Send.h"
#include "cli/Testing.h"
#include "cli/WorkingDir.h"


// Implementation

CommandRegistry::CommandRegistry()
	: commands()
{
	commands.AddItem(new Scan());
	commands.AddItem(new Inquiry());
	commands.AddItem(new Capabilities());
	commands.AddItem(new Debug());
	commands.AddItem(new Devices());
	commands.AddItem(new WorkingDir());
	commands.AddItem(new Files());
	commands.AddItem(new Get());
	commands.AddItem(new Send());
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