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
#ifndef __ENUMERATOR_POINTER_H__
#define __ENUMERATOR_POINTER_H__
#include "Define.h"
#include "DataTypes.h"
#include "ArrayInt.h"
#include "EnumeratorTemplate.h"


//Fix Enumerator unknown to use this as its super class... I think...
class CEnumeratorPointer : public __CEnumeratorTemplate<void>
{
public:
	int Add(char* szName, void* pvData, int iKeySize, int iNum, BOOL bReplace = TRUE);
};


#endif //__ENUMERATOR_POINTER_H__
