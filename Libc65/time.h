//Time.  There is no clock yet; these are only declared.
#ifndef __LIBC65_TIME_H__
#define __LIBC65_TIME_H__
#include <stddef.h>


#define CLOCKS_PER_SEC	1000


typedef long		time_t;
typedef long		clock_t;


struct tm
{
	int		tm_sec;
	int		tm_min;
	int		tm_hour;
	int		tm_mday;
	int		tm_mon;
	int		tm_year;
	int		tm_wday;
	int		tm_yday;
	int		tm_isdst;
};


extern "C"
{
time_t		time(time_t* ptTime);
clock_t		clock(void);
struct tm*	localtime(const time_t* ptTime);
struct tm*	gmtime(const time_t* ptTime);
time_t		mktime(struct tm* psTime);
double		difftime(time_t tEnd, time_t tStart);
size_t		strftime(char* szDest, size_t uiMax, const char* szFormat, const struct tm* psTime);
}


#endif // __LIBC65_TIME_H__
