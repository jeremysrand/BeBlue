#include "cli/Command.h"

#include "common/BlueSCSISend.h"

#ifndef CLI_SEND_H
#define CLI_SEND_H


// Interface

class Send : public Command, BlueSCSISendErrorHandler {
	public:
		Send();
		
		virtual const char * Command();
		virtual const char * Usage();
		virtual bool RequiresOneDevice();
		virtual bool ParseArgs(int argc, const char * argv[]);
		virtual int Execute();
		
		virtual void HandleSendError(const char * err, status_t status);
		
	private:
		const char * src;
		const char * dest;
};


#endif