#include <Carbon/Carbon.h>

OSErr startTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr lapTimeHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr stopTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr timeRecordsOfHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr getmsecHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
OSErr versionHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon);
