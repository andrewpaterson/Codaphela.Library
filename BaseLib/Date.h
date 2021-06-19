/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __DATE_H__
#define __DATE_H__
#include "Chars.h"


enum EDateRounding
{
	DR_Truncate,
	DR_RoundDown,
};


class CDate
{
CONSTRUCTABLE(CDate);
public:
	int16	msYear;
	char	mcMonth;
	char	mcDay;

	void 	Init(void);  //Sets date to today.
	void 	Init(int16 sYear, char cMonth, char cDay);
	void 	Today(void);
	void 	Tomorrow(void);
	void 	Yesterday(void);
	void 	AddDays(int iDays);
	void 	SubtractDays(int iDays);
	void 	ToStringCCYYMMDD(CChars* psz);
	int		DaysInYear(void);
	int		YearsBetween(CDate* pcOther, EDateRounding eRounding);
	int		MonthsBetween(CDate* pcOther, EDateRounding eRounding);
	int		GetYear(void);
	int		GetMonth(void);
	int		GetDayInMonth(void);
	int		GetDayInYear(void);
	int		CompareTo(CDate* pcRight);
	BOOL	Equals(CDate* pcRight);
	BOOL	After(CDate* pcRight);
	BOOL	Before(CDate* pcRight);

	BOOL 	Save(CFileWriter* pcFile);
	BOOL 	Load(CFileReader* pcFile);
};


#endif // !__DATE_H__

