#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "FatTime.h"

// date/time macros
#define FAT_ENCODE_DATE(month, day, year)			((((uint16)((year) - 1980)) << 9) | ((uint16)(month) << 5) | (uint16)(day))
#define FAT_DECODE_DATE(date, month, day, year)		(year) = ((date) >> 9) + 1980);(month) = ((date & 0x1E0) >> 5);(day) = (date & 0x1F)
#define FAT_ENCODE_TIME(hour, minute, second)		(((uint16)(hour) << 11) | ((uint16)(minute) << 5) | ((uint16)(second) >> 1))
#define FAT_DECODE_TIME(time, hour, minute, second)	(hour) = ((time) >> 11); (minute) = (((time) & 0x7E0) >> 5); (secs) = (((time) & 0x1F) << 1)


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 GetSystemClockDate(void)
{
	time_t		now;
	tm*			timeinfo;

	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_DATE(timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 GetSystemClockTime(void)
{
	time_t		now;
	tm*			timeinfo;

	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_TIME(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
time_t FatDecodeDateTime(uint16 date, uint16 time)
{
	//YYYYYYYMMMMDDDDD
	//0000000000011111
	//0000000111100000
	//1111111000000000
	//HHHHHMMMMMMSSSSS
	//0000000000011111
	//0000011111100000
	//1111100000000000
	//struct tm datetime;
	//time_t now;

	struct tm tm;
	tm.tm_year = ((date) >> 9) + 80;
	tm.tm_mon = (((date) & 0x1E0) >> 5) - 1;
	tm.tm_mday = (date) & 0x1F;
	tm.tm_hour = (time) >> 11;
	tm.tm_min = ((time) & 0x7E0) >> 5;
	tm.tm_sec = ((time) & 0x1F) << 1;
	return mktime(&tm);
}


