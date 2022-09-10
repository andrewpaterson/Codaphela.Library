/*
 * fat32lib - Portable FAT12/16/32 Filesystem Library
 * Copyright (C) 2013 Fernando Rodriguez (frodriguez.developer@outlook.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Fat.h"
#include "FatInternals.h"
 

// TODO:
// 1. Optimize FatFileSeek
//
// 2. Update FatFileAllocate to allocated clusters based on the # of bytes
//		needed and not the clusters needed to allocate those bytes. Right now
//		it's overallocating under some circumstances. The extra clusters get
//		freed when the file is closed anyways but it'll still be more efficient
//		that way.


uint16 rtc_get_fat_date()
{
	time_t now;
	struct tm* timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_DATE(timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900);
}


uint16 rtc_get_fat_time()
{
	time_t now;
	struct tm* timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_TIME(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
}


time_t fat_decode_date_time(uint16 date, uint16 time) 
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

