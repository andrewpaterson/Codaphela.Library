/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

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

	//NULL if the global can't hold another (its array would pass its size, which it reports).
	psItem = masItems.Add();
	if (!psItem)
	{
		return NULL;
	}
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
	if (!psItem)
	{
		return;
	}
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
	if (!psItem)
	{
		return;
	}
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
	if (!psItem)
	{
		return;
	}
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
	SIRDataItem*	psItem;

	psItem = AddItem(IRDK_Zero);
	if (psItem)
	{
		psItem->ulliValue = uiBytes;
	}
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
	muiNumRestrict = 0;
	mapcFunctions.Init();
	mapcGlobals.Init();
	mapcKnown.Init();
	mpasTargetOpcodes = NULL;
	muiNumTargetOpcodes = 0;
	muiSource = 0;
	mpcStatistics = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What compiling the module's functions is counted and timed in (the
// optimiser, the inliner and the back ends count through the module), or
// NULL for nothing.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetStatistics(CCPPCompileStatistics* pcStatistics)
{
	mpcStatistics = pcStatistics;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::Count(ECPPCompileCount eCount, uint64 uiCount)
{
	if (mpcStatistics)
	{
		mpcStatistics->Count(eCount, uiCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::Begin(ECPPCompilePhase ePhase)
{
	if (mpcStatistics)
	{
		mpcStatistics->Begin(ePhase);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::End(ECPPCompilePhase ePhase)
{
	if (mpcStatistics)
	{
		mpcStatistics->End(ePhase);
	}
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
// The space ptr means, for a module with none yet.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIRModule::AddDefaultSpace(int iPointerBytes)
{
	return AddSpace("default", iPointerBytes);
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
size CIRModule::NumSpaces(void) const
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
	int32*	piIndex;
	int32	iIndex;
	bool	bAdded;

	piIndex = mmSymbols.FindOrAdd(szName, (size)strlen(szName), &bAdded);
	if (!bAdded)
	{
		return (uint32)*piIndex;
	}
	iIndex = (int32)maszSymbols.NumElements();
	*piIndex = iIndex;
	maszSymbols.Add(szName);
	memset(masDefinitions.Add(), 0, sizeof(SIRSymbolDefinition));
	return (uint32)iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIRModule::FindSymbol(const char* szName)
{
	int32*	piIndex;

	piIndex = mmSymbols.Get(szName);
	return piIndex ? *piIndex : -1;
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


SIRSymbolAttributeName	gasIRSymbolAttributeNames[IRSA_NumWritten] =
{
	{ IRSA_Restrict, "restrict" },
	{ IRSA_RegisterCall, "regcall" },
	{ IRSA_NoWrites, "nowrites" },
	{ IRSA_NoThrow, "nothrow" },
};


//////////////////////////////////////////////////////////////////////////
//
// Gives the symbol each of uiAttributes (IRSA_), or takes them from it.
// The setters named for one attribute say what each means.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetSymbolAttributes(uint32 uiSymbol, uint8 uiAttributes, bool bSet)
{
	SIRSymbolDefinition*	psDefinition;
	uint8					uiBefore;

	psDefinition = masDefinitions.Get(uiSymbol);
	uiBefore = psDefinition->uiAttributes;
	psDefinition->uiAttributes = bSet ? (uint8)(uiBefore | uiAttributes) : (uint8)(uiBefore & ~uiAttributes);
	if ((uiBefore ^ psDefinition->uiAttributes) & IRSA_Restrict)
	{
		//HasRestrict counts them.
		if (bSet)
		{
			muiNumRestrict++;
		}
		else
		{
			muiNumRestrict--;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The symbol's attributes: none for a symbol the module hasn't.
//
//////////////////////////////////////////////////////////////////////////
uint8 CIRModule::GetSymbolAttributes(uint32 uiSymbol)
{
	return (uiSymbol < masDefinitions.NumElements()) ? masDefinitions.Get(uiSymbol)->uiAttributes : (uint8)0;
}


//////////////////////////////////////////////////////////////////////////
//
// What pcFrom knows of its symbol uiFromSymbol, of the attributes in
// uiAttributes, is known of uiSymbol here: the same symbol named in
// another unit's module (IRSA_Carried says which go with it).
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::CopySymbolAttributes(uint32 uiSymbol, CIRModule* pcFrom, uint32 uiFromSymbol, uint8 uiAttributes)
{
	SetSymbolAttributes(uiSymbol, pcFrom->GetSymbolAttributes(uiFromSymbol) & uiAttributes);
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the symbol as a pointer variable declared restrict (C's restrict on
// a file scope pointer): memory reached through a value loaded from it is
// reached through no other symbol or frame slot.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetRestrict(uint32 uiSymbol)
{
	SetSymbolAttributes(uiSymbol, IRSA_Restrict);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::IsRestrict(uint32 uiSymbol)
{
	return (GetSymbolAttributes(uiSymbol) & IRSA_Restrict) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// True if some symbol is restrict.
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::HasRestrict(void) const
{
	return muiNumRestrict != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the symbol as the unit's own (a string, a static variable or a
// static function): another unit's symbol of the same name is another
// thing.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetLocal(uint32 uiSymbol)
{
	SetSymbolAttributes(uiSymbol, IRSA_Local);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::IsLocal(uint32 uiSymbol)
{
	return (GetSymbolAttributes(uiSymbol) & IRSA_Local) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Marks the symbol as a function whose arguments a back end may pass in
// registers by its fixed rule (the W65C816's: CIRW65C816Backend's register
// arguments), given only the argument types.  The front end marks the
// functions with C++ linkage that aren't variadic: extern "C" functions
// (often written in assembly) and the runtime's keep the stack.  Every unit
// that names the function marks it, so callers and the definition agree.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetRegisterCall(uint32 uiSymbol)
{
	SetSymbolAttributes(uiSymbol, IRSA_RegisterCall);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::IsRegisterCall(uint32 uiSymbol)
{
	return (GetSymbolAttributes(uiSymbol) & IRSA_RegisterCall) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the symbol is a function that writes no memory its callers can
// see: only its own frame, no volatile access, and it calls only such
// functions (CIRInterprocedural::MarkNoWrites works it out for the unit's
// functions).  A call to it is no barrier to what the optimiser knows is
// in memory (CIRPasses::MayWriteMemory).
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetNoWrites(uint32 uiSymbol, bool bNoWrites)
{
	SetSymbolAttributes(uiSymbol, IRSA_NoWrites, bNoWrites);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::IsNoWrites(uint32 uiSymbol)
{
	return (GetSymbolAttributes(uiSymbol) & IRSA_NoWrites) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the symbol is a function that is never left with an exception: a
// front end marks what a declaration says, and CIRInterprocedural's
// MarkNoThrow works it out for the unit's functions.  The thrown after a
// call to it is false (NeverThrown), so the code a caller has for an
// exception from it goes.
//
//////////////////////////////////////////////////////////////////////////
void CIRModule::SetNoThrow(uint32 uiSymbol, bool bNoThrow)
{
	SetSymbolAttributes(uiSymbol, IRSA_NoThrow, bNoThrow);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRModule::IsNoThrow(uint32 uiSymbol)
{
	return (GetSymbolAttributes(uiSymbol) & IRSA_NoThrow) != 0;
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
	pcFunction->Init(uiSymbol, sReturnType, uiAttributes, &muiSource);
	mapcFunctions.Add(pcFunction);
	masDefinitions.Get(uiSymbol)->pcFunction = pcFunction;
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of pcFunction named szName, local, taking register arguments as it
// does and never throwing if it doesn't.  An existing function of that name
// is it.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRModule::CopyFunction(CIRFunction* pcFunction, const char* szName)
{
	CIRFunction*	pcCopy;
	int				iSymbol;

	iSymbol = FindSymbol(szName);
	pcCopy = (iSymbol >= 0) ? FindFunction((uint32)iSymbol) : NULL;
	if (pcCopy)
	{
		return pcCopy;
	}
	pcCopy = AddFunction(szName, pcFunction->msReturnType, pcFunction->muiAttributes);
	pcCopy->CopyBody(pcFunction);
	SetLocal(pcCopy->muiSymbol);
	if (IsRegisterCall(pcFunction->muiSymbol))
	{
		SetRegisterCall(pcCopy->muiSymbol);
	}
	if (IsNoThrow(pcFunction->muiSymbol))
	{
		SetNoThrow(pcCopy->muiSymbol, true);
	}
	return pcCopy;
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
size CIRModule::NumFunctions(void) const
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
size CIRModule::NumGlobals(void) const
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
