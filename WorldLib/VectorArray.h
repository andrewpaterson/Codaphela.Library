/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __VECTOR_ARRAY_H__
#define __VECTOR_ARRAY_H__
#include <d3dx9.h>
#include "BaseLib/PackedFloat.h"
#include "BaseLib/FreeList.h"
#include "BaseLib/GeometricTypes.h"


class SSkinnedVector
{
public:
	SFloat3*	apsSource[4];
	float		afWeights[4];
	SFloat3*	psCachedVector;

	void Init(void);
	void Kill(void);
	void Cache(void);
	void SetInput(int i, SFloat3* psSource, float fWeight);
	void SetOutput(SFloat3* psCached);
};


typedef CArrayTemplate<SFloat3>	CArrayVector;
typedef CArrayTemplate<SSkinnedVector>	CArraySkinnedVector;


#endif //__VECTOR_ARRAY_H__

