/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
	Float32Vec3Subtract(&msDirection, &msEnd, &msStart);
	mfLength = msDirection.Magnitude();
	msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegment::Set(SFloat32Vec3* sStart, SFloat32Vec3* sEnd)
{
	msStart = (*sStart);
	msEnd = (*sEnd);
	Float32Vec3Subtract(&msDirection, &msEnd, &msStart);
	mfLength = msDirection.Magnitude();
	msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegment::Set(float32 x1, float32 y1, float32 z1, float32 x2, float32 y2, float32 z2)
{
	msStart.Init(x1, y1, z1);
	msEnd.Init(x2, y2, z2);
	Float32Vec3Subtract(&msDirection, &msEnd, &msStart);
	mfLength = msDirection.Magnitude();
	msDirection /= mfLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLineSegment::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(msStart.Save(pcFile));
	ReturnOnFalse(msDirection.Save(pcFile));
	ReturnOnFalse(pcFile->WriteFloat(mfLength));
	ReturnOnFalse(msEnd.Save(pcFile));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLineSegment::Load(CFileReader* pcFile)
{
	ReturnOnFalse(msStart.Load(pcFile));
	ReturnOnFalse(msDirection.Load(pcFile));
	ReturnOnFalse(pcFile->ReadFloat(&mfLength));
	ReturnOnFalse(msEnd.Load(pcFile));
	return true;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLineSegment::Copy(CLineSegment* pcSource)
{
	msStart.Copy(&pcSource->msStart);
	msDirection.Copy(&pcSource->msDirection);
	mfLength = pcSource->mfLength;
	msEnd.Copy(&pcSource->msEnd);
}

