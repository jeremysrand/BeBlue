#include <List.h>

#ifndef CLI_COMMAND_REGISTRY_H
#define CLI_COMMAND_REGISTRY_H

// Forward declarations

class Command;


// Interface

class CommandRegistry
{
	public:
		CommandRegistry();
		~CommandRegistry();
		
		Command * GetCommand(const char * cmd);
		void PrintUsage();

	private:
		BList commands;	
};


#endif