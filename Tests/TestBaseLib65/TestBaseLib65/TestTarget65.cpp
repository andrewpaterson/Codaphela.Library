#include <stdio.h>
#include <string.h>
#include "TestTarget65.h"


//////////////////////////////////////////////////////////////////////////
//
// The target the test's command line names into *peDest: "W65C816" (or
// "816" for short), or "65C64" or nothing for the 65C64.  False, having
// said what it takes, for anything else or a second argument: the tests
// aren't run on a target that wasn't asked for.
//
//////////////////////////////////////////////////////////////////////////
bool TestTarget65Choose(int argc, char** argv, ECPPTarget* peDest)
{
	if (argc <= 1)
	{
		*peDest = CPPT_65C64;
		return true;
	}
	if ((argc == 2) && ((strcmp(argv[1], "W65C816") == 0) || (strcmp(argv[1], "816") == 0)))
	{
		*peDest = CPPT_W65C816;
		return true;
	}
	if ((argc == 2) && (strcmp(argv[1], "65C64") == 0))
	{
		*peDest = CPPT_65C64;
		return true;
	}
	fprintf(stderr, "%s: unknown argument \"%s\".  Give W65C816 (or 816) for the W65C816, or 65C64 or nothing for the 65C64.\n", argv[0], (argc == 2) ? argv[1] : argv[2]);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The target's name, as its command line and its directory in Output
// have it: a test's 65C64 and W65C816 runs write their own files, so the
// two can run at once.
//
//////////////////////////////////////////////////////////////////////////
const char* TestTarget65Name(ECPPTarget eTarget)
{
	return (eTarget == CPPT_W65C816) ? "W65C816" : "65C64";
}
