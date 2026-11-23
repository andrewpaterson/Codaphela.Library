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
#include "IRFunction.h"
#include "IRBlockCopier.h"


SIRAttributeName	gasIRAttributeNames[IRA_NumWordAttributes] =
{
	{ IRA_Local, "local" },
	{ IRA_Weak, "weak" },
	{ IRA_Constant, "constant" },
	{ IRA_NoReturn, "noreturn" },
	{ IRA_Inline, "inline" },
	{ IRA_ThisCall, "thiscall" },
	{ IRA_NoInline, "noinline" },
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::Init(uint32 uiSymbol, SIRType sReturnType, uint32 uiAttributes, uint32* puiSource)
{
	mpuiSource = puiSource;
	muiSymbol = uiSymbol;
	msReturnType = sReturnType;
	muiAttributes = uiAttributes;
	masParameters.Init();
	muiRestrictParameters = 0;

	masTemporaries.Init();
	maszTemporaryNames.Init();
	mpmTemporaryNames = NULL;

	mcConstants.Init();

	masKept.Init();
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
	masKept.Kill();

	mcConstants.Kill();

	SafeKill(mpmTemporaryNames);
	maszTemporaryNames.Kill();
	masTemporaries.Kill();

	masParameters.Kill();
}


//CopyBody's copier: every temporary and constant keeps its index, so a value is itself, and a block is the one
//with its index.
class CIRBodyCopier : public CIRBlockCopier
{
public:
	CIRFunction*	mpcTo;

protected:
	SIRReference	CopyValue(SIRReference sValue) override;
	CIRBlock*		CopyOf(CIRBlock* pcBlock) override;
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRBodyCopier::CopyValue(SIRReference sValue)
{
	return sValue;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIRBlock* CIRBodyCopier::CopyOf(CIRBlock* pcBlock)
{
	return mpcTo->GetBlock(pcBlock->muiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
// This function (just made, in pcSource's module) gets pcSource's parameters,
// temporaries, constants and blocks, with their sources.  Every temporary and
// constant keeps its index, so references mean the same here.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::CopyBody(CIRFunction* pcSource)
{
	CIRBodyCopier	cCopier;
	CIRBlock*		pcFrom;
	CIRBlock*		pcTo;
	SIRConstant*	psConstant;
	SIRReference	sTemporary;
	size			ui;
	size			uiBlock;

	for (ui = 0; ui < pcSource->masTemporaries.NumElements(); ui++)
	{
		sTemporary = IRReference(IRRK_Temporary, (uint32)ui);
		AddTemporary(pcSource->GetTemporaryType(sTemporary), pcSource->GetTemporaryName(sTemporary));
	}
	for (ui = 0; ui < pcSource->masParameters.NumElements(); ui++)
	{
		masParameters.Add(pcSource->masParameters.Get(ui));
	}
	muiRestrictParameters = pcSource->muiRestrictParameters;
	for (ui = 0; ui < pcSource->masKept.NumElements(); ui++)
	{
		masKept.Add(pcSource->masKept.Get(ui));
	}
	for (ui = 0; ui < pcSource->mcConstants.NumElements(); ui++)
	{
		psConstant = pcSource->mcConstants.GetKey(ui);
		AddConstant(psConstant->sType, psConstant->ulliBits);
	}

	//The blocks first: phis and jumps name them, by their index.
	pcSource->Renumber();
	for (uiBlock = 0; uiBlock < pcSource->NumBlocks(); uiBlock++)
	{
		AddBlock(pcSource->GetBlock(uiBlock)->GetName());
	}
	cCopier.mpcTo = this;
	for (uiBlock = 0; uiBlock < pcSource->NumBlocks(); uiBlock++)
	{
		pcFrom = pcSource->GetBlock(uiBlock);
		pcTo = GetBlock(uiBlock);
		cCopier.CopyBlock(pcFrom, pcTo);
		pcTo->msJump.uiSource = pcFrom->msJump.uiSource;
	}
	ComputePredecessors();
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
// Marks a pointer parameter restrict (C's restrict): memory reached through
// pointers based on its value is reached through no other pointer, symbol
// or slot while the function runs.  Only the first 32 parameters can be.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::SetRestrictParameter(size uiParameter)
{
	if (uiParameter < 32)
	{
		muiRestrictParameters |= (uint32)1 << uiParameter;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::IsRestrictParameter(size uiParameter)
{
	return (uiParameter < 32) && ((muiRestrictParameters & ((uint32)1 << uiParameter)) != 0);
}


//////////////////////////////////////////////////////////////////////////
//
// The index of the parameter sValue is, else -1.
//
//////////////////////////////////////////////////////////////////////////
int CIRFunction::FindParameter(SIRReference sValue)
{
	size	ui;

	if (!sValue.IsTemporary())
	{
		return -1;
	}
	for (ui = 0; ui < masParameters.NumElements(); ui++)
	{
		if (masParameters.Get(ui)->Equals(sValue))
		{
			return (int)ui;
		}
	}
	return -1;
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
size CIRFunction::NumTemporaries(void) const
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
// The function's constant of sType with these bits, added if it is new.
// Integers are truncated to their width.
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::AddConstant(SIRType sType, uint64 ulliBits)
{
	SIRConstant	sConstant;

	sConstant.sType = sType;
	sConstant.ulliBits = IRTruncate(sType, ulliBits);
	return IRReference(IRRK_Constant, (uint32)mcConstants.FindOrAddIndex(&sConstant));
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
	return mcConstants.GetKey(sConstant.Index());
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
		pcBlock->Init(mpuiSource, szUnique.Text());
		szUnique.Kill();
	}
	else
	{
		pcBlock->Init(mpuiSource);
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
size CIRFunction::NumBlocks(void) const
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
// Whether each block's index is its place already (Renumber would change
// nothing).
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::IsNumbered(void)
{
	size	ui;

	for (ui = 0; ui < mapcBlocks.NumElements(); ui++)
	{
		if (mapcBlocks.GetPtr(ui)->muiIndex != ui)
		{
			return false;
		}
	}
	return true;
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
	if ((uiOpcode == IRO_PtrAdd) && IsKept(sArgument0))
	{
		masKept.Add(&sResult);
	}
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
	if (psInstruction)
	{
		psInstruction->uiFlags = (bVolatile || IsKept(sAddress)) ? IRF_Volatile : 0;
	}
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
	if (psInstruction)
	{
		psInstruction->uiFlags = (bVolatile || IsKept(sAddress)) ? IRF_Volatile : 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIRReference CIRFunction::Slot(CIRBlock* pcBlock, SIRType sPointerType, uint32 uiSize, uint32 uiAlign, const char* szName, bool bKept)
{
	SIRReference	sResult;

	sResult = Add(pcBlock, IRO_Slot, sPointerType, AddInteger(IRType(IRTK_I32), uiSize), AddInteger(IRType(IRTK_I32), uiAlign), szName);
	if (bKept)
	{
		//A variable a debugger reads: what is stored in it is always there (volatile loads and stores, which no pass
		//takes out or moves), and the slot itself is never promoted, split or shared.
		pcBlock->masInstructions.Tail()->uiFlags |= IRF_Volatile;
		masKept.Add(&sResult);
	}
	return sResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The kept slots' loads and stores, volatile while the function is
// optimised (so no pass takes one out or moves it), made plain for the
// back end: it addresses them as any slot's, and keeps their stores
// itself (the slots are still marked, IRF_Volatile on the slot).  An
// access is a kept slot's when its address is the slot or a ptradd of it.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::PlainKeptAccesses(void)
{
	CArrayTemplatePtr<SIRInstruction>	apsDefinitions;
	size								uiBlock;
	size								uiInstruction;
	CIRBlock*							pcBlock;
	SIRInstruction*						psInstruction;
	SIRInstruction*						psDefinition;
	SIRReference						sAddress;

	if (masKept.NumElements() == 0)
	{
		return;
	}
	apsDefinitions.Init();
	apsDefinitions.GrowToAtLeastNumElements(NumTemporaries(), true, NULL);
	for (uiBlock = 0; uiBlock < NumBlocks(); uiBlock++)
	{
		pcBlock = GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if (psInstruction->sResult.IsTemporary())
			{
				apsDefinitions.SetPtr(psInstruction->sResult.Index(), psInstruction);
			}
		}
	}
	for (uiBlock = 0; uiBlock < NumBlocks(); uiBlock++)
	{
		pcBlock = GetBlock(uiBlock);
		for (uiInstruction = 0; uiInstruction < pcBlock->masInstructions.NumElements(); uiInstruction++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiInstruction);
			if ((psInstruction->uiOpcode != IRO_Load) && (psInstruction->uiOpcode != IRO_Store))
			{
				continue;
			}
			sAddress = psInstruction->asArguments[(psInstruction->uiOpcode == IRO_Load) ? 0 : 1];
			psDefinition = NULL;
			while (sAddress.IsTemporary())
			{
				psDefinition = apsDefinitions.GetPtr(sAddress.Index());
				if (!psDefinition || (psDefinition->uiOpcode != IRO_PtrAdd))
				{
					break;
				}
				sAddress = psDefinition->asArguments[0];
			}
			if (psDefinition && (psDefinition->uiOpcode == IRO_Slot) && (psDefinition->uiFlags & IRF_Volatile))
			{
				psInstruction->uiFlags &= ~IRF_Volatile;
			}
		}
	}
	apsDefinitions.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sAddress is in a kept slot (Slot's bKept): the slot, or an
// address added to it.
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::IsKept(SIRReference sAddress)
{
	size	ui;

	for (ui = 0; ui < masKept.NumElements(); ui++)
	{
		if (masKept.Get(ui)->Equals(sAddress))
		{
			return true;
		}
	}
	return false;
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
	if (!psInstruction)
	{
		return NULL;
	}
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


//////////////////////////////////////////////////////////////////////////
//
// False unless psInstruction (of pcFunction) is a divide or a remainder of
// integers by a constant.
//
//////////////////////////////////////////////////////////////////////////
bool SIRConstantDivision::Init(CIRFunction* pcFunction, SIRInstruction* psInstruction)
{
	uint64	ulliBits;

	if (!IsIRDivision(psInstruction->uiOpcode) || !psInstruction->asArguments[1].IsConstant() || !psInstruction->sType.IsInteger())
	{
		return false;
	}
	bSigned = IsIRSignedDivision(psInstruction->uiOpcode);
	bRemainder = IsIRRemainder(psInstruction->uiOpcode);
	ulliBits = pcFunction->GetConstant(psInstruction->asArguments[1])->ulliBits;
	lliDivisor = bSigned ? IRSignExtend(psInstruction->sType, ulliBits) : (int64)IRTruncate(psInstruction->sType, ulliBits);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts a walk of sValue's uses (of every temporary with bTemporaries)
// in papcBlocks (NULL for all the function's blocks) at the first; false
// if there is none.  NextUse goes on to the next.  One walk serves the
// passes that look for, count or rewrite the uses of a value, each with its
// own filter on the blocks.
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::FirstUse(SIRUse* psUse, SIRReference sValue, CArrayIRBlocks* papcBlocks, bool bTemporaries)
{
	psUse->sValue = sValue;
	psUse->bTemporaries = bTemporaries;
	psUse->papcBlocks = papcBlocks;
	psUse->uiBlock = 0;
	psUse->eKind = IRUK_Phi;
	psUse->uiIndex = 0;
	psUse->uiArgument = (size)-1;
	return NextUse(psUse);
}


//////////////////////////////////////////////////////////////////////////
//
// FirstUse of every temporary instead of one value: each operand that is a
// temporary.
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::FirstTemporaryUse(SIRUse* psUse, CArrayIRBlocks* papcBlocks)
{
	return FirstUse(psUse, IRNone(), papcBlocks, true);
}


//////////////////////////////////////////////////////////////////////////
//
// The walk's next use after the one it is at (block by block: the phis'
// arguments, the instructions' operands, then the jump); false when there
// are no more.  The block, phi and instruction are fetched again by their
// indices each step, and the counts read again, so the walker may add
// phis and instructions to the ends of blocks between steps (those added
// to the block it is in, after where it is, are walked too).
//
//////////////////////////////////////////////////////////////////////////
bool CIRFunction::NextUse(SIRUse* psUse)
{
	size			uiBlocks;
	SIRReference*	psSlot;

	psUse->uiArgument++;
	for (;;)
	{
		uiBlocks = psUse->papcBlocks ? psUse->papcBlocks->NumElements() : NumBlocks();
		if (psUse->uiBlock >= uiBlocks)
		{
			return false;
		}
		psUse->pcBlock = psUse->papcBlocks ? psUse->papcBlocks->GetPtr(psUse->uiBlock) : GetBlock(psUse->uiBlock);
		if (psUse->eKind == IRUK_Phi)
		{
			if (psUse->uiIndex >= psUse->pcBlock->masPhis.NumElements())
			{
				psUse->eKind = IRUK_Instruction;
				psUse->uiIndex = 0;
				psUse->uiArgument = 0;
				continue;
			}
			psUse->psPhi = psUse->pcBlock->masPhis.Get(psUse->uiIndex);
			if (psUse->uiArgument >= psUse->psPhi->asArguments.NumElements())
			{
				psUse->uiIndex++;
				psUse->uiArgument = 0;
				continue;
			}
			psUse->psArgument = psUse->psPhi->asArguments.Get(psUse->uiArgument);
			psSlot = &psUse->psArgument->sValue;
		}
		else if (psUse->eKind == IRUK_Instruction)
		{
			if (psUse->uiIndex >= psUse->pcBlock->masInstructions.NumElements())
			{
				psUse->eKind = IRUK_Jump;
				psUse->uiIndex = 0;
				psUse->uiArgument = 0;
				continue;
			}
			if (psUse->uiArgument >= 2)
			{
				psUse->uiIndex++;
				psUse->uiArgument = 0;
				continue;
			}
			psUse->psInstruction = psUse->pcBlock->masInstructions.Get(psUse->uiIndex);
			psSlot = &psUse->psInstruction->asArguments[psUse->uiArgument];
		}
		else
		{
			if (psUse->uiArgument >= 1)
			{
				psUse->uiBlock++;
				psUse->eKind = IRUK_Phi;
				psUse->uiIndex = 0;
				psUse->uiArgument = 0;
				continue;
			}
			psSlot = &psUse->pcBlock->msJump.sArgument;
		}
		if (psUse->bTemporaries ? psSlot->IsTemporary() : psSlot->Equals(psUse->sValue))
		{
			psUse->psSlot = psSlot;
			return true;
		}
		psUse->uiArgument++;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The use the walk is at reads sNew.  It is found again by its indices: a
// walk that worked sNew out may have added phis (CIRSSAUpdater), moving the
// block's phis.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::SetUse(SIRUse* psUse, SIRReference sNew)
{
	CIRBlock*	pcBlock;

	pcBlock = psUse->papcBlocks ? psUse->papcBlocks->GetPtr(psUse->uiBlock) : GetBlock(psUse->uiBlock);
	if (psUse->eKind == IRUK_Phi)
	{
		psUse->psPhi = pcBlock->masPhis.Get(psUse->uiIndex);
		psUse->psArgument = psUse->psPhi->asArguments.Get(psUse->uiArgument);
		psUse->psSlot = &psUse->psArgument->sValue;
	}
	else if (psUse->eKind == IRUK_Instruction)
	{
		psUse->psInstruction = pcBlock->masInstructions.Get(psUse->uiIndex);
		psUse->psSlot = &psUse->psInstruction->asArguments[psUse->uiArgument];
	}
	else
	{
		psUse->psSlot = &pcBlock->msJump.sArgument;
	}
	*psUse->psSlot = sNew;
}


//////////////////////////////////////////////////////////////////////////
//
// Every use of sOld (an operand, a phi's argument, a branch's or return's
// value) is sNew.
//
//////////////////////////////////////////////////////////////////////////
void CIRFunction::ReplaceUses(SIRReference sOld, SIRReference sNew)
{
	SIRUse	sUse;
	bool	bUse;

	for (bUse = FirstUse(&sUse, sOld); bUse; bUse = NextUse(&sUse))
	{
		*sUse.psSlot = sNew;
	}
}
