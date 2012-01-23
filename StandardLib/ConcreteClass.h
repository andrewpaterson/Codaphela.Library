/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __CONCRETE_CLASS_H__
#define __CONCRETE_CLASS_H__
#include "PreClass.h"
#include "BaseLib/MapStringInt.h"
#include "BaseLib/ArraySimple.h"


#define INSTANCED_CLASS_FIELD_DUPLICATE_NAME	0x01


class SInstancedClassField
{
public:
	int		iClassType;  // == iType on source member.
	int		iFieldIndex;  //Index in field array.
	int		iOffset;	//Offset in bytes from the start of the class.
	int		iSourceFieldIndex;  //Index in CAbstractClass macFields.
	int		iSourceClassType;  //Actual type.
	int		iFlags;
	int		iMapIndex;  //Index into msziFields.
};


typedef CArrayTemplate<SInstancedClassField>			SInstancedClassFieldArray;


#define INSTANCED_CLASS_METHOD_AMBIGUOUS	0x01	//2 super classes share this exact method at the same iDistance.


class SInstancedClassMethod
{
public:
	int					iArgumentType;
	int					iReturnType;
	EMethodConvention	eConvention;  //Calling convention: stdcall, cdecl or thiscall.
	int					iGeneric;  //-1 if there is no generic type.

	int					iMethodIndex;  //Index in method array.
	SFunctionPointer	mpvFunction;
	int					iSourceMethodIndex; //Index in CAbstractClass macMethods
	int					iSourceClassType;  //Actual type.
	int					iDistance;  //How many classes (inheritance) down this method is found.
	int					iFlags;
	int					iMapIndex;  //Index into msziMethods.

	int					miArgumentSize;  //I'm not sure why?
	int					miReturnSize;  //I'm not sure why?
};


typedef CArrayTemplate<SInstancedClassMethod>	SInstancedClassMethodArray;


struct SClassDistance
{
	int	iClass;
	int iDistance;
};


typedef CArrayTemplate<SClassDistance>	SClassDistanceArray;


class CClassStorage;
class CConcreteClass
{
public:
	CClassStorage*				mpcClassStorage;
	CArrayInt					maiClassOrder;  //Array of classes ordered according to position in memory in the instance.
												//Index 0 is first in memory, miUsedElements-1 is the class containing this.
	SClassDistanceArray			masClassDistance;  //Array of classes with their respective inheritance distance.
	CFreeListBlock				macInstances;
	int							miSize;
	SInstancedClassFieldArray	macFields;
	SInstancedClassMethodArray	macMethods;
	CArrayInt					maiGenericMethods;  //The index in the array is the generic number.  
													//The value at that index is the SInstancedClassMethod index in macMethods
	CMapStringInt				msziFields;  //Map of field name to index in macFields.
											 //Remember to check the duplicate field flag.
	CMapStringInt				msziMethods;  //Map of method name to index in macMethods.
											  //Assume duplicates are present.

	void 					Init(CClassStorage* pcClassStorage);
	void 					Kill(void);

	BOOL					Resolve(CAbstractClass* pcClass);
	BOOL					IsResolved(void);
	BOOL					RecurseResolve(CAbstractClass* pcClass, int iDistance);
	BOOL					ResolveFields(void);
	BOOL					ResolveMethods(void);
	BOOL					PrintUnresolvedSuperClasses(CChars* szDest);
	CAbstractClass*			GetSourceClass(void);  //Seriously, this is stupid and dangerous.
	CAbstractClass*			GetClassOrder(int iClassOrder);
	BOOL					MakeDirty(void);

	SInstancedClassField*	GetField(int iID);
	SInstancedClassField*	GetField(char* szName);
	SInstancedClassField*	GetNextField(SInstancedClassField* psField);
	int						GetFieldIndex(char* szName);
	char*					GetFieldName(int iID);
	int						NumFields(void);

	SInstancedClassMethod*	GetMethod(int iID);
	SInstancedClassMethod*	GetMethod(char* szName);
	SInstancedClassMethod*	GetGenericMethod(int iGeneric);
	SInstancedClassMethod*	GetNextMethod(SInstancedClassMethod* psMethod);
	int						GetMethodIndex(char* szName);
	char*					GetMethodName(int iID);
	int						NumMethods(void);

	BOOL					IsSmart(void);
	int						NumToPointers(void);
	int						NumEmbeddedObjects(void);
	int						GetClassInheritanceDistance(int iClassType);
	void					Dump(void);
	char*					ClassName(void);
};


#endif // __CONCRETE_CLASS_H__

