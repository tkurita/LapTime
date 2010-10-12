#include "LapTime.h"
#include <sys/time.h>
#include <AssertMacros.h>
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
	fprintf(stderr, "start startTimerHandle\n");
	OSErr err = noErr;
	AEDesc timer_spec;
	AECreateDesc(typeNull, NULL, 0, &timer_spec);
	CFIndex timer_id = newTimer(&err);
	
	AEBuildError ae_err;
	err = AEBuildDesc(&timer_spec, &ae_err, "TmSp{ID  :long(@)}",timer_id);
 	if (err != noErr) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &timer_spec);
bail:
	AEDisposeDesc(&timer_spec);
	gAdditionReferenceCount--;
	return err;
}

OSErr lapTime(CFIndex timer_id, double *last_lap, double *total_time, CFArrayRef *time_records)
{
	OSErr err = noErr;
	CFNumberRef pretime = NULL;
	CFNumberRef firsttime = NULL;
	double tlast, tfirst;
	if (!TIMERS) {
		err = kTimerIsInvalid;
		goto bail;
	}
	
	struct timeval tv;
	err = gettimeofday(&tv, NULL);
	if (err != noErr) {
		err = kFailToGettimeofday;
		goto bail;
	}
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;
	
	CFIndex timer_count = CFArrayGetCount(TIMERS);
	if (timer_id > timer_count) {
		err = kTimerIsInvalid;
		goto bail;
	}
	
	CFMutableArrayRef trecord = (CFMutableArrayRef)CFArrayGetValueAtIndex(TIMERS, timer_id);
	if ((CFNullRef)trecord == kCFNull) {
		err = kTimerIsInvalid;
		goto bail;
	}
	CFIndex n_records = CFArrayGetCount(trecord);
	CFNumberRef current_time = CFNumberCreate(NULL, kCFNumberDoubleType, &tm);
	CFArrayAppendValue(trecord, current_time);	
	
	if (time_records) {
		*time_records = trecord;
	}
	
	if (last_lap) {
		pretime = (CFNumberRef)CFArrayGetValueAtIndex(trecord, n_records-1);
		CFNumberGetValue(pretime, kCFNumberDoubleType, &tlast);
		*last_lap = tm - tlast;
	}
	
	if (total_time) {
		firsttime = (CFNumberRef)CFArrayGetValueAtIndex(trecord, 0);
		CFNumberGetValue(firsttime, kCFNumberDoubleType, &tfirst);
		*total_time = tm - tfirst;
	}
	
bail:
	return err;
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
	
	//double dt = lapTime(timer_id, &err);
	double dt = 0;
	err = lapTime(timer_id, &dt, NULL, NULL);
	if (err != noErr) {
		goto bail;
	}
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &dt, sizeof(dt), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);	
bail:
	if (err != noErr) {
		errmsg = errorMessageWithError(err);
		if (errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
			CFRelease(errmsg);
		}
	}		
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
	
	if (!TIMERS) {
		err = kTimerIsInvalid;
		goto bail;
	}
	
	CFIndex timer_count = CFArrayGetCount(TIMERS);
	if (timer_id > timer_count) {
		err = kTimerIsInvalid;
		goto bail;
	}
	
	//double dt = lapTime(timer_id, &err);
	double dt = 0;
	err = lapTime(timer_id, NULL, &dt, NULL);
	require_noerr(err, bail);
	
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &dt, sizeof(dt), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);	
	
	CFArraySetValueAtIndex(TIMERS, timer_id, kCFNull);
	
bail:
	if (err != noErr) {
		errmsg = errorMessageWithError(err);
		if (errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
			CFRelease(errmsg);
		}
	}		
	gAdditionReferenceCount--;
	return err;
}

OSErr timeRecordsOfHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	gAdditionReferenceCount++;
	OSErr err = noErr;
	CFIndex timer_id;
	AERecord timer_spec;
	AEDescList lap_time_list;
	CFStringRef errmsg = NULL;
	
	err = AEGetParamDesc(ev, keyDirectObject, typeAERecord, &timer_spec);
	if (err != noErr) goto bail;	
	err = AEGetKeyPtr(&timer_spec, 'ID  ', typeSInt32, NULL, &timer_id, sizeof(timer_id), NULL);
	if (err != noErr) goto bail;
	
	Boolean with_stopping = true;
	err = getBoolValue(ev, kStoppingParam, &with_stopping);
	
	CFArrayRef time_records; 
	double total_time = 0;
	err = lapTime(timer_id, NULL, &total_time, &time_records);
	if (err != noErr) goto bail;	
	
	err = AECreateList(NULL, 0, false, &lap_time_list);
	if (err != noErr) goto bail;	
	
	CFNumberRef t = (CFNumberRef)CFArrayGetValueAtIndex(time_records, 0);
	double pt;
	CFNumberGetValue(t, kCFNumberDoubleType, &pt);
	for (int n = 1; n < CFArrayGetCount(time_records); n++) {
		t = (CFNumberRef)CFArrayGetValueAtIndex(time_records, n);
		double ct;
		CFNumberGetValue(t, kCFNumberDoubleType, &ct);
		double dt = ct-pt;
		err = AEPutPtr(&lap_time_list, n, typeIEEE64BitFloatingPoint, &dt, sizeof(dt));
		if (err != noErr) goto bail;
		pt = ct;
	};
	
	AERecord result_desc;
	AEBuildError ae_err;
	err = AEBuildDesc(&result_desc, &ae_err, "tTRc{LpTs:@, toTm:doub(@)}",&lap_time_list, total_time);
	err = AEPutParamDesc(reply, keyAEResult, &result_desc);
	AEDisposeDesc(&lap_time_list);
	AEDisposeDesc(&result_desc);
	
	if (with_stopping) {
		CFArraySetValueAtIndex(TIMERS, timer_id, kCFNull);
	}
	
bail:
	if (err != noErr) {
		errmsg = errorMessageWithError(err);
		if (errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
			CFRelease(errmsg);
		}
	}
	gAdditionReferenceCount--;	
	return err;
}

OSErr getmsecHandler(const AppleEvent *ev, AppleEvent *reply, SRefCon refcon)
{
	gAdditionReferenceCount++;
	puts("start getmsecHandler");
	OSErr err = noErr;
	CFStringRef errmsg = NULL;
	
	struct timeval tv;
	err = gettimeofday(&tv, NULL);
	if (err != noErr) {
		err = kFailToGettimeofday;
		errmsg = errorMessageWithError(err);
		if (errmsg) {
			putStringToEvent(reply, keyErrorString, errmsg, kCFStringEncodingUTF8);
			CFRelease(errmsg);
		}
		goto bail;
	}
	
	double tm = (double)tv.tv_sec*1e3 + (double)tv.tv_usec*1e-3;	
	AEDesc desc;
 	err = AECreateDesc(typeIEEE64BitFloatingPoint, &tm, sizeof(tm), &desc);
 	if (err) goto bail;
 	err = AEPutParamDesc(reply, keyAEResult, &desc);
 	AEDisposeDesc(&desc);
bail:
	gAdditionReferenceCount--;
	return err;
}

