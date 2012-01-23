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
#include "ConcreteClass.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::Init(void)
{
	memset(this, 0, sizeof(CAbstractClass));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::Init(CClassStorage* pcClassStorage)
{
	macFields.Init(8);
	macMethods.Init(8);
	miSize = -1;
	miForcedSize = -1;
	miID = PT_Undefined;
	miFlags = SF_SIMPLE | SF_INCOMPLETE | SF_ZERO_INIT | SF_CLASS;
	maiSuperClasses.Init();
	maiSubClasses.Init();
	mcConcrete.Init(pcClassStorage);
	mszName.Init();
	miPrevVersionID = -1;
	miNextVersionID = -1;
	miVersionNumber = 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::Kill(void)
{
	int				i;
	CClassField*	pcField;
	CClassMethod*	psMethod;

	if (miFlags & SF_CLASS)
	{
		mcConcrete.Kill();
		maiSubClasses.Kill();
		maiSuperClasses.Kill();

		for (i = 0; i < macFields.NumElements(); i++)
		{
			pcField = macFields.Get(i);
			pcField->Kill();
		}
		macFields.Kill();

		for (i = 0; i < macMethods.NumElements(); i++)
		{
			psMethod = macMethods.Get(i);
			psMethod->Kill();
		}
		macMethods.Kill();

		mszName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::SetName(char* szName)
{
	mszName.Set(szName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::AddField(char* szType, char* szMemberName)
{
	int		iID;
	CChars	sz;

	iID = mcConcrete.mpcClassStorage->GetID(szType);
	if (iID == 0)
	{
		sz.Init("Could not find class[");
		sz.Append(szType);
		sz.Append("] whilst adding field[");
		sz.Append(mszName);
		sz.Append(".");
		sz.Append(szMemberName);
		sz.Append("]");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}
	else if (iID != -1)
	{
		return AddField(iID, szMemberName);
	}
	else
	{
		sz.Init("Could not find class[");
		sz.Append(szType);
		sz.Append("] whilst adding field[");
		sz.Append(mszName);
		sz.Append(".");
		sz.Append(szMemberName);
		sz.Append("]");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::AddField(int iType, char* szMemberName)
{
	CClassField*	pcField;
	CChars			sz;

	//If a field with this name already exists then bail.
	if (GetField(szMemberName) != NULL)
	{
		sz.Init("Could not add field ");
		sz.Append(mszName);
		sz.Append(".");
		sz.Append(szMemberName);
		sz.Append(".  It already exists.");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}

	pcField = macFields.Add();
	pcField->Init(-1, iType, szMemberName);

	if ((iType >= PT_VoidPointer) && (iType <= PT_SmartPointer))
	{
		miFlags &= ~SF_SIMPLE;
	}
	MakeDirty();
	return TRUE;
}

	
//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::RemoveField(char* szName)
{
	int				i;
	CClassField*	pcMember;

	for (i = 0; i < macFields.NumElements(); i++)
	{
		pcMember = macFields.Get(i);
		if (pcMember->szName.Equals(szName))
		{
			macFields.RemoveAt(i, TRUE);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CClassField* CAbstractClass::GetField(int iIndex)
{
	return macFields.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CClassField* CAbstractClass::GetField(char* szMemberName)
{
	int				i;
	CClassField*	pcField;

	for (i = 0; i < macFields.NumElements(); i++)
	{
		pcField = macFields.Get(i);
		if (pcField->szName.Equals(szMemberName))
		{
			return pcField;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CAbstractClass::NumFields(void)
{
	return macFields.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::AddMethod(int iArgumentType, int iReturnType, EMethodConvention eConvention, SFunctionPointer function, char* szName)
{
	CClassMethod*	psMethod;

	psMethod = macMethods.Add();
	psMethod->Init(iArgumentType, iReturnType, eConvention, function, szName);

	MakeDirty();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::AddMethod(int iGeneric, EMethodConvention eConvention, SFunctionPointer function)
{
	CClassMethod*	psMethod;

	psMethod = macMethods.Add();
	psMethod->Init(iGeneric, eConvention, function, mcConcrete.mpcClassStorage);

	MakeDirty();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::RemoveMethod(char* szName)
{
	int				i;
	CClassMethod*	pcMember;

	for (i = 0; i < macMethods.NumElements(); i++)
	{
		pcMember = macMethods.Get(i);
		if (pcMember->mszName.Equals(szName))
		{
			macMethods.RemoveAt(i, TRUE);
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CClassMethod* CAbstractClass::GetMethod(int iIndex)
{
	return macMethods.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CClassMethod* CAbstractClass::GetMethod(char* szMemberName)
{
	int				i;
	CClassMethod*	pcMember;

	for (i = 0; i < macMethods.NumElements(); i++)
	{
		pcMember = macMethods.Get(i);
		if (pcMember->mszName.Equals(szMemberName))
		{
			return pcMember;
		}
	}
	return NULL;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CAbstractClass::NumMethods(void)
{
	return macMethods.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::SetFlags(int iFlags)
{
	miFlags = iFlags;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::AddFlags(int iFlags)
{
	miFlags |= iFlags; 
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::RemoveFlags(int iFlags)
{
	miFlags &= ~iFlags;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::AddSuperClass(CAbstractClass* pcSuperClass)
{
	maiSuperClasses.Add(pcSuperClass->miID);
	pcSuperClass->maiSubClasses.Add(miID);
	MakeDirty();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::AddSuperClass(char* szName)
{
	CAbstractClass*	pcSuperClass;
	CChars	sz;

	pcSuperClass = mcConcrete.mpcClassStorage->Get(szName);
	if (pcSuperClass != NULL)
	{
		AddSuperClass(pcSuperClass);
		return TRUE;
	}
	else
	{
		sz.Init("Could not find class[");
		sz.Append(szName);
		sz.Append("] whilst setting the super class of class[");
		sz.Append(mszName);
		sz.Append("]");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::AddSuperClass(int iSuperClass)
{
	CAbstractClass*	pcSuperClass;

	pcSuperClass = mcConcrete.mpcClassStorage->Get(iSuperClass);
	if (pcSuperClass != NULL)
	{
		AddSuperClass(pcSuperClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass* CAbstractClass::GetSuperClass(int iIndex)
{
	int		iID;

	iID = maiSuperClasses.SafeGetValue(iIndex);
	if (iID == -1)
	{
		return NULL;
	}
	else
	{
		return mcConcrete.mpcClassStorage->Get(iID);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::GetSuperClasses(CArrayClassPtr* pacSuperClasses)
{
	mcConcrete.mpcClassStorage->Get(&maiSuperClasses, pacSuperClasses);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CAbstractClass::NumSuperClasses(void)
{
	return maiSuperClasses.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::AddSubClass(CAbstractClass* pcSubClass)
{
	maiSubClasses.Add(pcSubClass->miID);
	pcSubClass->maiSuperClasses.Add(miID);
	pcSubClass->MakeDirty();
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::AddSubClass(char* szName)
{
	CAbstractClass*	pcSubClass;
	CChars	sz;

	pcSubClass = mcConcrete.mpcClassStorage->Get(szName);
	if (pcSubClass != NULL)
	{
		AddSubClass(pcSubClass);
		return TRUE;
	}
	else
	{
		sz.Init("Could not find class[");
		sz.Append(szName);
		sz.Append("] whilst adding a sub-class of class[");
		sz.Append(mszName);
		sz.Append("]");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass* CAbstractClass::GetSubClass(int iIndex)
{
	int		iID;

	iID = maiSubClasses.SafeGetValue(iIndex);
	if (iID == -1)
	{
		return NULL;
	}
	else
	{
		return mcConcrete.mpcClassStorage->Get(iID);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::GetSubClasses(CArrayClassPtr* pacSubClasses)
{
	mcConcrete.mpcClassStorage->Get(&maiSubClasses, pacSubClasses);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CAbstractClass::NumSubClasses(void)
{
	return maiSubClasses.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CConcreteClass* CAbstractClass::ResolveInstanced(void)
{
	if (mcConcrete.Resolve(this))
	{
		return &mcConcrete;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CConcreteClass* CAbstractClass::GetInstanced(void)
{
	if (IsResolved())
	{
		return &mcConcrete;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::PrivateMakeDirty(void)
{
	if (miSize == -1)
	{
		return TRUE;
	}
	if (mcConcrete.MakeDirty())
	{
		miSize = -1;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CAbstractClass::MakeDirty(void)
{
	int			i;
	CAbstractClass*		pcClass;

	for (i = 0; i < maiSubClasses.NumElements(); i++)
	{
		pcClass = GetSubClass(i);
		pcClass->PrivateMakeDirty();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::IsDirty(void)
{
	return !IsResolved();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::IsResolved(void)
{
	int			i;
	CAbstractClass*		pcClass;

	if (miSize == -1)
	{
		return FALSE;
	}

	for (i = 0; i < maiSuperClasses.NumElements(); i++)
	{
		pcClass = GetSuperClass(i);
		if (!pcClass->IsResolved())
		{
			return FALSE;
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CAbstractClass::IsSuperResolved(void)
{
	int			i;
	CAbstractClass*		pcClass;

	for (i = 0; i < maiSuperClasses.NumElements(); i++)
	{
		pcClass = GetSuperClass(i);
		if (!pcClass->IsResolved())
		{
			return FALSE;
		}
	}

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CAbstractClass::ChunkName(void)
{
	return "CAbstractClass";
}

//
////////////////////////////////////////////////////////////////////////////
////																		//
////																		//
////////////////////////////////////////////////////////////////////////////
//BOOL CAbstractClass::LoadSpecific(CFileReader* pcFile, int iChunkNum, CClassStorage* pcClassStorage)
//{
//	int					i;
//	CClassField*		pcField;
//	CClassMethod*		pcMethod;
//
//	Init();
//	mcConcrete.Init(pcClassStorage);
//
//	ReturnOnFalse(pcFile->ReadChunkBegin(iChunkNum));
//	ReturnOnFalse(pcFile->ReadInt(&miID));
//	ReturnOnFalse(pcFile->ReadInt(&miSize));
//	ReturnOnFalse(pcFile->ReadInt(&miForcedSize));
//	ReturnOnFalse(pcFile->ReadInt(&miFlags));
//	ReturnOnFalse(pcFile->ReadInt(&miPrevVersionID));
//	ReturnOnFalse(pcFile->ReadInt(&miNextVersionID));
//	ReturnOnFalse(pcFile->ReadInt(&miVersionNumber));
//	ReturnOnFalse(pcFile->ReadString(&mszName));
//
//	ReturnOnFalse(pcFile->ReadArrayTemplateHeader(&macFields));
//	for (i = 0; i < macFields.NumElements(); i++)
//	{
//		pcField = macFields.Get(i);
//		pcField->Load(pcFile);
//	}
//
//	ReturnOnFalse(pcFile->ReadArrayTemplateHeader(&macMethods));
//	for (i = 0; i < macMethods.NumElements(); i++)
//	{
//		pcMethod = macMethods.Get(i);
//		pcMethod->Load(pcFile);
//	}
//
//	ReturnOnFalse(pcFile->ReadArrayInt(&maiSuperClasses));
//	ReturnOnFalse(pcFile->ReadArrayInt(&maiSubClasses));
//	return pcFile->ReadChunkEnd();
//}
//
//
////////////////////////////////////////////////////////////////////////////
////																		//
////																		//
////////////////////////////////////////////////////////////////////////////
//BOOL CAbstractClass::Save(CFileWriter* pcFile)
//{
//	int					i;
//	CClassField*		pcField;
//	CClassMethod*		pcMethod;
//
//	ReturnOnFalse(pcFile->WriteChunkBegin());
//	ReturnOnFalse(pcFile->WriteInt(miID));
//	ReturnOnFalse(pcFile->WriteInt(miSize));
//	ReturnOnFalse(pcFile->WriteInt(miForcedSize));
//	ReturnOnFalse(pcFile->WriteInt(miFlags));
//	ReturnOnFalse(pcFile->WriteInt(miPrevVersionID));
//	ReturnOnFalse(pcFile->WriteInt(miNextVersionID));
//	ReturnOnFalse(pcFile->WriteInt(miVersionNumber));
//	ReturnOnFalse(pcFile->WriteString(&mszName));
//
//	ReturnOnFalse(pcFile->WriteArrayTemplateHeader(&macFields));
//	for (i = 0; i < macFields.NumElements(); i++)
//	{
//		pcField = macFields.Get(i);
//		ReturnOnFalse(pcField->Save(pcFile));
//	}
//
//	ReturnOnFalse(pcFile->WriteArrayTemplateHeader(&macMethods));
//	for (i = 0; i < macMethods.NumElements(); i++)
//	{
//		pcMethod = macMethods.Get(i);
//		ReturnOnFalse(pcMethod->Save(pcFile));
//	}
//
//	ReturnOnFalse(pcFile->WriteArrayInt(&maiSuperClasses));
//	ReturnOnFalse(pcFile->WriteArrayInt(&maiSubClasses));
//	return pcFile->WriteChunkEnd(ChunkName());
//}

