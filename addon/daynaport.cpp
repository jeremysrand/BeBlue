/*
	Copyright 1999, Be Incorporated.   All Rights Reserved.
	This file may be used under the terms of the Be Sample Code License.
*/

#include <stdio.h>

#include "generic.h"

static const char TITLE[] = "DaynaPort SCSI/Link";
static const char CONFIG[] = "daynaport";
static const char LINK[] = "/dev/net/dayna";


extern "C" _EXPORT BNetDevice *
open_device(const char *device)
{
	GenericDevice *dev;

#if 0
	// Debug code.
	FILE * f = fopen("/tmp/open_device.out", "a");
	fprintf(f, "Dayna: open_device(\"%s\") called\n", device);
	fflush(f);
	fclose(f);
#endif
	
	dev = new GenericDevice();
	if (dev->Start(device) < B_NO_ERROR) {
		delete dev;
		return (NULL);
	}
	return (dev);
}

extern "C" _EXPORT BNetConfig *
open_config(const char *device)
{
#if 0
	// More debug code
	FILE * f = fopen("/tmp/open_config.out", "a");
	fprintf(f, "Dayna: open_config(\"%s\") called\n", device);
	fflush(f);
	fclose(f);
#endif

	return (new GenericConfig(TITLE, CONFIG, LINK));
}

#if 0
// I used this to detect that the image got loaded or not by the network
// server.  Could turn this into a generic debug logging facility.
class LoadProbe {
  public:
    LoadProbe()
    {
      FILE * f = fopen("/tmp/dayna.log", "a");
      fprintf(f, "Dayna: image loaded\n");
      fflush(f);
      fclose(f);
    }
    
    ~LoadProbe()
    {
      FILE * f = fopen("/tmp/dayna.log", "a");
      fprintf(f, "Dayna: image unloaded\n");
      fflush(f);
      fclose(f);
    }
};

static LoadProbe loadProbe;
#endif