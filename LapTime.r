#include <Carbon/Carbon.r>

#define Reserved8   reserved, reserved, reserved, reserved, reserved, reserved, reserved, reserved
#define Reserved12  Reserved8, reserved, reserved, reserved, reserved
#define Reserved13  Reserved12, reserved
#define dp_none__   noParams, "", directParamOptional, singleItem, notEnumerated, Reserved13
#define reply_none__   noReply, "", replyOptional, singleItem, notEnumerated, Reserved13
#define synonym_verb__ reply_none__, dp_none__, { }
#define plural__    "", {"", kAESpecialClassProperties, cType, "", reserved, singleItem, notEnumerated, readOnly, Reserved8, noApostrophe, notFeminine, notMasculine, plural}, {}

resource 'aete' (0, "LapTime Terminology") {
	0x1,  // major version
	0x0,  // minor version
	english,
	roman,
	{
		"LapTime Suite",
		"Commands to measure script's execution time.",
		'LpTm',
		1,
		1,
		{
			/* Events */

			"start timer",
			"Make a new timer and start the timer",
			'LpTm', 'StTm',
			'TmSp',
			"A new timer specifer ",
			replyRequired, singleItem, notEnumerated, Reserved13,
			dp_none__,
			{

			},

			"lap time",
			"Record a lap time.",
			'LpTm', 'LpTm',
			'nmbr',
			"time in msec from previous lap time or start timer",
			replyRequired, singleItem, notEnumerated, Reserved13,
			'TmSp',
			"the timer created by start timer command",
			directParamRequired,
			singleItem, notEnumerated, Reserved13,
			{

			},

			"stop timer",
			"Stop a timer and obtain elapsed time from start timer command",
			'LpTm', 'SpTm',
			'nmbr',
			"milliseconds from start timer",
			replyRequired, singleItem, notEnumerated, Reserved13,
			'TmSp',
			"a timer created by start timer command",
			directParamRequired,
			singleItem, notEnumerated, Reserved13,
			{

			},

			"get msec",
			"Get milliseconds since Jan. 1, 1970",
			'LpTm', 'GtMs',
			'nmbr',
			"milliseconds",
			replyRequired, singleItem, notEnumerated, Reserved13,
			dp_none__,
			{

			},

			"time records of",
			"Obtain lap times and total time recoded in the timer, and stop the timer",
			'LpTm', 'TmRc',
			'****',
			"all time records recoded by lap time commands",
			replyRequired, singleItem, notEnumerated, Reserved13,
			'TmSp',
			"a timer created by start timer command",
			directParamRequired,
			singleItem, notEnumerated, Reserved13,
			{
				"stopping", 'sToP', 'bool',
				"stopping a timer. The default is true.",
				required,
				singleItem, notEnumerated, Reserved13
			}
		},
		{
			/* Classes */

			"timer specifier", 'TmSp',
			"A specifier of a timer",
			{
				"id", 'ID  ', 'long',
				"identifier of a timer",
				reserved, singleItem, notEnumerated, readWrite, Reserved12
			},
			{
			},

			"time records", 'tTRc',
			"A set of time records of a timer",
			{
				"lap times", 'LpTs', 'nmbr',
				"lap times",
				reserved, listOfItems, notEnumerated, readWrite, Reserved12,

				"total times", 'toTm', 'nmbr',
				"milliseconds since stat timer command.",
				reserved, singleItem, notEnumerated, readWrite, Reserved12
			},
			{
			}
		},
		{
			/* Comparisons */
		},
		{
			/* Enumerations */
		}
	}
};
