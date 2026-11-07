/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __OBJECT_POINTER_ARRAYS_H__
#define __OBJECT_POINTER_ARRAYS_H__
#include "BaseLib/ArrayTemplateEmbedded.h"
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "StackPointer.h"


class CEmbeddedObject;
typedef CArrayTemplatePtr<CEmbeddedObject>				CArrayEmbeddedObjectPtr;
typedef CArrayTemplateEmbeddedPtr<CEmbeddedObject, 8>	CArrayEmbeddedObjectPtrE8;

class CBaseObject;
typedef CArrayTemplatePtr<CBaseObject>					CArrayBaseObjectPtr;
typedef CArrayTemplateEmbeddedPtr<CBaseObject, 32>		CArrayBaseObjectPtrE32;


class CPointer;
typedef CArrayTemplateEmbedded<SStackPointer, 32>		CArrayStackPointerE32;


#endif // __OBJECT_POINTER_ARRAYS_H__

