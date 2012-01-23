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
#include "CoreLib/TextParser.h"
#include "ClassStorage.h"


CClassStorage	gcClassStorage;


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ClassStorageInit(void)
{
	//The registrar entry point.
	gcClassStorage.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void ClassStorageKill(void)
{
	//The registrar destructor.
	gcClassStorage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassStorage::Init(void)
{
	BOOL	bResult;

	PrivateInit();
	PrivateAddPrimitives();

	miNextID = STRUCT_TYPES;

	bResult = ResolveAll();
	mbValid = bResult;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassStorage::PrivateInit(void)
{
	macClasses.Init(16);
	masMethodDescs.Init(16);

	miNextID = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassStorage::PrivateAddPrimitives(void)
{
	//Primitives
	Add(PT_int,		INT_BYTE_SIZE,		SF_PRIMITIVE | SF_BASIC,	"int",		NULL);
	Add(PT_short,	SHORT_BYTE_SIZE,	SF_PRIMITIVE | SF_BASIC,	"short",	NULL);
	Add(PT_char,	CHAR_BYTE_SIZE,		SF_PRIMITIVE | SF_BASIC,	"char",		NULL);
	Add(PT_float,	FLOAT_BYTE_SIZE,	SF_PRIMITIVE | SF_BASIC,	"float",	NULL);
	Add(PT_double,	DOUBLE_BYTE_SIZE,	SF_PRIMITIVE | SF_BASIC,	"double",	NULL);
	Add(PT_uint,	INT_BYTE_SIZE,		SF_PRIMITIVE | SF_BASIC,	"uint",		NULL);
	Add(PT_ushort,	SHORT_BYTE_SIZE,	SF_PRIMITIVE | SF_BASIC,	"ushort",	NULL);
	Add(PT_uchar,	CHAR_BYTE_SIZE,		SF_PRIMITIVE | SF_BASIC,	"uchar",	NULL);
	Add(PT_long,	LONG_BYTE_SIZE,		SF_PRIMITIVE | SF_BASIC,	"long",		NULL);
	Add(PT_ulong,	LONG_BYTE_SIZE,		SF_PRIMITIVE | SF_BASIC,	"ulong",	NULL);
	Add(PT_float2,	FLOAT2_BYTE_SIZE,	SF_PRIMITIVE,				"float2",	NULL);
	Add(PT_float3,	FLOAT3_BYTE_SIZE,	SF_PRIMITIVE,				"float3",	NULL);
	Add(PT_float4,	FLOAT4_BYTE_SIZE,	SF_PRIMITIVE,				"float4",	NULL);
	Add(PT_int2,	INT2_BYTE_SIZE,		SF_PRIMITIVE,				"int2",		NULL);
	Add(PT_int3,	INT3_BYTE_SIZE, 	SF_PRIMITIVE,				"int3",		NULL);
	Add(PT_int4,	INT4_BYTE_SIZE, 	SF_PRIMITIVE,				"int4",		NULL);
	Add(PT_bool,	BOOL_BYTE_SIZE,		SF_PRIMITIVE,				"bool",		NULL);
	Add(PT_void,	VOID_BYTE_SIZE,		SF_PRIMITIVE,				"void",		NULL);

	//Variable sized types.
	Add(PT_Block,		-1,		SF_PRIMITIVE | SF_INCOMPLETE,	"block",		NULL);	//Any size at all.
	Add(PT_Array,		-1,		SF_PRIMITIVE | SF_INCOMPLETE,	"array",		NULL);  //sizeof(CArrayBlock)
	Add(PT_List,		-1,		SF_PRIMITIVE | SF_INCOMPLETE,	"list",			NULL);  //sizeof(CLinkListTemplate)
	Add(PT_Tree,		-1,		SF_PRIMITIVE | SF_INCOMPLETE,	"tree",			NULL);  //sizeof(CTreeTemplate)
	Add(PT_Enumerator,	-1,		SF_PRIMITIVE | SF_INCOMPLETE,	"enumerator",	NULL);  //sizeof(CEnumeratorTemplate)
	Add(PT_Map,			-1,		SF_PRIMITIVE | SF_INCOMPLETE,	"map",			NULL);  //sizeof(CMapTemplate)

	//Special string.
	Add(PT_String, sizeof(CChars),	SF_PRIMITIVE,	"string",	NULL);  //this is a CChars

	//Pointers
	Add(PT_VoidPointer,		4,	SF_PRIMITIVE,	"VoidPointer",	NULL);  //Not sure which flags
	Add(PT_TypedPointer,	8,	SF_PRIMITIVE,	"TypedPointer", NULL);
	Add(PT_SmartPointer,	8,	SF_PRIMITIVE,	"SmartPointer",	NULL);  //Not sure which flags
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int AddWithFieldAndSuper(CClassStorage* pcClassStorage, char* szClassName, int iFieldClass)
{
	CAbstractClass*		pcClass;
	BOOL		bResult;
	int	iSmartClass;

	iSmartClass = pcClassStorage->miSmartInstance_ClassID;
	bResult = pcClassStorage->Add(szClassName, &pcClass);
	if (!bResult)
		return -1;

	bResult = pcClass->AddField(iFieldClass, "");  //Nameless field implies pass-thru/union.
	if (!bResult)
		return -1;

	pcClass->AddSuperClass(iSmartClass);
	return pcClass->miID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassStorage::Kill(void)
{
	int					i;
	CAbstractClass*				pcClass;
	CClassMethodDesc*	psMethodDesc;

	mbValid = FALSE;

	for (i = 0; i < masMethodDescs.NumElements(); i++)
	{
		psMethodDesc = masMethodDescs.Get(i);
		psMethodDesc->Kill();
	}
	masMethodDescs.Kill();

	for (i = 0; i < macClasses.NumElements(); i++)
	{
		pcClass = macClasses.Get(i);
		pcClass->Kill();
	}
	macClasses.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::ValidateType(int iType)
{
	CAbstractClass*	pcClass;

	//Only complete structures and primitives are allowed.
	if ((iType == PT_Array) ||
		(iType == PT_Block) ||
		(iType == PT_Enumerator) ||
		(iType == PT_List) ||
		(iType == PT_Tree) ||
		(iType == PT_VoidPointer) ||
		(iType == PT_CharPointer) ||
		(iType == PT_TypedPointer) ||
		(iType == PT_SmartPointer))
	{
		return FALSE;
	}

	//see if the struct type can be found.
	pcClass = Get(iType);
	if (pcClass)
	{
		return TRUE;
	}
	return FALSE;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass* CClassStorage::PrivateAdd(int iForcedID)
{	
	CAbstractClass*		pcClass;
	int			iNumToAdd;
	int			i;

	iNumToAdd = (iForcedID - macClasses.NumElements());
	if (iNumToAdd >= 0)
	{
		for (i = 0; i < iNumToAdd+1; i++)
		{
			pcClass = macClasses.Add();
			pcClass->Init();
		}
	}
	else
	{
		pcClass = macClasses.Get(iForcedID);
		if (pcClass->miFlags & SF_CLASS)
		{
			return NULL;
		}
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CClassStorage::Add(int iForcedID, int iSize, int iFlags, char* szName, CAbstractClass** ppcClass)
{
	CAbstractClass*		pcClass;

	if (iForcedID == -1)
	{
		iForcedID = miNextID;
		miNextID++;
	}

	pcClass = Get(szName);
	if (pcClass)
	{
		return PT_Undefined;
	}

	pcClass = PrivateAdd(iForcedID);
	if (!pcClass)
	{
		return PT_Undefined;
	}
	pcClass->Init(this);

	pcClass->miID = iForcedID;
	pcClass->miSize = iSize;
	pcClass->miFlags = iFlags | SF_CLASS;
	pcClass->SetName(szName);

	if (iSize != -1)
	{
		pcClass->ResolveInstanced();
	}

	if (ppcClass)
	{
		(*ppcClass) = pcClass;
	}
	return pcClass->miID;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CClassStorage::Add(char* szClassName, CAbstractClass** ppcClass)
{
	return Add(-1, -1, 0, szClassName, ppcClass);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CClassStorage::AddSystem(CAbstractClass** ppcClass)
{
	CChars	szName;
	BOOL	bResult;

	szName.Init("`SysGen");
	szName.Append(miNextID);
	bResult = Add(-1, -1, 0, szName.Text(), ppcClass);
	szName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::AddGenericMethod(int iGeneric, int iArgumentType, int iReturnType, char* szName)
{
	int 				i;
	int					iOldNum;
	CClassMethodDesc*	psStorageDesc;

	if (iGeneric < 0)
	{
		return FALSE;
	}

	if (iGeneric >= masMethodDescs.NumElements())
	{
		iOldNum = masMethodDescs.NumElements();
		masMethodDescs.GrowToNumElements(iGeneric+1);
		for (i = iOldNum; i < masMethodDescs.NumElements(); i++)
		{
			psStorageDesc = masMethodDescs.Get(i);
			psStorageDesc->miGeneric = -1;
		}
	}
	else
	{
		psStorageDesc = masMethodDescs.Get(iGeneric);
		if (psStorageDesc->miGeneric != -1)
		{
			//This generic method is already set.
			return FALSE;
		}
	}
	psStorageDesc->Init(iArgumentType, iReturnType, MC_Unknown, szName, iGeneric);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::PrivateResolve(int iStructType)
{
	CAbstractClass*		pcClass;

	pcClass = Get(iStructType);
	return PrivateResolve(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::PrivateResolve(char* szName)
{
	CAbstractClass*		pcClass;

	pcClass = Get(szName);
	return PrivateResolve(pcClass);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::PrivateResolve(CAbstractClass* pcClass)
{
	int							i;
	int							iNumMembers;
	CClassField*				psMember;
	int							iMemberOffset;
	int							iMemberSize;
	CConcreteClass*			pcInstanceClass;
	CChars						szError;

	if (!pcClass)
	{
		return FALSE;
	}

	//The size of the structure is -1 if not all of it's members have been resolved.
	if (pcClass->miSize != -1)
	{
		return TRUE;
	}

	if (!pcClass->IsSuperResolved())
	{
		return FALSE;
	}

	iMemberOffset = 0;
	iNumMembers = pcClass->macFields.NumElements();
	for (i = 0; i < iNumMembers; i++)
	{
		psMember = pcClass->GetField(i);
		psMember->iMemberOffset = iMemberOffset;
		iMemberSize = gcClassStorage.GetSize(psMember->iType);

		if (iMemberSize == -1)
		{
			return FALSE;
		}
		iMemberOffset += iMemberSize;
	}

	if (pcClass->miForcedSize == -1)
	{
		pcClass->miSize = iMemberOffset;
	}
	else
	{
		pcClass->miSize = pcClass->miForcedSize;
	}

	pcInstanceClass = pcClass->ResolveInstanced();
	if (pcInstanceClass)
	{
		return TRUE;
	}
	else
	{
		szError.Init("Could no Resolve Instanced class for [");
		szError.Append(pcClass->mszName);
		szError.Append("]");
		gcUserError.Set(szError.Text());
		szError.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::PrintUnresolvedClasses(CChars* szDest)
{
	int			i;
	CAbstractClass*		pcClass;
	BOOL		bAny;

	bAny = FALSE;
	for (i = STRUCT_TYPES; i < macClasses.NumElements(); i++)
	{
		pcClass = Get(i);
		if (!pcClass->IsResolved())
		{
			bAny = TRUE;
			szDest->Append(pcClass->mszName);
			szDest->Append(", ");
		}
	}
	if (bAny)
	{
		szDest->RemoveFromEnd(2);
	}
	return bAny;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::ResolveAll(void)
{
	int			i;
	CAbstractClass*		pcClass;
	BOOL		bResult;
	int			iLastNumResolved;
	int			iNumResolved;
	CChars		szError;

	if (macClasses.NumElements() < STRUCT_TYPES)
	{
		return TRUE;
	}

	iLastNumResolved = 0;
	for (;;)
	{
		iNumResolved = 0;
		for (i = STRUCT_TYPES; i < macClasses.NumElements(); i++)
		{
			pcClass = Get(i);
			if (pcClass->miFlags & SF_CLASS)
			{
				bResult = PrivateResolve(i);
				if (bResult)
				{
					iNumResolved++;
				}
				else
				{
					iNumResolved += 0;
				}
			}
		}
		if (iNumResolved == iLastNumResolved)
		{
			szError.Init("Unresolved Classes: ");
			PrintUnresolvedClasses(&szError);
			gcUserError.Set(szError.Text());
			szError.Kill();
			return FALSE;
		}
		else if (iNumResolved == macClasses.NumElements() - STRUCT_TYPES)
		{
			return TRUE;
		}
		iLastNumResolved = iNumResolved;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass* CClassStorage::Get(char* szName)
{
	int			i;
	CAbstractClass*		pcClass;

	for (i = 0; i < macClasses.NumElements(); i++)
	{
		pcClass = macClasses.Get(i);
		if (pcClass->miFlags & SF_CLASS)
		{
			if (pcClass->mszName.Equals(szName))
			{
				return pcClass;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass* CClassStorage::Get(int iID)
{
	CAbstractClass*	pcClass;

	pcClass = macClasses.SafeGet(iID);
	if ((pcClass) && (pcClass->miFlags & SF_CLASS))
	{
		return pcClass;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CClassStorage::GetID(char* szName)
{
	CAbstractClass*		pcClass;

	pcClass = Get(szName);
	if (pcClass)
	{
		return pcClass->miID;
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CClassStorage::GetName(int iID)
{
	CAbstractClass*		pcClass;

	pcClass = Get(iID);
	if (pcClass)
	{
		return pcClass->mszName.Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CClassMethodDesc* CClassStorage::GetGenericMethod(int iGeneric)
{
	CClassMethodDesc*	psDesc;

	psDesc = masMethodDescs.SafeGet(iGeneric);
	return psDesc;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassStorage::Remove(char* szStructName, BOOL bHasDependencies)
{
	//Not implemented yet.
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CClassStorage::GetSize(int iStructType)
{
	CAbstractClass*	pcClass;

	pcClass = Get(iStructType);
	if ((pcClass) && (pcClass->miFlags & SF_CLASS))
	{
		return pcClass->mcConcrete.miSize;
	}

	if (iStructType == PT_bit)
	{
		return 1 | BIT_SIZE;
	}
	else if (iStructType == PT_crumb)
	{
		return 2 | BIT_SIZE;
	}
	else if (iStructType == PT_nybble)
	{
		return 4 | BIT_SIZE;
	}
	else if (iStructType == PT_nickle)
	{
		return 5 | BIT_SIZE;
	}
	else if (iStructType == PT_sixbits)
	{
		return 6 | BIT_SIZE;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassStorage::Get(CArrayInt* paiIDs, CArrayClassPtr* papcClasses, BOOL bInitialiseArray)
{
	int			i;
	int			iID;
	CAbstractClass*		pcClass;

	if (bInitialiseArray)
	{
		papcClasses->Init(paiIDs->NumElements());
	}

	for (i = 0; i < paiIDs->NumElements(); i++)
	{
		iID = paiIDs->GetValue(i);
		pcClass = Get(iID);
		if (pcClass)
		{
			papcClasses->Add(&pcClass);
		}
	}
}


////////////////////////////////////////////////////////////////////////////
////																		//
////																		//
////////////////////////////////////////////////////////////////////////////
//BOOL CClassStorage::Save(CFileWriter* pcFile)
//{
//	int			i;
//	CAbstractClass*		pcClass;
//
//	ReturnOnFalse(pcFile->WriteChunkBegin());
//	for (i = 0; i < macClasses.NumElements(); i++)
//	{
//		pcClass = macClasses.Get(i);
//		if (pcClass->miFlags & SF_CLASS)
//		{
//			ReturnOnFalse(pcClass->Save(pcFile));
//		}
//	}
//	return pcFile->WriteChunkEnd("CClassStorage");
//}
//
//
////////////////////////////////////////////////////////////////////////////
////
////
////////////////////////////////////////////////////////////////////////////
//BOOL CClassStorage::Load(CFileReader* pcFile)
//{
//	int iChunkNum;
//
//	iChunkNum = pcFile->FindFirstChunkWithName("CClassStorage");
//	return LoadSpecific(pcFile, iChunkNum);
//}
//
//
////////////////////////////////////////////////////////////////////////////
////																		//
////																		//
////////////////////////////////////////////////////////////////////////////
//BOOL CClassStorage::LoadSpecific(CFileReader* pcFile, int iChunkNum)
//{	
//	CAbstractClass*		pcClass;
//	CAbstractClass		cTemp;
//
//	PrivateInit();
//
//	ReturnOnFalse(pcFile->ReadChunkBegin(iChunkNum));
//
//	iChunkNum = pcFile->FindFirstChunkWithName("CAbstractClass");
//	while (iChunkNum != -1)
//	{
//		ReturnOnFalse(cTemp.LoadSpecific(pcFile, iChunkNum, this));
//		pcClass = PrivateAdd(cTemp.miID);
//		memcpy(pcClass, &cTemp, sizeof(CAbstractClass));
//		iChunkNum = pcFile->FindNextChunkWithName();
//	}
//	mbValid = TRUE;
//	miNextID = macClasses.NumElements();
//	return pcFile->ReadChunkEnd();
//}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassStorage::Dump(void)
{
	CAbstractClass*		pcClass;
	int					iIndex;
	int					iMemberNum;
	CClassField*		psMember;
	CChars				sz;

	sz.Init();
	for (iIndex = 0; iIndex < macClasses.NumElements(); iIndex++)
	{
		pcClass = macClasses.Get(iIndex);

		if (pcClass->miFlags & SF_CLASS)
		{
			sz.AppendNewLine();
			sz.Append("---- [");
			sz.Append(pcClass->mszName.Text());
			sz.Append("] ----\n");

			for (iMemberNum = 0; iMemberNum < pcClass->macFields.NumElements(); iMemberNum++)
			{
				psMember = pcClass->macFields.Get(iMemberNum);
				sz.Append("  ");
				sz.Append(psMember->szName.Text());
				sz.AppendNewLine();
			}
		}
	}
	sz.Dump();
	sz.Kill();
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassMethod* GetGenericMethod(int iClassID, int iGeneric)
{
	CAbstractClass*					pcClass;
	CConcreteClass*		pcIClass;
	SInstancedClassMethod*	pcMethod;

	pcClass = gcClassStorage.Get(iClassID);
	if (pcClass)
	{
		pcIClass = pcClass->GetInstanced();
		if (pcIClass)
		{
			pcMethod = pcIClass->GetGenericMethod(iGeneric);
			return pcMethod;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void LogGenericMethodError(int iClassID, int iGeneric)
{
	CChars					sz;
	CAbstractClass*					pcClass;
	CConcreteClass*		pcIClass;
	SInstancedClassMethod*	pcMethod;
	CClassMethodDesc*		psDesc;

	sz.Init();

	pcClass = gcClassStorage.Get(iClassID);
	if (pcClass)
	{
		pcIClass = pcClass->GetInstanced();
		if (pcIClass)
		{
			pcMethod = pcIClass->GetGenericMethod(iGeneric);
			if (pcMethod)
			{
				sz.Append("The method was found, why are you logging an erro?");
			}
			else
			{
				psDesc = gcClassStorage.GetGenericMethod(iGeneric);
				if (psDesc)
				{
					pcMethod = pcIClass->GetMethod(psDesc->mszName.Text());
					if (pcMethod)
					{
						sz.Append("Method [");
						sz.Append(pcClass->mszName);
						sz.Append(".");
						sz.Append(psDesc->mszName);
						sz.Append("] is not generic.  Did you set .iGeneric?");
					}
					else
					{
						sz.Append("Could not find method [");
						sz.Append(pcClass->mszName);
						sz.Append(".");
						sz.Append(psDesc->mszName);
						sz.Append("].");
					}
				}
				else
				{
					sz.Append("Could no match the generic method with ID [");
					sz.Append(iGeneric);
					sz.Append("].  No such generic registered.");
				}
			}
		}
		else
		{
			sz.Append("Could not resolve instance class [");
			sz.Append(pcClass->mszName);
			sz.Append("].  Did you call ResolveAll?");
		}
	}
	else
	{
		sz.Append("Could not match a class with ID [");
		sz.Append(iClassID);
		sz.Append("].  No such class registered.");
	}
	gcUserError.Set(sz.Text());
	sz.Kill();
}

