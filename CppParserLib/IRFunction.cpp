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
#include "IRFunction.h"


SIRAttributeName	gasIRAttributeNames[IRA_NumWordAttributes] =
{
	{ IRA_Local, "local" },
	{ IRA_Weak, "weak" },
	{ IRA_Constant, "constant" },
	{ IRA_NoReturn, "noreturn" },
	{ IRA_Inline, "inline" },
	{ IRA_ThisCall, "thiscall" },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::Init(uint32 uiSymbol, SIRType sReturnType, uint32 uiAttributes)
{
	muiSymbol = uiSymbol;
	msReturnType = sReturnType;
	muiAttributes = uiAttributes;
	masParameters.Init();

	masTemporaries.Init();
	maszTemporaryNames.Init();
	mpmTemporaryNames = NULL;

	masConstants.Init();
	mauiConstantHash.Init();

	mapcBlocks.Init();
	mpmBlockNames = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::Kill(void)
{
	size		ui;
	CIRBlock*	pcBlock;

	for (ui = 0; ui < mapcBlocks.NumElements(); ui++)
	{
		pcBlock = mapcBlocks.GetPtr(ui);
		SafeKill(pcBlock);
	}
	SafeKill(mpmBlockNames);
	mapcBlocks.Kill();

	mauiConstantHash.Kill();
	masConstants.Kill();

	SafeKill(mpmTemporaryNames);
	maszTemporaryNames.Kill();
	masTemporaries.Kill();

	masParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A name is kept unless it is empty or all digits (those are how unnamed
// temporaries and blocks print).
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::IsName(const char* szName)
{
	const char*	sz;

	if (szName == NULL)
	{
		return false;
	}
	for (sz = szName; *sz; sz++)
	{
		if ((*sz < '0') || (*sz > '9'))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// szName itself when it is not taken, otherwise szName.1, szName.2 ...
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::UniqueName(const char* szName, CMapStringBlock* pmNames, CChars* pszDest)
{
	uint32	uiSuffix;

	pszDest->Init(szName);
	uiSuffix = 1;
	while (pmNames->Get(pszDest->Text()) != NULL)
	{
		pszDest->Clear();
		pszDest->Append(szName);
		pszDest->Append('.');
		pszDest->Append(uiSuffix);
		uiSuffix++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddTemporary(SIRType sType, const char* szName)
{
	SIRTemporary*	psTemporary;
	CChars			szUnique;
	size			uiIndex;

	uiIndex = masTemporaries.NumElements();
	psTemporary = masTemporaries.Add();
	psTemporary->sType = sType;
	psTemporary->uiName = 0;

	if (IsName(szName))
	{
		if (mpmTemporaryNames == NULL)
		{
			mpmTemporaryNames = NewMalloc<CMapStringInt>();
			mpmTemporaryNames->Init();
		}
		UniqueName(szName, mpmTemporaryNames, &szUnique);
		maszTemporaryNames.Add(&szUnique);
		psTemporary->uiName = (uint32)maszTemporaryNames.NumElements();
		mpmTemporaryNames->Put(szUnique.Text(), (int32)uiIndex);
		szUnique.Kill();
	}
	return IRReference(IRRK_Temporary, (uint32)uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddParameter(SIRType sType, const char* szName)
{
	SIRReference	sParameter;

	sParameter = AddTemporary(sType, szName);
	masParameters.Add(&sParameter);
	return sParameter;
}


//////////////////////////////////////////////////////////////////////////
//
// IRNone if there is no temporary called szName.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::FindTemporary(const char* szName)
{
	int32	iIndex;

	iIndex = mpmTemporaryNames ? mpmTemporaryNames->Get(szName) : -1;
	if (iIndex == -1)
	{
		return IRNone();
	}
	return IRReference(IRRK_Temporary, (uint32)iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRFunction::NumTemporaries(void)
{
	return masTemporaries.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRType CIRFunction::GetTemporaryType(SIRReference sTemporary)
{
	return masTemporaries.Get(sTemporary.Index())->sType;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::SetTemporaryType(SIRReference sTemporary, SIRType sType)
{
	masTemporaries.Get(sTemporary.Index())->sType = sType;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if the temporary has no name.
//
//////////////////////////////////////////////////////////////////////////
char* CIRFunction::GetTemporaryName(SIRReference sTemporary)
{
	uint32	uiName;

	uiName = masTemporaries.Get(sTemporary.Index())->uiName;
	return (uiName == 0) ? NULL : maszTemporaryNames.GetText(uiName - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint32 CIRFunction::HashConstant(SIRType sType, uint64 ulliBits)
{
	uint64	ulliHash;

	ulliHash = (ulliBits ^ ((uint64)sType.uiKind << 56) ^ ((uint64)sType.uiSpace << 48)) * 0x9E3779B97F4A7C15ull;
	return (uint32)(ulliHash >> 32);
}


//////////////////////////////////////////////////////////////////////////
//
// Doubles the hash table (at least 16 entries) and puts every constant back.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::GrowConstantHash(void)
{
	size			uiSize;
	size			ui;
	uint32			uiMask;
	uint32			uiSlot;
	SIRConstant*	psConstant;
	uint32			uiEntry;

	uiSize = mauiConstantHash.NumElements() * 2;
	if (uiSize < 16)
	{
		uiSize = 16;
	}
	mauiConstantHash.ReInit();
	mauiConstantHash.GrowToAtLeastNumElements(uiSize, true, 0);
	uiMask = (uint32)uiSize - 1;
	for (ui = 0; ui < masConstants.NumElements(); ui++)
	{
		psConstant = masConstants.Get(ui);
		uiSlot = HashConstant(psConstant->sType, psConstant->ulliBits) & uiMask;
		while (*mauiConstantHash.Get(uiSlot) != 0)
		{
			uiSlot = (uiSlot + 1) & uiMask;
		}
		uiEntry = (uint32)ui + 1;
		mauiConstantHash.Set(uiSlot, &uiEntry);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's constant of sType with these bits, added if it is new.
// Integers are truncated to their width.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddConstant(SIRType sType, uint64 ulliBits)
{
	SIRConstant*	psConstant;
	uint32			uiMask;
	uint32			uiSlot;
	uint32			uiEntry;

	ulliBits = IRTruncate(sType, ulliBits);
	if ((masConstants.NumElements() + 1) * 2 > mauiConstantHash.NumElements())
	{
		GrowConstantHash();
	}

	uiMask = (uint32)mauiConstantHash.NumElements() - 1;
	uiSlot = HashConstant(sType, ulliBits) & uiMask;
	for (;;)
	{
		uiEntry = *mauiConstantHash.Get(uiSlot);
		if (uiEntry == 0)
		{
			break;
		}
		psConstant = masConstants.Get(uiEntry - 1);
		if (psConstant->sType.Equals(sType) && (psConstant->ulliBits == ulliBits))
		{
			return IRReference(IRRK_Constant, uiEntry - 1);
		}
		uiSlot = (uiSlot + 1) & uiMask;
	}

	uiEntry = (uint32)masConstants.NumElements() + 1;
	mauiConstantHash.Set(uiSlot, &uiEntry);
	psConstant = masConstants.Add();
	psConstant->sType = sType;
	psConstant->ulliBits = ulliBits;
	return IRReference(IRRK_Constant, uiEntry - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddInteger(SIRType sType, int64 lliValue)
{
	return AddConstant(sType, (uint64)lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddFloat(float fValue)
{
	uint32	uiBits;

	memcpy(&uiBits, &fValue, sizeof(uint32));
	return AddConstant(IRType(IRTK_F32), uiBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddDouble(double fValue)
{
	uint64	ulliBits;

	memcpy(&ulliBits, &fValue, sizeof(uint64));
	return AddConstant(IRType(IRTK_F64), ulliBits);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRConstant* CIRFunction::GetConstant(SIRReference sConstant)
{
	return masConstants.Get(sConstant.Index());
}


//////////////////////////////////////////////////////////////////////////
//
// An integer constant's value, sign extended from its width.
//
//////////////////////////////////////////////////////////////////////////
int64 CIRFunction::GetSigned(SIRReference sConstant)
{
	SIRConstant*	psConstant;

	psConstant = GetConstant(sConstant);
	return IRSignExtend(psConstant->sType, psConstant->ulliBits);
}


//////////////////////////////////////////////////////////////////////////
//
// A symbol is a pointer in the default space.
//
//////////////////////////////////////////////////////////////////////////
SIRType CIRFunction::TypeOf(SIRReference sReference)
{
	switch (sReference.Kind())
	{
	case IRRK_Temporary:
		return GetTemporaryType(sReference);
	case IRRK_Constant:
		return GetConstant(sReference)->sType;
	case IRRK_Symbol:
		return IRType(IRTK_Pointer);
	default:
		return IRType(IRTK_None);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRFunction::AddBlock(const char* szName)
{
	CIRBlock*	pcBlock;
	CChars		szUnique;

	pcBlock = NewMalloc<CIRBlock>();
	if (IsName(szName))
	{
		if (mpmBlockNames == NULL)
		{
			mpmBlockNames = NewMalloc<CMapIRBlocks>();
			mpmBlockNames->Init();
		}
		UniqueName(szName, mpmBlockNames, &szUnique);
		mpmBlockNames->Put(szUnique.Text(), &pcBlock);
		pcBlock->Init(szUnique.Text());
		szUnique.Kill();
	}
	else
	{
		pcBlock->Init();
	}
	pcBlock->muiIndex = mapcBlocks.NumElements();
	mapcBlocks.Add(pcBlock);
	return pcBlock;
}


//////////////////////////////////////////////////////////////////////////
//
// NULL if there is no block called szName.
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRFunction::FindBlock(const char* szName)
{
	CIRBlock**	ppcBlock;

	if (mpmBlockNames == NULL)
	{
		return NULL;
	}
	ppcBlock = mpmBlockNames->Get(szName);
	return ppcBlock ? *ppcBlock : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRFunction::GetEntry(void)
{
	return mapcBlocks.SafeGetPtr(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRFunction::NumBlocks(void)
{
	return mapcBlocks.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRFunction::GetBlock(size uiIndex)
{
	return mapcBlocks.GetPtr(uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// papcBlocks holds every one of the function's blocks in their new order.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::SetBlockOrder(CArrayIRBlocks* papcBlocks)
{
	mapcBlocks.Copy(papcBlocks);
	Renumber();
}


//////////////////////////////////////////////////////////////////////////
//
// Removes and frees pcBlock.  Nothing may still jump to it.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::RemoveBlock(CIRBlock* pcBlock)
{
	size	ui;

	mapcBlocks.RemoveAt(pcBlock->muiIndex, true);
	for (ui = pcBlock->muiIndex; ui < mapcBlocks.NumElements(); ui++)
	{
		mapcBlocks.GetPtr(ui)->muiIndex = ui;
	}
	if (mpmBlockNames && !pcBlock->mszName.Empty())
	{
		mpmBlockNames->Remove(pcBlock->GetName());
	}
	SafeKill(pcBlock);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::Renumber(void)
{
	size	ui;

	for (ui = 0; ui < mapcBlocks.NumElements(); ui++)
	{
		mapcBlocks.GetPtr(ui)->muiIndex = ui;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Each block's predecessors from the jumps, in block order.  A brif to one
// block twice makes it a predecessor once.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::ComputePredecessors(void)
{
	size		ui;
	CIRBlock*	pcBlock;

	for (ui = 0; ui < mapcBlocks.NumElements(); ui++)
	{
		mapcBlocks.GetPtr(ui)->mapcPredecessors.FakeSetUsedElements(0);
	}
	for (ui = 0; ui < mapcBlocks.NumElements(); ui++)
	{
		pcBlock = mapcBlocks.GetPtr(ui);
		if (pcBlock->NumSuccessors() >= 1)
		{
			pcBlock->msJump.pcThen->mapcPredecessors.Add(pcBlock);
		}
		if ((pcBlock->NumSuccessors() == 2) && (pcBlock->msJump.pcElse != pcBlock->msJump.pcThen))
		{
			pcBlock->msJump.pcElse->mapcPredecessors.Add(pcBlock);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds an instruction with a new temporary for its result: sType for most,
// i1 for a comparison.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::Add(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sArgument0, SIRReference sArgument1, const char* szName)
{
	SIROpcode*		psOpcode;
	SIRReference	sResult;

	psOpcode = GetIROpcode(uiOpcode);
	sResult = AddTemporary((psOpcode && (psOpcode->uiResult == IRRS_I1)) ? IRType(IRTK_I1) : sType, szName);
	pcBlock->AddInstruction(uiOpcode, sType, sResult, sArgument0, sArgument1);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::Add(CIRBlock* pcBlock, uint16 uiOpcode, SIRType sType, SIRReference sArgument0, const char* szName)
{
	return Add(pcBlock, uiOpcode, sType, sArgument0, IRNone(), szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::Load(CIRBlock* pcBlock, SIRType sType, SIRReference sAddress, bool bVolatile, const char* szName)
{
	SIRReference	sResult;
	SIRInstruction*	psInstruction;

	sResult = AddTemporary(sType, szName);
	psInstruction = pcBlock->AddInstruction(IRO_Load, sType, sResult, sAddress, IRNone());
	psInstruction->uiFlags = bVolatile ? IRF_Volatile : 0;
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::Store(CIRBlock* pcBlock, SIRType sType, SIRReference sValue, SIRReference sAddress, bool bVolatile)
{
	SIRInstruction*	psInstruction;

	psInstruction = pcBlock->AddInstruction(IRO_Store, sType, IRNone(), sValue, sAddress);
	psInstruction->uiFlags = bVolatile ? IRF_Volatile : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::Slot(CIRBlock* pcBlock, SIRType sPointerType, uint32 uiSize, uint32 uiAlign, const char* szName)
{
	return Add(pcBlock, IRO_Slot, sPointerType, AddInteger(IRType(IRTK_I32), uiSize), AddInteger(IRType(IRTK_I32), uiAlign), szName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::Argument(CIRBlock* pcBlock, SIRType sType, SIRReference sValue)
{
	pcBlock->AddInstruction(IRO_Arg, sType, IRNone(), sValue, IRNone());
}


//////////////////////////////////////////////////////////////////////////
//
// Calls sCallee with the arg instructions just added.  iNamed is the number
// of named arguments of a variadic call, -1 if the call is not variadic.
// The result is IRNone when sType is IRTK_None.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::Call(CIRBlock* pcBlock, SIRType sType, SIRReference sCallee, int iNamed, bool bThisCall, const char* szName)
{
	SIRReference	sResult;

	sResult = sType.IsNone() ? IRNone() : AddTemporary(sType, szName);
	AddCall(pcBlock, sType, sResult, sCallee, iNamed, bThisCall);
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The call instruction with its result already made.
//
//////////////////////////////////////////////////////////////////////////
SIRInstruction* CIRFunction::AddCall(CIRBlock* pcBlock, SIRType sType, SIRReference sResult, SIRReference sCallee, int iNamed, bool bThisCall)
{
	SIRInstruction*	psInstruction;

	psInstruction = pcBlock->AddInstruction(IRO_Call, sType, sResult, sCallee, IRNone());
	if (iNamed >= 0)
	{
		psInstruction->uiFlags = IRF_Variadic;
		psInstruction->uiNamed = (uint8)iNamed;
	}
	if (bThisCall)
	{
		psInstruction->uiFlags |= IRF_ThisCall;
	}
	return psInstruction;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times each temporary is used, by instructions, phis and jumps
// (pauiUses gets an entry per temporary, and one more).
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::CountUses(CArrayTemplate<uint32>* pauiUses)
{
	size			ui;
	size			uiInstruction;
	size			uiArgument;
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	SIRReference	sValue;

	pauiUses->FakeSetUsedElements(0);
	pauiUses->GrowToAtLeastNumElements(NumTemporaries() + 1, true, 0);
	for (ui = 0; ui < NumBlocks(); ui++)
	{
		pcBlock = GetBlock(ui);
		for (uiInstruction = 0; uiInstruction < pcBlock->masPhis.NumElements(); uiInstruction++)
		{
			psPhi = pcBlock->masPhis.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				sValue = psPhi->asArguments.Get(uiArgument)->sValue;
				if (sValue.IsTemporary())
				{
					(*pauiUses->Get(sValue.Index()))++;
				}
			}
		}
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			for (uiArgument = 0; uiArgument < 2; uiArgument++)
			{
				sValue = psInstruction->asArguments[uiArgument];
				if (sValue.IsTemporary())
				{
					(*pauiUses->Get(sValue.Index()))++;
				}
			}
		}
		sValue = pcBlock->msJump.sArgument;
		if (sValue.IsTemporary())
		{
			(*pauiUses->Get(sValue.Index()))++;
		}
	}
}
