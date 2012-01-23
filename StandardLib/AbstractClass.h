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
#ifndef __TYPE_DESCRIPTOR_H__
#define __TYPE_DESCRIPTOR_H__
#include "BaseLib/ChunkFile.h"
#include "PreClass.h"
#include "ConcreteClass.h"


class CAbstractClass
{
public:
	CConcreteClass		mcConcrete;		//This is first so a class can be cast to a CConcreteClass.
										//Which is wrong, it should be a CConcreteClass*.

	int					miID;		//Numeric name uniquely identifying this class.
	int					miSize;		//The total size of the fields of this class.
	int					miForcedSize;  //The size is forced to this regardless of fields.
	int					miFlags; 	//Description of what can and can't be done with this class.  See SF_PRIMITIVE etc...
	CChars				mszName;
	SClassFieldArray	macFields;
	CClassMethodArray	macMethods;
	
	//Versioning.
	int					miPrevVersionID;
	int					miNextVersionID;
	int					miVersionNumber;

	//Inheritance
	CArrayInt			maiSuperClasses;
	CArrayInt			maiSubClasses;

	static char*		ChunkName(void);

	void				Init(void);
	void 				Init(CClassStorage* pcClassStorage);
	void 				Kill(void);
	void				SetName(char* szName);

	//Instancing
	CConcreteClass*	ResolveInstanced(void);
	CConcreteClass*	GetInstanced(void);

	//Members.
	BOOL 				AddField(int iType, char* szName);
	BOOL				AddField(char* szType, char* szName);
	BOOL				RemoveField(char* szName);
	CClassField* 		GetField(int iIndex);
	CClassField* 		GetField(char* szMemberName);
	int					GetFieldID(char* szMemberName);
	int					NumFields(void);
	void				MakeDirty(void);
	BOOL				PrivateMakeDirty(void);
	BOOL				IsDirty(void);
	BOOL				IsResolved(void);
	BOOL				IsSuperResolved(void);

	//Members
	BOOL				AddMethod(int iArgumentType, int iReturnType, EMethodConvention eConvention, SFunctionPointer function, char* szName);
	BOOL				AddMethod(int iGeneric, EMethodConvention eConvention, SFunctionPointer function);
	BOOL				RemoveMethod(char* szName);
	CClassMethod* 		GetMethod(int iIndex);
	CClassMethod* 		GetMethod(char* szMemberName);
	int					NumMethods(void);

	//Flags.
	void				SetFlags(int iFlags);
	void				AddFlags(int iFlags);
	void				RemoveFlags(int iFlags);
	int					GetFlags(void);

	//Inheritance
	void				AddSuperClass(CAbstractClass* pcSuperClass);
	BOOL				AddSuperClass(char* szName);
	void				AddSuperClass(int iSuperClass);
	CAbstractClass*		GetSuperClass(int iIndex);
	void				GetSuperClasses(CArrayClassPtr* pacSuperClasses);
	int					NumSuperClasses(void);
	BOOL				AddSubClass(char* szName);
	void				AddSubClass(CAbstractClass* pcSubClass);
	CAbstractClass*		GetSubClass(int iIndex);
	void				GetSubClasses(CArrayClassPtr* pacSubClasses);
	int					NumSubClasses(void);

	//BOOL				LoadSpecific(CFileReader* pcFile, int iChunkNum, CClassStorage* pcClassStorage);
	//BOOL				Save(CFileWriter* pcFile);
};


#endif // __TYPE_DESCRIPTOR_H__

