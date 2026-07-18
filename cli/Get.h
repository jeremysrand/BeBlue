#include "cli/Command.h"

#include "common/BlueSCSIGet.h"


#ifndef CLI_GET_H
#define CLI_GET_H


// Interface

class Get : public Command, BlueSCSIGetErrorHandler {
	public:
		Get();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
		virtual void HandleGetError(const char * err, status_t status);
		
	private:
		const char * src;
		const char * dest;
};


#endif