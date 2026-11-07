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
#ifndef __CLASSES_H__
#define __CLASSES_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/MapIntTemplate.h"
#include "BaseLib/FreeList.h"
#include "UnmanagedClasses.h"
#include "PrimitiveClasses.h"
#include "Class.h"
#include "BaseLib/GlobalDataTypesIO.h"


typedef CMapStringTemplate<CClass*> CMapClassesByName;
typedef CMapIntTemplate<CClass*>	CMapClassesByType;


class CObjects;
class CClasses
{
protected:
	CMapClassesByName	mmcpClassesByName;
	CMapClassesByType	mmcpClassesByType;
	CFreeList			maClasses;

	CUnmanagedClasses	mcUnmanaged;
	CPrimitiveClasses	mcPrimitive;

	uint32				muiCurrentClassType;

	CClass*				mpcPointer;

public:
	void				Init(CObjects* pcObjects);
	void				Kill(void);
	void				AddSystemClasses(void);

	CClass*				Get(const char* szClassName);
	CClass*				Get(EPrimitiveType eType);
	CClass*				Get(uint32 iType);

	CClass*				GetValid(EPrimitiveType eType);

	CClass*				Add(const char* szClassName, size uiSize, EPrimitiveType eType);
	CClass*				Add(const char* szClassName, size uiSize);

	CClass*				GetPointer(void);

	CUnmanagedClasses*	GetUnmanagedClasses(void);
	CPrimitiveClasses*	GetPrimitiveClasses(void);

	void				ValidateType(EPrimitiveType eType);
	template<class M>
	void				AddConstructorAndIO(void);
	template<class M>
	void				AddSystemClassAndConstructor(void);
	template<class M>
	void				AddConstructor(void);

protected:
	uint32				GetNextClassType(void);
};


//////////////////////////////////////////////////////////////////////////
//
// M's data IO and constructor, each unless it is there already.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CClasses::AddConstructorAndIO(void)
{
	if (gcDataTypesIO.GetIO(M::StaticClassName()) == NULL)
	{
		gcDataTypesIO.Add<M>();
	}
	AddConstructor<M>();
}


//////////////////////////////////////////////////////////////////////////
//
// M's class, completed and marked a system class, and its constructor.
// The class is completed from an M constructed on the stack.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CClasses::AddSystemClassAndConstructor(void)
{
	M*				pcObject;
	CStackMemory<>	cStack;

	pcObject = StackConstruct<M>(&cStack);
	pcObject->CBaseObject::CompleteClass(this)->System();
	cStack.Kill();

	AddConstructor<M>();
}


//////////////////////////////////////////////////////////////////////////
//
// M's constructor, unless it is there already.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CClasses::AddConstructor(void)
{
	if (!gcConstructors.Contains(M::StaticClassName()))
	{
		gcConstructors.Add<M>();
	}
}


#endif // __CLASSES_H__

