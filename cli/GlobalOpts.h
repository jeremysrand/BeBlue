#ifndef CLI_CLOBAL_OPTS_H
#define CLI_GLOBAL_OPTS_H


// Forward declarations

class BlueSCSIDevice;


// Interface

class GlobalOpts {
	public:
		GlobalOpts();
		
		bool HasDevice();
		BlueSCSIDevice & Device();
		void SetDevice(BlueSCSIDevice * deviceArg);
		
		bool IsVerbose();
		void SetVerbose(bool arg);
	
	private:
		BlueSCSIDevice * device;
		bool verbose;
};

#endif