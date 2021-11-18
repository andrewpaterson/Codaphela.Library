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
#ifndef __DATE_TIME_H__
#define __DATE_TIME_H__
#include "Date.h"
#include "Chars.h"


class CDateTime
{
CONSTRUCTABLE(CDateTime);
public:
	CDate		mcDate;
	int64		miNanosecondsInDay;

	void 	Init(void);  //Sets date/time to now.
	BOOL	Save(CFileWriter* pcFile);
	BOOL	Load(CFileReader* pcFile);
};


#endif // !__DATE_H__

