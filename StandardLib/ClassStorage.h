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
#ifndef __TYPE_STORAGE_H__
#define __TYPE_STORAGE_H__
#include "AbstractClass.h"


typedef CArrayTemplate<CAbstractClass>		CArrayClass;
typedef CArrayTemplate<CClassMethodDesc>	CArrayMethodDesc;


class CClassStorage
{
private:
	void				PrivateInit(void);
	void				PrivateAddPrimitives(void);
	CAbstractClass*		PrivateAdd(int iForcedID);

public:
	CArrayClass			macClasses;
	int					miNextID;
	CArrayMethodDesc	masMethodDescs;
	BOOL				mbValid;

	//ClassIDs as added by 'this'
	int					miInstance_ClassID;
	int 				miSmartInstance_ClassID;

	void 				Init(void);
	void 				Kill(void);
	int					Add(int iForcedID, int iSize, int iFlags, char* szName, CAbstractClass** ppcClass);
	int					Add(char* szTypeName, CAbstractClass** ppcClass);
	int					AddSystem(CAbstractClass** ppcClass);
	BOOL				AddGenericMethod(int iGeneric, int iArgumentType, int iReturnType, char* szName);
	int					GetID(char* szTypeName);
	char*				GetName(int iID);
	CClassMethodDesc*	GetGenericMethod(int iGeneric);
	BOOL				PrivateResolve(int iID);
	BOOL				PrivateResolve(char* szName);
	BOOL				PrivateResolve(CAbstractClass* pcClass);
	BOOL				ResolveAll(void);
	BOOL				PrintUnresolvedClasses(CChars* szDest);
	CAbstractClass*		Get(int iID);
	CAbstractClass*		Get(char* szName);
	BOOL				ValidateType(int iID);
	BOOL				Remove(char* szStructName, BOOL bHasDependencies = TRUE);
	int					GetSize(int iID);
	void				Get(CArrayInt* paiIDs, CArrayClassPtr* papcClasses, BOOL bInitialiseArray = TRUE);
	void				Dump(void);

	//BOOL				Save(CFileWriter* pcFile);
	//BOOL				Load(CFileReader* pcFile);
	//BOOL				LoadSpecific(CFileReader* pcFile, int iChunkNum);
};


extern CClassStorage	gcClassStorage;


void ClassStorageInit(void);
void ClassStorageKill(void);


SInstancedClassMethod*	GetGenericMethod(int iClassID, int iGeneric);
void					LogGenericMethodError(int iClassID, int iGeneric);


#endif // __TYPE_STORAGE_H__

