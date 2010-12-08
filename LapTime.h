#include <Carbon/Carbon.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_5
typedef SInt32                          SRefCon;
#endif

OSErr startTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr lapTimeHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr stopTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr timeRecordsOfHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr getmsecHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr versionHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
