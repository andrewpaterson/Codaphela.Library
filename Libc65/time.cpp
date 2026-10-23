//Time for the 65C64, in C++ for the 65C64 generator.  There is no clock yet: time is 0 (1970-01-01) and
//processor time is unavailable.  Times are UTC; localtime is gmtime.  Only integer arithmetic is used.
#include <time.h>


struct tm	gsTime;


time_t time(time_t* ptTime)
{
	if (ptTime)
	{
		*ptTime = 0;
	}
	return 0;
}


clock_t clock(void)
{
	return (clock_t)-1;
}


static bool IsLeapYear(int iYear)
{
	return ((iYear % 4) == 0) && (((iYear % 100) != 0) || ((iYear % 400) == 0));
}


static int DaysInMonth(int iYear, int iMonth)
{
	static const int	aiDays[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	return ((iMonth == 1) && IsLeapYear(iYear)) ? 29 : aiDays[iMonth];
}


struct tm* gmtime(const time_t* ptTime)
{
	long	lDays;
	long	lSeconds;
	int		iYear;
	int		iMonth;
	int		iYearDays;

	lDays = *ptTime / 86400;
	lSeconds = *ptTime % 86400;
	if (lSeconds < 0)
	{
		lSeconds += 86400;
		lDays--;
	}
	gsTime.tm_sec = (int)(lSeconds % 60);
	gsTime.tm_min = (int)((lSeconds / 60) % 60);
	gsTime.tm_hour = (int)(lSeconds / 3600);

	//1970-01-01 was a Thursday.
	gsTime.tm_wday = (int)((lDays + 4) % 7);
	if (gsTime.tm_wday < 0)
	{
		gsTime.tm_wday += 7;
	}

	iYear = 1970;
	for (;;)
	{
		iYearDays = IsLeapYear(iYear) ? 366 : 365;
		if (lDays < 0)
		{
			iYear--;
			lDays += IsLeapYear(iYear) ? 366 : 365;
		}
		else if (lDays >= iYearDays)
		{
			lDays -= iYearDays;
			iYear++;
		}
		else
		{
			break;
		}
	}
	gsTime.tm_year = iYear - 1900;
	gsTime.tm_yday = (int)lDays;
	for (iMonth = 0; lDays >= DaysInMonth(iYear, iMonth); iMonth++)
	{
		lDays -= DaysInMonth(iYear, iMonth);
	}
	gsTime.tm_mon = iMonth;
	gsTime.tm_mday = (int)lDays + 1;
	gsTime.tm_isdst = 0;
	return &gsTime;
}


struct tm* localtime(const time_t* ptTime)
{
	return gmtime(ptTime);
}


//Fields outside their ranges are carried into the others, then all of *psTime is set from the time, so it is
//normalised and has tm_wday and tm_yday.
time_t mktime(struct tm* psTime)
{
	long	lDays;
	int		iYear;
	int		iMonth;
	int		iMonths;
	time_t	tTime;

	iYear = psTime->tm_year + 1900 + psTime->tm_mon / 12;
	iMonths = psTime->tm_mon % 12;
	if (iMonths < 0)
	{
		iMonths += 12;
		iYear--;
	}

	lDays = 0;
	if (iYear >= 1970)
	{
		for (int i = 1970; i < iYear; i++)
		{
			lDays += IsLeapYear(i) ? 366 : 365;
		}
	}
	else
	{
		for (int i = iYear; i < 1970; i++)
		{
			lDays -= IsLeapYear(i) ? 366 : 365;
		}
	}
	for (iMonth = 0; iMonth < iMonths; iMonth++)
	{
		lDays += DaysInMonth(iYear, iMonth);
	}
	lDays += psTime->tm_mday - 1;
	tTime = ((lDays * 24 + psTime->tm_hour) * 60 + psTime->tm_min) * 60 + psTime->tm_sec;
	*psTime = *gmtime(&tTime);
	return tTime;
}

