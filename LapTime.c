#include "LapTime.h"
#include <sys/time.h>
#include "AEUtils.h"
#include "LapTimeConstants.h"

UInt32 gAdditionReferenceCount = 0;

static CFMutableArrayRef TIMERS = NULL;

CFIndex newTimer(OSErr *err)
{	
	CFIndex timer_id= -1;
	CFRange range;
	CFMutableArrayRef lap_times = CFArrayCreateMutable(NULL, 3, &kCFTypeArrayCallBacks);
	
	if (!TIMERS) {
		TIMERS = CFArrayCreateMutable(NULL, 3, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(TIMERS, lap_times);
		timer_id = 0;
	} else {
		range = CFRangeMake(0, CFArrayGetCount(TIMERS));
		timer_id = CFArrayGetFirstIndexOfValue(TIMERS, range, kCFNull);
		if (timer_id < 0) {
			CFArrayAppendValue(TIMERS, lap_times);
			timer_id = range.length;
		} else {
			CFArraySetValueAtIndex(TIMERS, timer_id, lap_times);
		}
	}
	
	struct timeval tv;
	*err = gettimeofday(&tv, NULL);
	if (*err != noErr) {
		CFArraySetValueAtIndex(TIMERS, timer_id, kCFNull);
		*err = kFailToGettimeofday;
		goto bail;
	}
	
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;
	CFNumberRef current_time = CFNumberCreate(NULL, kCFNumberDoubleType, &tm);
	CFArrayAppendValue(lap_times, current_time);
bail:
	return timer_id;
}

OSErr startTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	gAdditionReferenceCount++;
	OSErr err = noErr;
	AEDesc timer_spec;
	AECreateDesc(typeNull, NULL, 0, &timer_spec);
	//AEDesc id_desc;
	//AECreateDesc(typeNull, NULL, 0, &id_desc);
	CFIndex timer_id = newTimer(&err);
 	//err = AECreateDesc(typeSInt32, &timer_id, sizeof(timer_id), &id_desc);
	
	AEBuildError ae_err;
	err = AEBuildDesc(&timer_spec, &ae_err, "TmSp{ID  :long(@)}",timer_id);
 	if (err != noErr) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &timer_spec);
bail:
	AEDisposeDesc(&timer_spec);
	gAdditionReferenceCount--;
	return err;
}

double lapTime(CFIndex timer_id, OSErr *err)
{
	double result = -1;
	if (!TIMERS) {
		*err = kTimerIsInvalid;
		goto bail;
	}

	struct timeval tv;
	*err = gettimeofday(&tv, NULL);
	if (*err != noErr) {
		*err = kFailToGettimeofday;
		goto bail;
	}
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;

	CFIndex timer_count = CFArrayGetCount(TIMERS);
	if (timer_id > timer_count) {
		*err = kTimerIsInvalid;
		goto bail;
	}
	
	CFArrayRef lap_times = (CFArrayRef)CFArrayGetValueAtIndex(TIMERS, timer_id);
	CFNumberRef last_lap = (CFNumberRef)CFArrayGetValueAtIndex(lap_times, CFArrayGetCount(lap_times)-1);
	double tlast;
	CFNumberGetValue(last_lap, kCFNumberDoubleType, &tlast);
	result = tm - tlast;
	
bail:
	return result;
}

CFStringRef errorMessageWithError(OSErr err)
{
	CFStringRef result = NULL;
	switch (err) {
		case kTimerIsInvalid:
			result = CFSTR("The timer id is invalid.");	
			break;
		case kFailToGettimeofday:
			result = CFSTR("Failed to get gettimeofday.");	
			break;
		default:
			break;
	}
	return result;
}

OSErr lapTimeHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	gAdditionReferenceCount++;
	OSErr err = noErr;
	CFIndex timer_id;
	AERecord timer_spec;
	CFStringRef errmsg = NULL;
	
	err = AEGetParamDesc(ev, keyDirectObject, typeAERecord, &timer_spec);
	if (err != noErr) goto bail;	
	err = AEGetKeyPtr(&timer_spec, 'ID  ', typeSInt32, NULL, &timer_id, sizeof(timer_id), NULL);
	if (err != noErr) goto bail;
	
	double dt = lapTime(timer_id, &err);
	if (err != noErr) {
		errmsg = errorMessageWithError(err);
		if (!errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
		}
		goto bail;
	}
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &dt, sizeof(dt), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);	
bail:
	safeRelease(errmsg);
	gAdditionReferenceCount--;
	return err;
}

OSErr stopTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	gAdditionReferenceCount++;
	OSErr err = noErr;
	CFIndex timer_id;
	AERecord timer_spec;
	CFStringRef	errmsg = NULL;
	
	err = AEGetParamDesc(ev, keyDirectObject, typeAERecord, &timer_spec);
	if (err != noErr) goto bail;	
	err = AEGetKeyPtr(&timer_spec, 'ID  ', typeSInt32, NULL, &timer_id, sizeof(timer_id), NULL);
	if (err != noErr) goto bail;
	
	CFIndex timer_count = CFArrayGetCount(TIMERS);
	if (timer_id > timer_count) {
		err = kTimerIsInvalid;
		errmsg = errorMessageWithError(err);
		if (!errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
		}
		goto bail;
	}
	
	double dt = lapTime(timer_id, &err);
	if (err != noErr) {
		errmsg = errorMessageWithError(err);
		if (!errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
		}
		goto bail;
	}
	
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &dt, sizeof(dt), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);	
	
	CFArraySetValueAtIndex(TIMERS, timer_id, kCFNull);
	
bail:
	safeRelease(errmsg);
	gAdditionReferenceCount--;
	return err;
}

OSErr getmsecHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	gAdditionReferenceCount++;
	OSErr err = noErr;
	CFStringRef errmsg = NULL;
	
	struct timeval tv;
	err = gettimeofday(&tv, NULL);
	if (err != noErr) {
		err = kFailToGettimeofday;
		errmsg = errorMessageWithError(err);
		putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
		goto bail;
	}
	
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;	
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &tm, sizeof(tm), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);
bail:
	safeRelease(errmsg);
	gAdditionReferenceCount--;
	return err;
}

