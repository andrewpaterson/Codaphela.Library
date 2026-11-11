#include <time.h>
#include "BaseLib/MersenneTwister.h"


//A bit for each check that passes.
int main()
{
	CMersenneTwister	cRandom;
	uint32				uiXor;
	uint32				uiValue;
	int					iPassed;
	int					i;
	int					iDie;
	bool				bRange;
	bool				bSeen[6];
	time_t				tTime;
	struct tm*			psTime;

	//The reference MT19937 sequence for the default seed.
	iPassed = 0;
	cRandom.Init(5489);
	uiXor = 0;
	for (i = 0; i < 1000; i++)
	{
		uiValue = cRandom.Next();
		if ((i == 0) && (uiValue == 3499211612u))
		{
			iPassed |= 1;
		}
		if ((i == 1) && (uiValue == 581869302u))
		{
			iPassed |= 2;
		}
		if ((i == 999) && (uiValue == 1341017984u))
		{
			iPassed |= 4;
		}
		uiXor ^= uiValue;
	}
	if (uiXor == 0xEEC731EBu)
	{
		iPassed |= 8;
	}

	//Dice rolls stay in range and every face comes up.
	bRange = true;
	for (i = 0; i < 6; i++)
	{
		bSeen[i] = false;
	}
	for (i = 0; i < 200; i++)
	{
		iDie = cRandom.Next(1, 6);
		bRange &= (iDie >= 1) && (iDie <= 6);
		if (bRange)
		{
			bSeen[iDie - 1] = true;
		}
	}
	if (bRange && bSeen[0] && bSeen[1] && bSeen[2] && bSeen[3] && bSeen[4] && bSeen[5])
	{
		iPassed |= 16;
	}
	cRandom.Kill();

	//Seeded from the clock, which the 65C64 does not have yet.
	cRandom.Init();
	cRandom.Next();
	cRandom.Kill();
	if ((time(NULL) == 0) && (clock() == (clock_t)-1))
	{
		iPassed |= 32;
	}

	//2001-09-09 01:46:40, a Sunday, and back.
	tTime = 1000000000;
	psTime = gmtime(&tTime);
	if ((psTime->tm_year == 101) && (psTime->tm_mon == 8) && (psTime->tm_mday == 9) && (psTime->tm_hour == 1) && (psTime->tm_min == 46) && (psTime->tm_sec == 40) && (psTime->tm_wday == 0) && (psTime->tm_yday == 251) && (mktime(psTime) == tTime))
	{
		iPassed |= 64;
	}
	tTime = -86400 * 365;
	psTime = gmtime(&tTime);
	if ((psTime->tm_year == 69) && (psTime->tm_mon == 0) && (psTime->tm_mday == 1) && (psTime->tm_wday == 3) && (mktime(psTime) == tTime))
	{
		iPassed |= 128;
	}
	return iPassed;
}

