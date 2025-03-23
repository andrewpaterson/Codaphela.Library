/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela InputLib

Codaphela InputLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela InputLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela InputLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "InputDevices.h"
#include "InputDeviceDesc.h"
#include "InputCategory.h"
#include "InputDeviceCopyContext.h"

#include "BaseLib/TextFile.h"

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::Init(char* szID, char* szFriendlyName, CInputCategory* pcCategory, bool bPhysical)
{
	mszID.Init(szID);
	mszFriendlyName.Init(szFriendlyName);
	mpcDataFormat = NULL;
	mlcInputs.Init();
	mlcVariables.Init();
	mpcCategory = pcCategory;
	mszComment.Init();
	macSwitches.Init();
	mapcDevices.Init();
	mbPhysical = bPhysical;
	mcVariableChordDescs.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::Kill(void)
{
	mcVariableChordDescs.Kill();
	mapcDevices.Kill();
	macSwitches.Kill();
	mszComment.Kill();
	mszFriendlyName.Kill();
	mszID.Kill();
	mpcCategory = NULL;
	mlcInputs.Kill();
	mlcVariables.Kill();

	mpcDataFormat = NULL;
	CUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDevices* CInputDeviceDesc::GetInputDevices(void)
{
	return mpcCategory->GetInputDevices();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceDesc::Is(char* szID)
{
	return mszID.Equals(szID);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceDesc::HasTag(char* szTagName)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDeviceDesc::AddStateInput(char* szFriendlyName)
{
	return AddInput(ISET_State, szFriendlyName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDeviceDesc::AddDeltaInput(char* szFriendlyName)
{
	return AddInput(ISET_Delta, szFriendlyName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDeviceDesc::AddInput(EInputSourceType eType, char* szFriendlyName)
{
	CInputSourceDesc*	pcSource;

	pcSource = mlcInputs.Add();
	pcSource->Init(this, eType, szFriendlyName, mlcInputs.NumElements()-1);
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::SetDataFormat(CInputDataFormat* pcDataFormat)
{
	mpcDataFormat = pcDataFormat;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::SetComment(char* szComment)
{
	mszComment.Set(szComment);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CInputDeviceDesc::RenameSource(char* szOldName, char* szNewName)
{
	CInputSourceDesc*	pcSource;
	SSetIterator		sIter;

	pcSource = mlcInputs.StartIteration(&sIter);
	while (pcSource)
	{
		if (pcSource->Is(szOldName))
		{
			pcSource->SetFriendlyName(szNewName);
			return true;
		}
		pcSource = mlcInputs.Iterate(&sIter);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableDesc* CInputDeviceDesc::GetVariable(char* szName)
{
	CInputDeviceVariableDesc*	pcVariableDesc;
	SSetIterator				sIter;

	pcVariableDesc = mlcVariables.StartIteration(&sIter);
	while (pcVariableDesc)
	{
		if (pcVariableDesc->Equals(szName))
		{
			return pcVariableDesc;
		}
		pcVariableDesc = mlcVariables.Iterate(&sIter);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableDesc* CInputDeviceDesc::AddVariable(char* szName)
{
	CInputDeviceVariableDesc*	pcVariableDesc;

	pcVariableDesc = mlcVariables.Add();
	pcVariableDesc->Init(szName, this);
	return pcVariableDesc;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDeviceDesc::GetSource(char* szFriendlyName)
{
	CInputSourceDesc*	pcSource;
	SSetIterator		sIter;

	pcSource = mlcInputs.StartIteration(&sIter);
	while (pcSource)
	{
		if (pcSource->Is(szFriendlyName))
		{
			return pcSource;
		}
		pcSource = mlcInputs.Iterate(&sIter);
	}
	return pcSource;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::GetSources(CArrayIntAndPointer* apcDest, CInputCategoryGeneric* pcGeneric)
{
	SSetIterator		sIter;
	CInputSourceDesc*	pcSourceDesc;

	pcSourceDesc = mlcInputs.StartIteration(&sIter);
	while (pcSourceDesc)
	{
		if (pcSourceDesc->GetGenerics()->Contains(&pcGeneric))
		{
			apcDest->AddIfUnique(pcSourceDesc, -1);
		}
		pcSourceDesc = mlcInputs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::GetSources(CArrayIntAndPointer* apcDest)
{
	SSetIterator		sIter;
	CInputSourceDesc*	pcSourceDesc;

	pcSourceDesc = mlcInputs.StartIteration(&sIter);
	while (pcSourceDesc)
	{
		apcDest->AddIfUnique(pcSourceDesc, -1);
		pcSourceDesc = mlcInputs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputCategory* CInputDeviceDesc::GetCategory(void)
{
	return mpcCategory;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceSwitch* CInputDeviceDesc::AddVariableSwitch(CInputDeviceVariableValue* pcVariableValue)
{
	//Er, you need the device as well I think.

	CInputVirtualDeviceSwitch*	pcInputVirtualDeviceSwitch;

	pcInputVirtualDeviceSwitch = macSwitches.Add();
	pcInputVirtualDeviceSwitch->Init(pcVariableValue);
	return pcInputVirtualDeviceSwitch;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::Copy(CInputDeviceDesc* pcSourceDesc)
{
	//This assumes Init has already been called.

	CInputDeviceCopyContext		cContext;

	if (pcSourceDesc->mpcCategory != mpcCategory)
	{
		return;
	}

	cContext.Init(pcSourceDesc, this);

	CopySources(&cContext);

	cContext.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::CopySources(CInputDeviceCopyContext* pcContext)
{
	CInputSourceDesc*	pcInputSourceDesc;
	CInputSourceDesc*	pcInputDestDesc;
	size				iNumInputs;
	SSetIterator		sIter;

	iNumInputs = mlcInputs.NumElements();
	pcInputSourceDesc = pcContext->mpcSourceDeviceDesc->mlcInputs.StartIteration(&sIter);
	while (pcInputSourceDesc)
	{
		pcInputDestDesc = mlcInputs.Add();
		pcInputDestDesc->Init(this, pcInputSourceDesc->GetType(), pcInputSourceDesc->GetFriendlyName(), pcInputSourceDesc->GetStateIndex() + iNumInputs);
		pcInputDestDesc->SetRest(pcInputSourceDesc->GetRestValue(), pcInputSourceDesc->GetEmitRestEvent(), pcInputSourceDesc->HasRestValue());
		pcInputDestDesc->CopyValues(pcInputSourceDesc);
		pcInputDestDesc->CopyActions(pcInputSourceDesc);
		pcContext->mmppSources.Put(pcInputSourceDesc, pcInputDestDesc);
		pcInputSourceDesc = pcContext->mpcSourceDeviceDesc->mlcInputs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::CopyVariables(CInputDeviceCopyContext* pcContext)
{
	CInputDeviceVariableDesc*	pcVariableSourceDesc;
	CInputDeviceVariableDesc*	pcVariableDestDesc;
	SSetIterator				sIter;

	pcVariableSourceDesc = pcContext->mpcSourceDeviceDesc->mlcVariables.StartIteration(&sIter);
	while (pcVariableSourceDesc)
	{
		pcVariableDestDesc = mlcVariables.Add();
		pcContext->mmppVariables.Put(pcVariableSourceDesc, pcVariableDestDesc);

		pcVariableDestDesc->Init(pcVariableSourceDesc->GetName(), this);
		pcVariableDestDesc->Copy(pcVariableSourceDesc, pcContext);
		pcVariableSourceDesc = pcContext->mpcSourceDeviceDesc->mlcVariables.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::AddDevice(CInputDevice* pcDevice)
{
	mapcDevices.Add(&pcDevice);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::RemoveDevice(CInputDevice* pcDevice)
{
	size		iIndex;

	iIndex = mapcDevices.Find(&pcDevice);
	mapcDevices.RemoveAt(iIndex, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CInputDeviceDesc::GetUnusedID(void)
{
	size			i;
	CInputDevice*	pcDevice;
	CArrayInt		aiIDs;
	size			iID;
	size			uiNumElements;

	uiNumElements = mapcDevices.NumElements();
	if (uiNumElements == 0)
	{
		return 0;
	}
	else if (uiNumElements == 1)
	{
		pcDevice = *mapcDevices.Get(0);
		if (pcDevice->GetDescriptionID() == 0)
		{
			return 1;
		}
		return 0;
	}

	aiIDs.Init();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDevice = *mapcDevices.Get(i);
		aiIDs.Add(pcDevice->GetDescriptionID());
	}
	aiIDs.QuickSort();
	iID = aiIDs.FindUnusedInSorted();
	aiIDs.Kill();
	return iID;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputVirtualDeviceDesc* CInputDeviceDesc::CreateDefaultVirtualDesc(void)
{
	CInputVirtualDeviceDesc*	pcVirtualDesc;
	SSetIterator				sIter;
	CInputSourceDesc*			pcSourceDesc;
	CInputDevices*				pcInputDevices;
	CChars						szTemp;

	pcInputDevices = GetInputDevices();

	szTemp.Init(mszFriendlyName.Text());
	szTemp.Append(" Default");
	pcVirtualDesc = pcInputDevices->CreateVirtualDeviceDescription(szTemp.Text(), true);
	szTemp.Kill();

	pcSourceDesc = mlcInputs.StartIteration(&sIter);
	while (pcSourceDesc)
	{
		pcVirtualDesc->AddSource(pcSourceDesc, -1);
		pcSourceDesc = mlcInputs.Iterate(&sIter);
	}
	return pcVirtualDesc;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::ToString(CChars* psz)
{
	SSetIterator				sIter;
	CInputSourceDesc*			pcInputSourceDesc;

	psz->Append(" --- CInputDeviceDesc ---\n");
	psz->Append("ID: ");
	psz->Append(mszID.Text());
	psz->AppendNewLine();
	psz->Append("Name: ");
	psz->Append(mszFriendlyName.Text());
	psz->AppendNewLine();
	psz->Append("Category: ");
	psz->Append(mpcCategory->GetCategoryName());
	psz->AppendNewLine();
	psz->Append("Comment: ");
	psz->Append(mszComment.Text());
	psz->AppendNewLine();
//	CInputDataFormat*					mpcDataFormat;

	psz->Append(" - CInputSourceDesc : mlcInputs -\n");
	pcInputSourceDesc = mlcInputs.StartIteration(&sIter);
	while (pcInputSourceDesc)
	{
		pcInputSourceDesc->ToString(psz);
		pcInputSourceDesc = mlcInputs.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableDesc* CInputDeviceDesc::StartVariablesIteration(SSetIterator* psIter)
{
	return mlcVariables.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputDeviceVariableDesc* CInputDeviceDesc::IterateVariables(SSetIterator* psIter)
{
	return mlcVariables.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDeviceDesc::StartInputsIteration(SSetIterator* psIter)
{
	return mlcInputs.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CInputSourceDesc* CInputDeviceDesc::IterateInputs(SSetIterator* psIter)
{
	return mlcInputs.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CInputDeviceDesc::NumInputs(void) { return mlcInputs.NumElements(); }
CInputDataFormat* CInputDeviceDesc::GetDataFormat(void) { return mpcDataFormat; }
bool CInputDeviceDesc::IsPhysical(void) { return mbPhysical; }
CInputChordDescs* CInputDeviceDesc::GetVariableChordDescs(void) { return &mcVariableChordDescs; }
char* CInputDeviceDesc::GetID(void) { return mszID.Text(); }
char* CInputDeviceDesc::GetFriendlyName(void) { return mszFriendlyName.Text(); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CInputDeviceDesc::Dump(void)
{
	CTextFile	cFile;
	CChars		szName;

	cFile.Init();
	ToString(&cFile.mcText);

	szName.Init("../");
	szName.Append(mszFriendlyName.Text());
	szName.Append(".txt");
	cFile.Write(szName.Text());
	szName.Kill();
	cFile.Kill();
}


