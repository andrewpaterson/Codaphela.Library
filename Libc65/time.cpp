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


//The days in the year before each month (February's 28).
static const int	gaiDaysBeforeMonth[13] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };


//a / b rounded down (b more than 0).
static int FloorDivide(int a, int b)
{
	return (a >= 0) ? a / b : -((b - 1 - a) / b);
}


//The days from 1970-01-01 to the first day of iYear (less than 0 before 1970): 365 a year and one for each leap year
//between, without a loop over them (1969 has 477 leap years before it, from year 1).
static long DaysBeforeYear(int iYear)
{
	int	iYears;

	iYears = iYear - 1;
	return ((long)iYear - 1970) * 365 + (FloorDivide(iYears, 4) - FloorDivide(iYears, 100) + FloorDivide(iYears, 400) - 477);
}


//The days in the year before the first of iMonth (0 to 11).
static int DaysBeforeMonth(int iYear, int iMonth)
{
	return gaiDaysBeforeMonth[iMonth] + (((iMonth > 1) && IsLeapYear(iYear)) ? 1 : 0);
}


struct tm* gmtime(const time_t* ptTime)
{
	long	lDays;
	long	lSeconds;
	int		iYear;
	int		iMonth;
	int		iYearDay;

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

	//365 days a year is near enough to start from; the leap days move it a year at most.
	iYear = (int)(1970 + ((lDays >= 0) ? lDays / 365 : -((364 - lDays) / 365)));
	while (DaysBeforeYear(iYear) > lDays)
	{
		iYear--;
	}
	while (DaysBeforeYear(iYear + 1) <= lDays)
	{
		iYear++;
	}
	iYearDay = (int)(lDays - DaysBeforeYear(iYear));
	gsTime.tm_year = iYear - 1900;
	gsTime.tm_yday = iYearDay;
	for (iMonth = 11; DaysBeforeMonth(iYear, iMonth) > iYearDay; iMonth--)
	{
	}
	gsTime.tm_mon = iMonth;
	gsTime.tm_mday = iYearDay - DaysBeforeMonth(iYear, iMonth) + 1;
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
	int		iMonths;
	time_t	tTime;

	iYear = psTime->tm_year + 1900 + psTime->tm_mon / 12;
	iMonths = psTime->tm_mon % 12;
	if (iMonths < 0)
	{
		iMonths += 12;
		iYear--;
	}

	lDays = DaysBeforeYear(iYear) + DaysBeforeMonth(iYear, iMonths) + psTime->tm_mday - 1;
	tTime = ((lDays * 24 + psTime->tm_hour) * 60 + psTime->tm_min) * 60 + psTime->tm_sec;
	*psTime = *gmtime(&tTime);
	return tTime;
}

