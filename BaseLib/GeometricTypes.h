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
#ifndef __GEOMETRIC_TYPES_H__
#define __GEOMETRIC_TYPES_H__
#include "Float2.h"
#include "Float3.h"
#include "Float4.h"
#include "Float4x4.h"
#include "Double2.h"
#include "Double3.h"
#include "Double4.h"
#include "Double4x4.h"
#include "Quaternion.h"


//Hack to make this compile.  These used to be part of StandardTypes.
SFloat3*	GetPosition(SFloat3* psPoints, int iStride, int iPoint);
SFloat3*	GetNormal(SFloat3* psNormals, int iStride, int iNormal);
int			GetIndex(SFloat3* psPoints, int iStride, SFloat3* psPosition);


#endif // __GEOMETRIC_TYPES_H__

