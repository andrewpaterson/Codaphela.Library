/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "IRModule.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRGlobal::Init(uint32 uiSymbol, uint32 uiAlign, uint32 uiAttributes)
{
	muiSymbol = uiSymbol;
	muiAlign = uiAlign;
	muiAttributes = uiAttributes;
	masItems.Init();
	macBytes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRGlobal::Kill(void)
{
	macBytes.Kill();
	masItems.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRDataItem* CIRGlobal::AddItem(EIRDataKind eKind)
{
	SIRDataItem*	psItem;

	psItem = masItems.Add();
	memset(psItem, 0, sizeof(SIRDataItem));
	psItem->uiKind = (uint8)eKind;
	return psItem;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRGlobal::AddValue(SIRType sType, uint64 ulliBits)
{
	SIRDataItem*	psItem;

	psItem = AddItem(IRDK_Value);
	psItem->sType = sType;
	psItem->ulliValue = IRTruncate(sType, ulliBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRGlobal::AddSymbol(SIRType sType, uint32 uiSymbol, int64 lliOffset)
{
	SIRDataItem*	psItem;

	psItem = AddItem(IRDK_Symbol);
	psItem->sType = sType;
	psItem->ulliValue = uiSymbol;
	psItem->lliOffset = lliOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRGlobal::AddBytes(const uint8* pucBytes, size uiLength)
{
	SIRDataItem*	psItem;

	psItem = AddItem(IRDK_Bytes);
	psItem->uiStart = macBytes.NumElements();
	psItem->uiLength = uiLength;
	if (uiLength > 0)
	{
		macBytes.InsertBlockAfterEnd((uint8*)pucBytes, uiLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRGlobal::AddZero(size uiBytes)
{
	AddItem(IRDK_Zero)->ulliValue = uiBytes;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRGlobal::Size(CIRModule* pcModule)
{
	size			ui;
	size			uiSize;
	SIRDataItem*	psItem;

	uiSize = 0;
	for (ui = 0; ui < masItems.NumElements(); ui++)
	{
		psItem = masItems.Get(ui);
		switch (psItem->uiKind)
		{
		case IRDK_Value:
		case IRDK_Symbol:
			uiSize += pcModule->GetSize(psItem->sType);
			break;
		case IRDK_Bytes:
			uiSize += psItem->uiLength;
			break;
		case IRDK_Zero:
			uiSize += (size)psItem->ulliValue;
			break;
		}
	}
	return uiSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::Init(void)
{
	maszSpaces.Init();
	maucSpaceSizes.Init();
	maszSymbols.Init();
	mmSymbols.Init();
	masDefinitions.Init();
	mapcFunctions.Init();
	mapcGlobals.Init();
	mapcKnown.Init();
	mpasTargetOpcodes = NULL;
	muiNumTargetOpcodes = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::Kill(void)
{
	size			ui;
	CIRFunction*	pcFunction;
	CIRGlobal*		pcGlobal;

	for (ui = 0; ui < mapcKnown.NumElements(); ui++)
	{
		pcGlobal = mapcKnown.GetPtr(ui);
		SafeKill(pcGlobal);
	}
	mapcKnown.Kill();
	for (ui = 0; ui < mapcGlobals.NumElements(); ui++)
	{
		pcGlobal = mapcGlobals.GetPtr(ui);
		SafeKill(pcGlobal);
	}
	mapcGlobals.Kill();
	for (ui = 0; ui < mapcFunctions.NumElements(); ui++)
	{
		pcFunction = mapcFunctions.GetPtr(ui);
		SafeKill(pcFunction);
	}
	mapcFunctions.Kill();
	masDefinitions.Kill();
	mmSymbols.Kill();
	maszSymbols.Kill();
	maucSpaceSizes.Kill();
	maszSpaces.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The first space added is the default (ptr).
//
//////////////////////////////////////////////////////////////////////////
uint8 CIRModule::AddSpace(const char* szName, int iPointerBytes)
{
	uint8	uiSize;

	uiSize = (uint8)iPointerBytes;
	maszSpaces.Add(szName);
	maucSpaceSizes.Add(&uiSize);
	return (uint8)(maszSpaces.NumElements() - 1);
}


//////////////////////////////////////////////////////////////////////////
//
// -1 if there is no space called szName.
//
//////////////////////////////////////////////////////////////////////////
int CIRModule::FindSpace(const char* szName)
{
	size	uiIndex;

	uiIndex = maszSpaces.GetIndex(szName);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return -1;
	}
	return (int)uiIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRModule::NumSpaces(void)
{
	return maszSpaces.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIRModule::GetSpaceName(uint8 uiSpace)
{
	return maszSpaces.GetText(uiSpace);
}


//////////////////////////////////////////////////////////////////////////
//
// 0 if there is no such space.
//
//////////////////////////////////////////////////////////////////////////
int CIRModule::GetPointerSize(uint8 uiSpace)
{
	uint8*	puiSize;

	puiSize = maucSpaceSizes.SafeGet(uiSpace);
	return puiSize ? *puiSize : 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Bytes in memory; i1 is never in memory and is 0.
//
//////////////////////////////////////////////////////////////////////////
int CIRModule::GetSize(SIRType sType)
{
	switch (sType.uiKind)
	{
	case IRTK_I8:
		return 1;
	case IRTK_I16:
		return 2;
	case IRTK_I32:
	case IRTK_F32:
		return 4;
	case IRTK_I64:
	case IRTK_F64:
		return 8;
	case IRTK_Pointer:
		return GetPointerSize(sType.uiSpace);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol's index, added if it is new.
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRModule::AddSymbol(const char* szName)
{
	int32	iIndex;

	iIndex = mmSymbols.Get(szName);
	if (iIndex != -1)
	{
		return (uint32)iIndex;
	}
	iIndex = (int32)maszSymbols.NumElements();
	maszSymbols.Add(szName);
	mmSymbols.Put(szName, iIndex);
	memset(masDefinitions.Add(), 0, sizeof(SIRSymbolDefinition));
	return (uint32)iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIRModule::FindSymbol(const char* szName)
{
	return mmSymbols.Get(szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CIRModule::GetSymbolName(uint32 uiSymbol)
{
	return maszSymbols.GetText(uiSymbol);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRModule::Symbol(const char* szName)
{
	return IRReference(IRRK_Symbol, AddSymbol(szName));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRModule::AddFunction(const char* szName, SIRType sReturnType, uint32 uiAttributes)
{
	CIRFunction*	pcFunction;

	uint32			uiSymbol;

	uiSymbol = AddSymbol(szName);
	pcFunction = NewMalloc<CIRFunction>();
	pcFunction->Init(uiSymbol, sReturnType, uiAttributes);
	mapcFunctions.Add(pcFunction);
	masDefinitions.Get(uiSymbol)->pcFunction = pcFunction;
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes and frees a function; its symbol stays (undefined).
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::RemoveFunction(CIRFunction* pcFunction)
{
	size	uiIndex;

	uiIndex = mapcFunctions.Find(pcFunction);
	if (uiIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return;
	}
	mapcFunctions.RemoveAt(uiIndex, true);
	masDefinitions.Get(pcFunction->muiSymbol)->pcFunction = NULL;
	SafeKill(pcFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRModule::FindFunction(uint32 uiSymbol)
{
	SIRSymbolDefinition*	psDefinition;

	psDefinition = masDefinitions.SafeGet(uiSymbol);
	return psDefinition ? psDefinition->pcFunction : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRModule::NumFunctions(void)
{
	return mapcFunctions.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRModule::GetFunction(size uiIndex)
{
	return mapcFunctions.GetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRGlobal* CIRModule::AddGlobal(const char* szName, uint32 uiAlign, uint32 uiAttributes)
{
	CIRGlobal*	pcGlobal;

	uint32		uiSymbol;

	uiSymbol = AddSymbol(szName);
	pcGlobal = NewMalloc<CIRGlobal>();
	pcGlobal->Init(uiSymbol, uiAlign, uiAttributes);
	mapcGlobals.Add(pcGlobal);
	masDefinitions.Get(uiSymbol)->pcGlobal = pcGlobal;
	return pcGlobal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRGlobal* CIRModule::FindGlobal(uint32 uiSymbol)
{
	SIRSymbolDefinition*	psDefinition;

	psDefinition = masDefinitions.SafeGet(uiSymbol);
	return psDefinition ? psDefinition->pcGlobal : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The contents of read only data defined outside the module (a vtable the
// generator writes), for the optimiser to read through.  It is not a
// global of the module: nothing writes it out.
//
//////////////////////////////////////////////////////////////////////////
CIRGlobal* CIRModule::AddKnownData(const char* szName, uint32 uiAlign)
{
	CIRGlobal*	pcKnown;

	pcKnown = NewMalloc<CIRGlobal>();
	pcKnown->Init(AddSymbol(szName), uiAlign, IRA_Constant);
	mapcKnown.Add(pcKnown);
	return pcKnown;
}


//////////////////////////////////////////////////////////////////////////
//
// The contents of a symbol that are never written: a constant global or
// known data; NULL for any other.
//
//////////////////////////////////////////////////////////////////////////
CIRGlobal* CIRModule::FindConstantData(uint32 uiSymbol)
{
	CIRGlobal*	pcGlobal;
	size		ui;

	pcGlobal = FindGlobal(uiSymbol);
	if (pcGlobal)
	{
		return (pcGlobal->muiAttributes & IRA_Constant) ? pcGlobal : NULL;
	}
	for (ui = 0; ui < mapcKnown.NumElements(); ui++)
	{
		pcGlobal = mapcKnown.GetPtr(ui);
		if (pcGlobal->muiSymbol == uiSymbol)
		{
			return pcGlobal;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRModule::NumGlobals(void)
{
	return mapcGlobals.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRGlobal* CIRModule::GetGlobal(size uiIndex)
{
	return mapcGlobals.GetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the symbol is a function or global of this module.
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::IsDefined(uint32 uiSymbol)
{
	return (FindFunction(uiSymbol) != NULL) || (FindGlobal(uiSymbol) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The target's opcodes; pasOpcodes[0] is opcode IRO_NumGeneric.  The table
// is not copied.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetTargetOpcodes(SIROpcode* pasOpcodes, uint16 uiNumOpcodes)
{
	mpasTargetOpcodes = pasOpcodes;
	muiNumTargetOpcodes = uiNumOpcodes;
}


//////////////////////////////////////////////////////////////////////////
//
// A generic or target opcode; NULL if there is none.
//
//////////////////////////////////////////////////////////////////////////
SIROpcode* CIRModule::GetOpcode(uint16 uiOpcode)
{
	if (uiOpcode < IRO_NumGeneric)
	{
		return GetIROpcode(uiOpcode);
	}
	if (uiOpcode - IRO_NumGeneric < muiNumTargetOpcodes)
	{
		return &mpasTargetOpcodes[uiOpcode - IRO_NumGeneric];
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// IRO_None if szName is not an opcode.
//
//////////////////////////////////////////////////////////////////////////
uint16 CIRModule::FindOpcode(const char* szName)
{
	uint16	uiOpcode;

	uiOpcode = FindIROpcode(NULL, 0, 0, szName);
	if ((uiOpcode == IRO_None) && mpasTargetOpcodes)
	{
		uiOpcode = FindIROpcode(mpasTargetOpcodes, muiNumTargetOpcodes, IRO_NumGeneric, szName);
	}
	return uiOpcode;
}
