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
#include "ConcreteClass.h"
#include "AbstractClass.h"
#include "ClassStorage.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConcreteClass::Init(CClassStorage* pcClassStorage)
{
	maiClassOrder.Init(4);
	masClassDistance.Init(4);
	macInstances.Init(0);
	miSize = -1;
	macFields.Init(4);
	macMethods.Init(4);
	maiGenericMethods.Init(8);
	msziFields.Init(8, TRUE);
	msziMethods.Init(8, TRUE);
	mpcClassStorage = pcClassStorage;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConcreteClass::Kill(void)
{

	//This is incomplete... every instance must be killed first.
	macInstances.Kill();

	maiClassOrder.Kill();
	masClassDistance.Kill();
	miSize = -1;

	macFields.Kill();
	macMethods.Kill();
	maiGenericMethods.Kill();
	msziFields.Kill();
	msziMethods.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConcreteClass::PrintUnresolvedSuperClasses(CChars* szDest)
{
	int			iClassNum;
	CAbstractClass*		pcClass;
	BOOL		bAny;
	int			iID;

	bAny = FALSE;
	for (iClassNum = 0; iClassNum < maiClassOrder.NumElements(); iClassNum++)
	{
		iID = maiClassOrder.GetValue(iClassNum);
		pcClass = mpcClassStorage->Get(iID);

		if (pcClass->miSize == -1)
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
BOOL CConcreteClass::Resolve(CAbstractClass* pcClass)
{
	CChars		szError;

	maiClassOrder.ReInit();
	masClassDistance.ReInit();

	if (!RecurseResolve(pcClass, 0))
		return FALSE;

	if (!ResolveFields())
	{
		szError.Init("Cannot resolve class[");
		szError.Append(ClassName());
		szError.Append("].  It has unresolved Super-classes: ");
		PrintUnresolvedSuperClasses(&szError);
		gcUserError.Set(szError.Text());
		szError.Kill();
		return FALSE;
	}

	if (!ResolveMethods())
		return FALSE;

	macInstances.Init(miSize);  //Element size, not chunk size.
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConcreteClass::IsResolved(void)
{
	if (miSize != -1)
	{
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
BOOL CConcreteClass::MakeDirty(void)
{
	CChars	sz;

	if (miSize == -1)
	{
		return TRUE;
	}

	if (macInstances.NumElements() != 0)
	{
		sz.Init("Cannot make class[");
		sz.Append(ClassName());
		sz.Append("] dirty.  It has instances");
		gcUserError.Set(sz.Text());
		sz.Kill();
		return FALSE;
	}

	//Destroy everything, incidently setting miSize to -1.
	Kill();
	Init(mpcClassStorage);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CConcreteClass::ClassName(void)
{
	CAbstractClass*	pcClass;

	pcClass = GetSourceClass();
	return pcClass->mszName.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConcreteClass::RecurseResolve(CAbstractClass* pcClass, int iDistance)
{
	int					i;
	CAbstractClass*				pcSuperClass;
	int					iNumSuperClasses;
	int					iPos;
	SClassDistance*		psClassDistance;

	iNumSuperClasses = pcClass->maiSuperClasses.NumElements();
	for (i = 0; i < iNumSuperClasses; i++)
	{
		pcSuperClass = pcClass->GetSuperClass(i);
		RecurseResolve(pcSuperClass, iDistance+1);
	}

	iPos = maiClassOrder.AddIfUnique(pcClass->miID);
	if (iPos == -1)
	{
		psClassDistance = masClassDistance.Add();
		psClassDistance->iClass = pcClass->miID;
		psClassDistance->iDistance = iDistance;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConcreteClass::ResolveFields(void)
{
	int						iClassNum;
	int						iFieldNum;
	CAbstractClass*					pcClass;
	int						iID;
	SInstancedClassField*	psField;
	CClassField*			psSourceField;
	BOOL					bDuplicate;
	int						iOffset;
	int						iMapIndex;
	CChars*					pszKey;
	int*					piFieldIndex;
	int						iTotalFieldNum;

	for (iClassNum = 0; iClassNum < maiClassOrder.NumElements(); iClassNum++)
	{
		iID = maiClassOrder.GetValue(iClassNum);
		pcClass = mpcClassStorage->Get(iID);

		if (pcClass->miSize == -1)
		{
			miSize = -1;
			return FALSE;
		}
	}

	macFields.ReInit();

	iOffset = 0;
	iTotalFieldNum = 0;
	for (iClassNum = 0; iClassNum < maiClassOrder.NumElements(); iClassNum++)
	{
		iID = maiClassOrder.GetValue(iClassNum);
		pcClass = mpcClassStorage->Get(iID);

		for (iFieldNum = 0; iFieldNum < pcClass->NumFields(); iFieldNum++)
		{
			psSourceField = pcClass->GetField(iFieldNum);
			bDuplicate = (GetField(psSourceField->szName.Text()) != NULL);

			psField = macFields.Add();
			psField->iClassType = psSourceField->iType;
			psField->iSourceFieldIndex = iFieldNum;
			psField->iSourceClassType = pcClass->miID;
			psField->iFieldIndex = macFields.NumElements()-1;
			psField->iFlags = 0;
			psField->iMapIndex = -1;
			SetFlag(&psField->iFlags, INSTANCED_CLASS_FIELD_DUPLICATE_NAME, bDuplicate);
			psField->iOffset = psSourceField->iMemberOffset + iOffset;
			msziFields.PutAllowDuplicates(&psSourceField->szName, iTotalFieldNum);
			iTotalFieldNum++;
		}
		iOffset += pcClass->miSize;
	}

	for (iMapIndex = 0; iMapIndex < msziFields.NumElements(); iMapIndex++)
	{
		msziFields.GetAtIndex(iMapIndex, &pszKey, &piFieldIndex);

		psField = macFields.Get(*piFieldIndex);
		psField->iMapIndex = iMapIndex;
	}

	miSize = iOffset;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConcreteClass::ResolveMethods(void)
{
	int						iClassNum;
	CAbstractClass*					pcClass;
	int						iID;
	int						iMethodNum;
	CClassMethod*			psSourceMethod;
	SInstancedClassMethod*	psMethod;
	int						iMapIndex;
	int*					piMethodIndex;
	CChars*					pszKey;
	int						iGeneric;
	int						iOldNum;
	int						i;

	for (iClassNum = 0; iClassNum < maiClassOrder.NumElements(); iClassNum++)
	{
		iID = maiClassOrder.GetValue(iClassNum);
		pcClass = mpcClassStorage->Get(iID);

		for (iMethodNum = 0; iMethodNum < pcClass->NumMethods(); iMethodNum++)
		{
			psSourceMethod = pcClass->GetMethod(iMethodNum);

			psMethod = macMethods.Add();

			psMethod->iArgumentType = psSourceMethod->miArgumentType;
			psMethod->iReturnType = psSourceMethod->miReturnType;
			psMethod->eConvention = psSourceMethod->meConvention;
			psMethod->iGeneric =  psSourceMethod->miGeneric;

			psMethod->iMethodIndex = macMethods.NumElements()-1;
			psMethod->mpvFunction = psSourceMethod->mpvFunction;
			psMethod->iSourceMethodIndex = iMethodNum;
			psMethod->iSourceClassType = pcClass->miID;
			psMethod->iDistance = GetClassInheritanceDistance(iID);
			psMethod->iFlags = 0;
			psMethod->iMapIndex = -1;

			psMethod->miReturnSize = mpcClassStorage->GetSize(psMethod->iReturnType);  //Should error check here.
			psMethod->miArgumentSize = mpcClassStorage->GetSize(psMethod->iArgumentType);  //Should error check here.

			msziMethods.PutAllowDuplicates(&psSourceMethod->mszName, iMethodNum);
		}

		for (iMapIndex = 0; iMapIndex < msziMethods.NumElements(); iMapIndex++)
		{
			msziMethods.GetAtIndex(iMapIndex, &pszKey, &piMethodIndex);

			psMethod = macMethods.Get(*piMethodIndex);
			psMethod->iMapIndex = iMapIndex;

			//Add the generic index
			if (psMethod->iGeneric != -1)
			{
				iGeneric = psMethod->iGeneric;
				if (iGeneric >= maiGenericMethods.NumElements())
				{
					iOldNum = maiGenericMethods.NumElements();
					maiGenericMethods.GrowToNumElements(iGeneric+1);
					for (i = iOldNum; i < maiGenericMethods.NumElements(); i++)
					{
						maiGenericMethods.SetValue(i, -1);
					}
				}
				maiGenericMethods.SetValue(iGeneric, psMethod->iMethodIndex);
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CConcreteClass::GetClassInheritanceDistance(int iClassType)
{
	int					i;
	SClassDistance*		psClassDistance;

	for (i = 0; i < masClassDistance.NumElements(); i++)
	{
		psClassDistance = masClassDistance.Get(i);
		if (psClassDistance->iClass == iClassType)
		{
			return psClassDistance->iDistance;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass*	CConcreteClass::GetSourceClass(void)
{
	return (CAbstractClass*)this;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassField* CConcreteClass::GetField(int iIndex)
{
	return macFields.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassField* CConcreteClass::GetField(char* szMemberName)
{
	SInstancedClassField*	psField;
	int*					piIndex;
	CChars					sz;

	sz.Fake(szMemberName);
	piIndex = msziFields.GetWithKeyAssumeDuplicates(&sz);
	if (piIndex)
	{
		psField = macFields.Get(*piIndex);
		return psField;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassField* CConcreteClass::GetNextField(SInstancedClassField* psField)
{
	CChars*					psKey;
	int*					piFieldIndex;
	SInstancedClassField*	psNextField;

	if (psField)
	{
		if (msziFields.GetAtIndex(psField->iMapIndex, &psKey, &piFieldIndex))
		{
			if (msziFields.GetWithKeyNextDuplicate(psKey, psField->iMapIndex, &piFieldIndex))
			{
				psNextField = macFields.Get(*piFieldIndex);
				return psNextField;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int	CConcreteClass::GetFieldIndex(char* szMemberName)
{
	SInstancedClassField*	psField;

	psField = GetField(szMemberName);
	if (psField)
	{
		return psField->iFieldIndex;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CConcreteClass::GetFieldName(int iMemberID)
{
	SInstancedClassField*	psField;
	CChars*					pszKey;
	int*					piFieldIndex;

	psField = macFields.SafeGet(iMemberID);
	if (psField)
	{
		msziFields.GetAtIndex(psField->iMapIndex, &pszKey, &piFieldIndex);
		//iMemberIndex & *piFieldIndex must be the same.
		return pszKey->Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CConcreteClass::NumFields(void)
{
	return macFields.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassMethod* CConcreteClass::GetMethod(int iIndex)
{
	return macMethods.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassMethod* CConcreteClass::GetMethod(char* szMemberName)
{
	SInstancedClassMethod*	psMethod;
	int*					piIndex;
	CChars					sz;

	sz.Fake(szMemberName);
	piIndex = msziMethods.GetWithKeyAssumeDuplicates(&sz);
	if (piIndex)
	{
		psMethod = macMethods.Get(*piIndex);
		return psMethod;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassMethod* CConcreteClass::GetNextMethod(SInstancedClassMethod* psMethod)
{
	CChars*					psKey;
	int*					piMethodIndex;
	SInstancedClassMethod*	psNextMethod;

	if (psMethod)
	{
		if (msziMethods.GetAtIndex(psMethod->iMapIndex, &psKey, &piMethodIndex))
		{
			if (msziMethods.GetWithKeyNextDuplicate(psKey, psMethod->iMapIndex, &piMethodIndex))
			{
				psNextMethod = macMethods.Get(*piMethodIndex);
				return psNextMethod;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int	CConcreteClass::GetMethodIndex(char* szMemberName)
{
	SInstancedClassMethod*	psMethod;

	psMethod = GetMethod(szMemberName);
	if (psMethod)
	{
		return psMethod->iMethodIndex;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CConcreteClass::GetMethodName(int iMemberID)
{
	SInstancedClassMethod*	psMethod;
	CChars*					pszKey;
	int*					piMethodIndex;

	psMethod = macMethods.SafeGet(iMemberID);
	if (psMethod)
	{
		msziMethods.GetAtIndex(psMethod->iMapIndex, &pszKey, &piMethodIndex);
		//iMemberIndex & *piMethodIndex must be the same.
		return pszKey->Text();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInstancedClassMethod* CConcreteClass::GetGenericMethod(int iGeneric)
{
	int						iIndex;
	SInstancedClassMethod*	psMethod;

	if ((iGeneric >= 0) && (iGeneric < maiGenericMethods.NumElements()))
	{
		iIndex = maiGenericMethods.GetValue(iGeneric);
		if (iIndex != -1)
		{
			psMethod = macMethods.Get(iIndex);
			return psMethod;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CConcreteClass::NumMethods(void)
{
	return macMethods.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CConcreteClass::IsSmart(void)
{
	int		iFirstID;
	int		iSecondID;

	if (maiClassOrder.NumElements() >= 2)
	{
		iFirstID = maiClassOrder.GetValue(0);
		iSecondID = maiClassOrder.GetValue(1);
		if ((mpcClassStorage->miInstance_ClassID == iFirstID) && (mpcClassStorage->miSmartInstance_ClassID == iSecondID))
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CAbstractClass* CConcreteClass::GetClassOrder(int iClassOrder)
{
	int		iID;

	iID = maiClassOrder.SafeGetValue(iClassOrder);
	if (iID == -1)
	{
		return NULL;
	}
	else
	{
		return mpcClassStorage->Get(iID);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CConcreteClass::NumToPointers(void)
{
	int						i;
	SInstancedClassField*	psField;
	int						iCount;

	iCount = 0;
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.SafeGet(i);
		if (psField->iClassType == PT_SmartPointer)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CConcreteClass::NumEmbeddedObjects(void)
{
	int						i;
	SInstancedClassField*	psField;
	int						iCount;
	CAbstractClass*					pcClass;

	iCount = 0;
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.SafeGet(i);
		if (psField->iClassType >= STRUCT_TYPES)  // >= STRUCT_TYPES might be a problem.
		{
			pcClass = mpcClassStorage->Get(psField->iClassType);
			if (pcClass->mcConcrete.IsSmart())
			{
				iCount++;
			}
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CConcreteClass::Dump(void)
{
	SInstancedClassField*	psField;
	SInstancedClassMethod*	psMethod;
	int						i;
	CAbstractClass*			pcFieldClass;
	CAbstractClass*			pcArgumentClass;
	CAbstractClass*			pcReturnClass;
	char*					szName;
	CChars					sz;

	sz.Append(((CAbstractClass*)this)->mszName.Text());


	sz.Append("\n---- Fields ----\n");
	for (i = 0; i < macFields.NumElements(); i++)
	{
		psField = macFields.SafeGet(i);
		pcFieldClass = mpcClassStorage->Get(psField->iClassType);
		sz.Append(pcFieldClass->mszName.Text());
		sz.Append(" ");
		szName = GetFieldName(i);
		sz.Append(szName);
		sz.Append("\n");
	}

	sz.Append("\n---- Methods ----\n");
	for (i = 0; i < macMethods.NumElements(); i++)
	{
		psMethod = macMethods.SafeGet(i);
		pcReturnClass = mpcClassStorage->Get(psMethod->iReturnType);
		sz.Append(pcReturnClass->mszName.Text());
		sz.Append(" ");
		szName = GetMethodName(i);
		sz.Append(szName);
		sz.Append("(");
		pcArgumentClass = mpcClassStorage->Get(psMethod->iArgumentType);
		sz.Append(pcArgumentClass->mszName.Text());
		sz.Append(")\n");
	}
	sz.Append("\n");
	sz.Dump();
	sz.Kill();
}
