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
#include "PreClass.h"
#include "ClassStorage.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassField::Init(int iMemberOffset, int iType, char* szMemberName)
{
	this->iMemberOffset = iMemberOffset;
	this->iType = iType;
	this->szName.Init(szMemberName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassField::Kill(void)
{
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassField::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt(&iType));
	ReturnOnFalse(pcFile->ReadInt(&iMemberOffset));
	ReturnOnFalse(pcFile->ReadString(&szName));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassField::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(iType));
	ReturnOnFalse(pcFile->WriteInt(iMemberOffset));
	ReturnOnFalse(pcFile->WriteString(&szName));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethod::Init(int iArgumentType, int iReturnType, EMethodConvention eConvention, SFunctionPointer function, char* szName)
{
	if (!CClassMethodDesc::Init(iArgumentType, iReturnType, eConvention, szName, -1))
	{
		mpvFunction = GetNULLCall;
		return FALSE;
	}
	mpvFunction = function;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethod::Init(int iGeneric, EMethodConvention eConvention, SFunctionPointer function, CClassStorage* pcClassStorage)
{
	CClassMethodDesc*	psDesc;

	psDesc = pcClassStorage->GetGenericMethod(iGeneric);
	if (psDesc)
	{
		CClassMethodDesc::Copy(psDesc);
		mpvFunction = function;
		meConvention = eConvention;
		return TRUE;
	}
	mpvFunction = GetNULLCall;
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassMethod::Kill(void)
{
	CClassMethodDesc::Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethodDesc::Init(int iArgumentType, int iReturnType, EMethodConvention eConvention, char* szName, int iGeneric)
{
	miReturnType = iReturnType;
	miArgumentType = iArgumentType;
	meConvention = eConvention;
	miGeneric = iGeneric;
	mszName.Init(szName);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassMethodDesc::Copy(CClassMethodDesc* psSource)
{
	miArgumentType = psSource->miArgumentType;
	miReturnType = psSource->miReturnType;
	meConvention = psSource->meConvention;
	mszName.Init(psSource->mszName);
	miGeneric = psSource->miGeneric;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CClassMethodDesc::Kill(void)
{
	mszName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethodDesc::Load(CFileReader* pcFile)
{
	ReturnOnFalse(pcFile->ReadInt(&miArgumentType));
	ReturnOnFalse(pcFile->ReadInt(&miReturnType));
	ReturnOnFalse(pcFile->ReadInt((int*)&meConvention));
	ReturnOnFalse(pcFile->ReadString(&mszName));
	ReturnOnFalse(pcFile->ReadInt(&miGeneric));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethodDesc::Save(CFileWriter* pcFile)
{
	ReturnOnFalse(pcFile->WriteInt(miArgumentType));
	ReturnOnFalse(pcFile->WriteInt(miReturnType));
	ReturnOnFalse(pcFile->WriteInt((int)meConvention));
	ReturnOnFalse(pcFile->WriteString(&mszName));
	ReturnOnFalse(pcFile->WriteInt(miGeneric));
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethod::Load(CFileReader* pcFile)
{
	mpvFunction.SetNull();
	return CClassMethodDesc::Load(pcFile);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CClassMethod::Save(CFileWriter* pcFile)
{
	return CClassMethodDesc::Save(pcFile);
}


