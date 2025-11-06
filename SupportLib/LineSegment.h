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
#ifndef __LINE_SEGMENT_H__
#define __LINE_SEGMENT_H__
#include "BaseLib/ChunkFile.h"
#include "BaseLib/GeometricTypes.h"


class CLineSegment
{
public:
	SFloat3		msStart;		//starting point of line
	SFloat3		msDirection;	//unit direction from start to msEnd
	float		mfLength;		
	SFloat3		msEnd;		//msEnd of line

	void Set(SFloat3* sStart, SFloat3* sEnd);
	void Set(float x1, float y1, float z1, float x2, float y2, float z2);
	void Set(void);
	void Copy(CLineSegment* pcSource);

	bool Save(CFileWriter* pcFile);
	bool Load(CFileReader* pcFile);
};


#endif //__LINE_SEGMENT_H__

