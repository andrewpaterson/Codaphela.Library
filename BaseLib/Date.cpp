/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Date.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDate::Init(int16 sYear, int8 cMonth, int8 cDay)
{
	msYear = sYear;
	mcMonth = cMonth;
	mcDay = cDay;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDate::Save(CFileWriter* pcFile)
{
	bool	bResult;

	bResult = pcFile->WriteInt16(msYear);
	bResult &= pcFile->WriteInt8(mcMonth);
	bResult &= pcFile->WriteInt8(mcDay);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDate::Load(CFileReader* pcFile)
{
	bool	bResult;

	bResult = pcFile->ReadInt16(&msYear);
	bResult &= pcFile->ReadInt8(&mcMonth);
	bResult &= pcFile->ReadInt8(&mcDay);
	return bResult;
}

