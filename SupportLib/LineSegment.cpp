/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela ShapeLib

Codaphela ShapeLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela ShapeLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela ShapeLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "LineSegment.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegment::Set(void)
{
	Float3Subtract(&msDirection, &msEnd, &msStart);
	mfLength = Float3Length(&msDirection);
	msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegment::Set(SFloat3* sStart, SFloat3* sEnd)
{
	msStart = (*sStart);
	msEnd = (*sEnd);
	Float3Subtract(&msDirection, &msEnd, &msStart);
	mfLength = Float3Length(&msDirection);
	msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegment::Set(float x1, float y1, float z1, float x2, float y2, float z2)
{
	msStart = SFloat3(x1, y1, z1);
	msEnd = SFloat3(x2, y2, z2);
	Float3Subtract(&msDirection, &msEnd, &msStart);
	mfLength = Float3Length(&msDirection);
	msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLineSegment::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(msStart.Save(pcFile));
	ReturnOnFalse(msDirection.Save(pcFile));
	ReturnOnFalse(pcFile->WriteFloat(mfLength));
	ReturnOnFalse(msEnd.Save(pcFile));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLineSegment::Load(CFileReader* pcFile)
{
	ReturnOnFalse(msStart.Load(pcFile));
	ReturnOnFalse(msDirection.Load(pcFile));
	ReturnOnFalse(pcFile->ReadFloat(&mfLength));
	ReturnOnFalse(msEnd.Load(pcFile));
	return TRUE;

}

