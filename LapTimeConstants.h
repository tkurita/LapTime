#define kTimerIsInvalid 1820
#define kFailToGettimeofday 1821

#define kLapTimeSuite    'LpTm'
#define kStartTimerEvent 'StTm'
#define kLapTimeEvent    'LpTm'
#define kStopTimeEvent   'SpTm'
#define kGetMsecEvent    'GtMs'
#define kTimeRecordsOfEvent 'TmRc'

enum {
	typeTimerSpecifier = 'TmSp',
	typeTimeRcords = 'tTRc'
};

enum {
	typeLapTims = 'LpTs',
	typeTotalTime = 'toTm'
};