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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "Fat.h"
#include "FatInternals.h"
 

 // function pointer for rtc access routing
 #if !defined(FAT_USE_SYSTEM_TIME)
typedef struct TIMEKEEPER
{
	FAT_GET_SYSTEM_TIME fat_get_system_time;
}
TIMEKEEPER;
static TIMEKEEPER timekeeper;
#endif

// TODO:
// 1. Optimize FatFileSeek
//
// 2. Update FatFileAllocate to allocated clusters based on the # of bytes
//		needed and not the clusters needed to allocate those bytes. Right now
//		it's overallocating under some circumstances. The extra clusters get
//		freed when the file is closed anyways but it'll still be more efficient
//		that way.


// registers the function that gets the system time
#if !defined(FAT_USE_SYSTEM_TIME)
void fat_register_system_time_function(FAT_GET_SYSTEM_TIME system_time)
{
	timekeeper.fat_get_system_time = system_time;
}
#endif


/*
// converts the filename to it's Unicode UTF16 representation
*/
char get_long_name_for_entry(uint16* dst, uint8* src)
{
	register int i;
	for (i = 0; i < (int)strlen((char*)src); i++)
	{
		dst[i] = (uint16)src[i];
	}
	dst[i] = 0x0;
	/*
	// todo: check that this is a valid filename
	// and that it only uses ASCII chars since we don't
	// support unicode at this time
	*/
	return FAT_SUCCESS;
}


/*
// compares two short names after they
// have been formatted by get_short_name_for_entry
// returns 1 if both name are equal and 0 otherwise
*/
char fat_compare_short_name(uint8* name1, uint8* name2)
{
	return memcmp(name1, name2, 11) == 0;
}

/*
// performs an ASCII comparison on two UTF16 strings
*/
char fat_compare_long_name(uint16* name1, uint16* name2)
{
	register short i;
	for (i = 0; i < 256; i++)
	{
		if (toupper((char)name1[i]) != toupper((char)name2[i]))
			return 0;
		if ((char)name1[i] == 0x0)
			return 1;
	}
	return 1;
}


/*
// converts an 8.3 filename to the format required
// by the FAT directory entry structure
*/
uint16 get_short_name_for_entry(uint8* dest, uint8* src, char lfn_disabled)
{

	char tmp[13];
	char has_uppercase = 0;
	uint16 dot_index;
	uint16 length;
	uint16 i;

	/*
	// check that the name is actually a long filename
	// before processing it as such
	*/
	if (!lfn_disabled)
	{
		uint8 c;
		char is_lfn = 0;
		length = (uint16)strlen((char*)src);
		dot_index = indexof('.', (char*)src, 0x0);
		/*
		// if the file hs no extension and is longer than 8 chars
		// or if the name part has more than 8 chars or the extension more than 8
		// or if it has more than one dot then we need to handle it as a lfn
		*/
		if (dot_index < 0 && length > 8) is_lfn = 1;
		if (dot_index >= 0)
		{
			if (dot_index > 7 || (length - dot_index/*, 0*/) > 4) is_lfn = 1;
			if (dot_index >= 0) if (indexof('.', (char*)src, 1) >= 0) is_lfn = 1;
		}
		else
		{
			if (length > 8) is_lfn = 1;
		}
		/*
		// if it has spaces or lowercase letters we must also
		// handle it as a long filename
		*/
		if (!is_lfn)
		{
			for (i = 0; i < length; i++)
			{
				if (src[i] == 0x20 || src[i] != toupper(src[i]))
				{
					is_lfn = 1;
					break;
				}
			}
		}
		if (is_lfn)
		{
			/*
			// first we find the location of the LAST dot
			*/
			dot_index = length;
			for (i = length - 1; i; i--)
			{
				if (src[i] == '.')
				{
					dot_index = i;
					break;
				}
			}
			/*
			// now we copy the first 8 chars of the filename
			// excluding dots and spaces and we pad it with
			// spaces
			*/
			c = 0;
			for (i = 0; i < 8; i++)
			{
				while (c < dot_index)
				{
					if (src[c] == 0x20 || src[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < dot_index)
				{
					tmp[i] = toupper(src[c++]);
				}
				else
				{
					tmp[i] = 0x20;
				}
			}
			/*
			// do the same for the extension
			*/
			c = dot_index + 1;
			for (i = 8; i < 11; i++)
			{
				while (c < length)
				{
					if (src[c] == 0x20 || src[c] == '.')
					{
						c++;
					}
					else
					{
						break;
					}
				}
				if (c < length)
				{
					tmp[i] = toupper(src[c++]);
				}
				else
				{
					tmp[i] = 0x20;
				}
			}
			/*
			// now we copy it to the callers buffer and we're done
			*/
			for (i = 0; i < 11; i++)
				*dest++ = tmp[i];
			/*
			// return special code so the caller knows
			// to store the long name
			*/
			return FAT_LFN_GENERATED;
		}
	}

	/*
	// trim-off spaces - if the result is
	// greater than 12 it will return an empty
	// string
	*/
	strtrim(tmp, (char*)src, 12);
	/*
	// if the name length was invalid return
	// error code
	*/
	if (*tmp == 0 || strlen(tmp) > 12)
		return FAT_INVALID_FILENAME;
	/*
	// find the location of the dot
	*/
	dot_index = (uintptr_t)strchr(tmp, (int)'.');

	/*
	// strchr gave us the address of the dot, we now
	// convert it to a 1-based index
	*/
	if (dot_index)
		dot_index -= (uintptr_t)tmp - 0x1;
	/*
	// get the length of the input string
	*/
	length = (uint16)strlen(tmp);
	/*
	// check that this is a valid 8.3 filename
	*/
	if ((length > 0x9 &&
		(dot_index == 0x0 || (dot_index) > 0x9)) ||
		(dot_index > 0x0 && (length - dot_index) > 0x5))
		return FAT_INVALID_FILENAME;
	/*
	// copy the 1st part of the filename to the
	// destination bBuffer
	*/
	for (i = 0x0; i < 0x8; i++)
	{
		if (dot_index == 0x0)
		{
			if (i < length)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
		else
		{
			if (i < dot_index - 0x1)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;

				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}
	/*
	// if there's not extension fill the extension
	// characters with spaces
	*/
	if (dot_index == 0x0)
	{
		for (i = 0x0; i < 0x3; i++)
			*dest++ = 0x20;
	}
	/*
	// if there is an extension...
	*/
	else
	{
		/*
		// copy the extension characters to the
		// destination bBuffer
		*/
		for (i = dot_index; i < dot_index + 0x3; i++)
		{
			if (i < length)
			{
				if (lfn_disabled && (tmp[i] != toupper(tmp[i])))
					has_uppercase = 1;
				*dest++ = toupper(tmp[i]);
			}
			else
			{
				*dest++ = 0x20;
			}
		}
	}
	/*
	// return success code
	*/
	return has_uppercase ? FAT_INVALID_FILENAME : FAT_SUCCESS;
}


// computes the short filename checksum
uint8 fat_long_entry_checksum(uint8* filename)
{
	uint16 len;
	uint8 sum = 0;
	for (len = 11; len != 0; len--)
	{
		sum = ((sum & 1) ? 0x80 : 0) + (sum >> 1) + *filename++;
	}
	return sum;
}


int indexof(char chr, char* str, int index)
{
	int i = 0;
	str = str + index;

	do
	{
		if (str[i] == chr)
		{
			return i;
		}
		i++;
	} while (str[i]);

	return -1;
}


uint16 rtc_get_fat_date()
{
#if defined(FAT_USE_SYSTEM_TIME)
	time_t now;
	struct tm* timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_DATE(timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900);
#else
	if (timekeeper.fat_get_system_time)
	{
		time_t now = timekeeper.fat_get_system_time();
		struct tm* timeinfo = localtime(&now);
		return FAT_ENCODE_DATE(timeinfo->tm_mon + 1, timeinfo->tm_mday, timeinfo->tm_year + 1900);
	}
	else
	{
		return FAT_ENCODE_DATE(8, 2, 1985);
	}
#endif
}


uint16 rtc_get_fat_time()
{
#if defined(FAT_USE_SYSTEM_TIME)
	time_t now;
	struct tm* timeinfo;
	time(&now);
	timeinfo = localtime(&now);
	return FAT_ENCODE_TIME(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
#else
	if (timekeeper.fat_get_system_time)
	{
		time_t now;
		struct tm* timeinfo;
		now = timekeeper.fat_get_system_time();
		timeinfo = localtime(&now);
		return FAT_ENCODE_TIME(timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	}
	else
	{
		return FAT_ENCODE_TIME(12, 5, 35);
	}
#endif
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


// treams leading and trailing spaces. If the result
// exceeds the max length the destination will be set
// to an empty string
// todo: clean it up a bit
void strtrim(char* dest, char* src, size_t max) {

	uint32 max_length;
	uint32 lead_spaces = 0x0;
	uint32 last_char = 0x0;
	uint32 i;
	char* dst = dest;

	max_length = strlen(src);
	/*
	// count the lead spaces
	*/
	for (i = 0; i < max_length && src[i] == 0x20; i++)
		lead_spaces++;
	/*
	// if the whole string is full of spaces
	// return an empty string
	*/
	if (max_length == lead_spaces) {
		*dest = 0x0;
		return;
	}
	/*
	// calculate the index of the last non-space
	// character
	*/
	for (last_char = max_length - 1;
		last_char > 0 && (src[last_char] == 0x20); last_char--);

	/*
	// copy the non-space characters to the
	// destination uiBuffer
	*/
	for (i = lead_spaces; i <= last_char; i++)
	{
		*dest++ = src[i];
		if (!max--)
		{
			*dst = 0x0;
			return;
		}
	}
	/*
	// set the null terminator
	*/
	*dest = 0x0;
}


void FatParsePath(char* path, char* path_part, char** filename_part)
{
	*filename_part = path + strlen(path);

	while (*--(*filename_part) != '\\' && (*filename_part) != path);

	while (path != *filename_part)
	{
		*path_part++ = *path++;
	}
	*path_part = 0;
	(*filename_part)++;
}



