/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	int8	mcMonth;
	int8	mcDay;

	void 	Init(void);  //Sets date to today.
	void 	Init(int16 sYear, int8 cMonth, int8 cDay);
	void 	Today(void);
	void 	Tomorrow(void);
	void 	Yesterday(void);
	void 	AddDays(int iDays);
	void 	SubtractDays(int iDays);
	void 	ToStringCCYYMMDD(CChars* psz);
	size	DaysInYear(void);
	size	YearsBetween(CDate* pcOther, EDateRounding eRounding);
	size	MonthsBetween(CDate* pcOther, EDateRounding eRounding);
	size	GetYear(void);
	size	GetMonth(void);
	size	GetDayInMonth(void);
	size	GetDayInYear(void);
	int		CompareTo(CDate* pcRight);
	bool	Equals(CDate* pcRight);
	bool	After(CDate* pcRight);
	bool	Before(CDate* pcRight);

	bool 	Save(CFileWriter* pcFile);
	bool 	Load(CFileReader* pcFile);
};


#endif // __DATE_H__

