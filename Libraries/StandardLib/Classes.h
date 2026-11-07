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
	template<class Class>
	void				AddConstructorAndIO(void);
	template<class Class>
	void				AddSystemClassAndConstructor(void);
	template<class Class>
	void				AddConstructor(void);

protected:
	uint32				GetNextClassType(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CClasses::AddConstructorAndIO(void)
{			
	const char*		szClassName;
	SDataIO*		psIO;
	bool			bConstructor;
	CStackMemory<>	cStack;
	Class*			pcClass;

	pcClass = StackConstruct<Class>(&cStack);
	szClassName = pcClass->ClassName();
	cStack.Kill();

	psIO = gcDataTypesIO.GetIO(szClassName);
	if (!psIO)
	{
		gcDataTypesIO.Add<Class>();
	}

	bConstructor = gcConstructors.Contains(szClassName);
	if (!bConstructor)
	{
		gcConstructors.Add<Class>();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CClasses::AddSystemClassAndConstructor(void)
{
	bool			bConstructor;
	Class*			pvM;
	CClass*			pcClass;
	CStackMemory<>	cStack;
	const char*		szClassName;

	pvM = StackConstruct<Class>(&cStack);
	szClassName = pvM->ClassName();
	pcClass = pvM->CBaseObject::CompleteClass(this);
	pcClass->System();

	bConstructor = gcConstructors.Contains(szClassName);
	if (!bConstructor)
	{
		gcConstructors.Add<Class>();
	}
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class Class>
void CClasses::AddConstructor(void)
{
	bool			bConstructor;
	Class*			pvM;
	CStackMemory<>	cStack;
	const char*		szClassName;

	pvM = StackConstruct<Class>(&cStack);
	szClassName = pvM->ClassName();

	bConstructor = gcConstructors.Contains(szClassName);
	if (!bConstructor)
	{
		gcConstructors.Add<Class>();
	}
	cStack.Kill();
}


#endif // __CLASSES_H__

