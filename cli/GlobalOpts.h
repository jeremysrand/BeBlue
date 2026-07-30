#ifndef CLI_CLOBAL_OPTS_H
#define CLI_GLOBAL_OPTS_H


// Forward declarations

class BlueSCSIDevice;
class FileLogger;


// Interface

class GlobalOpts {
	public:
		GlobalOpts();
		~GlobalOpts();
		
		bool HasDevice();
		BlueSCSIDevice & Device();
		void SetDevice(BlueSCSIDevice * deviceArg);
		
		bool IsVerbose();
		void SetVerbose(bool arg);
		
		bool ShouldRecurse();
		void SetRecurse(bool arg);
		
		bool ShouldForce();
		void SetForce(bool arg);
		
		FileLogger * Logger();
		void AddFileLogger(const char * path);
	
	private:
		BlueSCSIDevice * device;
		FileLogger * logger;
		bool verbose;
		bool recurse;
		bool force;
};

#endif