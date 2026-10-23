/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

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
#ifndef __ARRAY_OWNED_PTR_H__
#define __ARRAY_OWNED_PTR_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/PointerFunctions.h"


//An array of pointers that owns its elements.  Kill kills and frees every element.
template<class M>
class CArrayOwnedPtr : public CArrayTemplatePtr<M>
{
public:
	void	Kill(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayOwnedPtr<M>::Kill(void)
{
	size	i;
	size	uiNumElements;
	M*		pcElement;

	uiNumElements = CArrayTemplatePtr<M>::NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcElement = CArrayTemplatePtr<M>::GetPtr(i);
		SafeKill(pcElement);
	}

	CArrayTemplatePtr<M>::Kill();
}


#endif // __ARRAY_OWNED_PTR_H__

