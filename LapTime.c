#include "LapTime.h"
#include <sys/time.h>

static CFMutableArrayRef TIMERS = NULL;

OSErr startTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	OSErr err = noErr;
	CFIndex timer_index= -1;
	CFRange range;
	CFMutableArrayRef lap_times = CFArrayCreateMutable(NULL, 3, &kCFTypeArrayCallBacks);

	if (!TIMERS) {
		TIMERS = CFArrayCreateMutable(NULL, 3, &kCFTypeArrayCallBacks);
		CFArrayAppendValue(TIMERS, lap_times);
		timer_index = 0;
	} else {
		range = CFRangeMake(0, CFArrayGetCount(TIMERS));
		timer_index = CFArrayGetFirstIndexOfValue(TIMERS, range, kCFNull);
		if (timer_index < 0) {
			CFArrayAppendValue(TIMERS, lap_times);
			timer_index = range.length;
		} else {
			CFArraySetValueAtIndex(TIMERS, timer_index, lap_times);
		}
	}
	
	struct timeval tv;
	err = gettimeofday(&tv, NULL);
	if (err != noErr) {
		goto bail;
	}
	
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;
	CFNumberRef current_time = CFNumberCreate(NULL, kCFNumberDoubleType, &tm);
	CFArrayAppendValue(lap_times, current_time);
	AEDesc desc;
 	err = AECreateDesc(typeSInt32, &timer_index, sizeof(timer_index), &desc);
 	if (err != noErr) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);
bail:
	if (err != noErr) {
		CFArraySetValueAtIndex(TIMERS, timer_index, lap_times);
	}
	return err;
}

double lapTime(CFIndex timder_id, OSErr *err)
{
	double result = -1;
	if (!TIMERS) {
		*err = 1820;
		goto bail;
	}

	struct timeval tv;
	err = gettimeofday(&tv, NULL);
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;

	CFIndex timer_count = CFArrayGetCount(TIMERS);
	if (timer_id > timer_count) {
		*err = 1820;
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

OSErr lapTimeHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	OSErr err = noErr;
	
	CFIndex timer_id;
	err = AEGetParamPtr(ev, keyDirectObject, typeSInt32, NULL, &timer_id, sizeof(timer_id), NULL);
	if (err != noErr) goto bail;
	
	double dt = lapTime(timer_id, &err);
	if (err != noErr) {
		goto bail;
	}
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &dt, sizeof(dt), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);	
bail:
	return err;
}

OSErr stopTimerHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	OSErr err = noErr;
	CFIndex timer_index;
	err = AEGetParamPtr(ev, keyDirectObject, typeSInt32, NULL, &timer_index, sizeof(timer_index), NULL);
	if (err != noErr) goto bail;
	CFIndex timer_count = CFArrayGetCount(TIMERS);
	if (timer_index > timer_count) {
		goto bail;
	}
	
	CFArraySetValueAtIndex(TIMERS, timer_index, kCFNull);
	
bail:
	return err;
}
