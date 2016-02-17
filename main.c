#include "LapTime.h"
#include "LapTimeConstants.h"

extern UInt32 gAdditionReferenceCount;
CFBundleRef		gAdditionBundle;

struct AEEventHandlerInfo {
	FourCharCode			evClass, evID;
	AEEventHandlerProcPtr	proc;
};

typedef struct AEEventHandlerInfo AEEventHandlerInfo;

static AEEventHandlerUPP *gEventUPPs;

static OSErr InstallMyEventHandlers();
static void RemoveMyEventHandlers();

const AEEventHandlerInfo gEventInfo[] = {
{ kLapTimeSuite, kStartTimerEvent, startTimerHandler },
{ kLapTimeSuite, kLapTimeEvent, lapTimeHandler},
{ kLapTimeSuite, kStopTimeEvent, stopTimerHandler},
{ kLapTimeSuite, kGetMsecEvent, getmsecHandler},
{ kLapTimeSuite, kTimeRecordsOfEvent, timeRecordsOfHandler},
{ kLapTimeSuite, kVersionEvent, versionHandler}
};

const int kEventHandlerCount = (sizeof(gEventInfo) / sizeof(AEEventHandlerInfo));

int countEventHandlers()
{
	return sizeof(gEventInfo)/sizeof(AEEventHandlerInfo);
}

OSErr SAInitialize(CFBundleRef theBundle)
{
#if useLog
	printf("start SAInitialize\n");
#endif	
	gAdditionBundle = theBundle;  // no retain needed.
	gEventUPPs = malloc(sizeof(AEEventHandlerUPP)*kEventHandlerCount);
	return InstallMyEventHandlers();
}

void SATerminate()
{
	RemoveMyEventHandlers();
	free(gEventUPPs);
}

Boolean SAIsBusy()
{
	return gAdditionReferenceCount != 0;
}

static OSErr InstallMyEventHandlers()
{
#if useLog
	printf("start InstallMyEventHandlers\n");
#endif
	OSErr		err;
	for (size_t i = 0; i < kEventHandlerCount; ++i) {
		if ((gEventUPPs[i] = NewAEEventHandlerUPP(gEventInfo[i].proc)) != NULL) {
#if useLog
			printf("before AEInstallEventHandler\n");
#endif
			err = AEInstallEventHandler(gEventInfo[i].evClass, gEventInfo[i].evID, gEventUPPs[i], 0, true);
		}
		else {
			err = memFullErr;
		}
		
		if (err != noErr) {
			SATerminate();  // Call the termination function ourselves, because the loader won't once we fail.
			return err;
		}
	}
	
	return noErr; 
}

static void RemoveMyEventHandlers()
{
	OSErr		err;
	size_t		i;
#if useLog	
	printf("start RemoveMyEventHandlers\n");
#endif	
	for (i = 0; i < kEventHandlerCount; ++i) {
		//printf("%i\n",i);
		if (gEventUPPs[i] &&
			(err = AERemoveEventHandler(gEventInfo[i].evClass, gEventInfo[i].evID, gEventUPPs[i], true)) == noErr)
		{
			DisposeAEEventHandlerUPP(gEventUPPs[i]);
			gEventUPPs[i] = NULL;
		}
	}
#if useLog
	printf("end RemoveMyEventHandlers\n");
#endif	
}

#if !__LP64__
int main(int argc, char *argv[]) // for debugging
{
	gEventUPPs = malloc(sizeof(AEEventHandlerUPP)*kEventHandlerCount);
	InstallMyEventHandlers();
    RunApplicationEventLoop();
	RemoveMyEventHandlers();
	free(gEventUPPs);
	return 0;
}
#endif