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
#include "IROptimiserInternal.h"


//What an address is an address of: the questions the optimiser's passes ask (CIRAddresses).


//////////////////////////////////////////////////////////////////////////
//
// An address as a root (a temporary or symbol that isn't a ptradd of a
// constant) and a constant offset from it.
//
//////////////////////////////////////////////////////////////////////////
void CIRAddresses::SplitAddress(SIRReference sAddress, SIRReference* psRoot, int64* plliOffset)
{
	SIRInstruction*	psDefinition;
	int				iSteps;
	size			uiParameter;

	*psRoot = sAddress;
	*plliOffset = 0;
	for (iSteps = 0; iSteps < IR_ADDRESS_STEPS; iSteps++)
	{
		psDefinition = mpcContext->mcDefinitions.DefinedBy(*psRoot, IRO_PtrAdd);
		if (!psDefinition || !psDefinition->asArguments[1].IsConstant())
		{
			break;
		}
		*plliOffset += mpcFunction->GetSigned(psDefinition->asArguments[1]);
		*psRoot = psDefinition->asArguments[0];
	}

	//Every argaddr of a parameter is the same memory: the first is its root.
	psDefinition = mpcContext->mcDefinitions.Instruction(*psRoot);
	if (psDefinition && (psDefinition->uiOpcode == IRO_ArgAddress))
	{
		uiParameter = (size)mpcFunction->GetSigned(psDefinition->asArguments[0]);
		if (uiParameter < mpcContext->masArgAddresses.NumElements())
		{
			*psRoot = *mpcContext->masArgAddresses.Get(uiParameter);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if a root is a slot or an argaddr: memory no other root names.
//
//////////////////////////////////////////////////////////////////////////
bool CIRAddresses::IsFrameRoot(SIRReference sRoot)
{
	SIRInstruction*	psDefinition;

	psDefinition = mpcContext->mcDefinitions.Instruction(sRoot);
	return psDefinition && ((psDefinition->uiOpcode == IRO_Slot) || (psDefinition->uiOpcode == IRO_ArgAddress));
}


//////////////////////////////////////////////////////////////////////////
//
// What a root's pointer is based on: a pointer loaded from a restrict
// symbol or a parameter, through ptradds (with any offset) and phis whose
// values are all based on the same.  Else IRPB_Unknown.
//
//////////////////////////////////////////////////////////////////////////
void CIRAddresses::PointerBase(SIRReference sRoot, SIRPointerBase* psBase)
{
	SIRReference	asWork[IR_BASE_VALUES];
	SIRReference	asSeen[IR_BASE_VALUES];
	size			uiWork;
	size			uiSeen;
	size			ui;
	SIRReference	sValue;
	SIRInstruction*	psDefinition;
	SIRPhi*			psPhi;
	SIRPointerBase	sBase;
	bool			bFound;
	int				iSteps;
	int				iParameter;

	psBase->eKind = IRPB_Unknown;
	if (!mpcContext->mbRestrict)
	{
		return;
	}

	asWork[0] = sRoot;
	uiWork = 1;
	uiSeen = 0;
	bFound = false;
	while (uiWork > 0)
	{
		sValue = asWork[--uiWork];
		for (iSteps = 0; iSteps < IR_ADDRESS_STEPS; iSteps++)
		{
			psDefinition = mpcContext->mcDefinitions.DefinedBy(sValue, IRO_PtrAdd);
			if (!psDefinition)
			{
				break;
			}
			sValue = psDefinition->asArguments[0];
		}
		for (ui = 0; ui < uiSeen; ui++)
		{
			if (asSeen[ui].Equals(sValue))
			{
				break;
			}
		}
		if (ui < uiSeen)
		{
			continue;
		}
		if (uiSeen == IR_BASE_VALUES)
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}
		asSeen[uiSeen++] = sValue;

		psPhi = mpcContext->mcDefinitions.Phi(sValue);
		if (psPhi)
		{
			for (ui = 0; ui < psPhi->asArguments.NumElements(); ui++)
			{
				if (uiWork == IR_BASE_VALUES)
				{
					psBase->eKind = IRPB_Unknown;
					return;
				}
				asWork[uiWork++] = psPhi->asArguments.Get(ui)->sValue;
			}
			continue;
		}

		iParameter = mpcFunction->FindParameter(sValue);
		psDefinition = mpcContext->mcDefinitions.Instruction(sValue);
		if (iParameter >= 0)
		{
			sBase.eKind = IRPB_Parameter;
			sBase.uiIndex = (uint32)iParameter;
			sBase.bRestrict = mpcFunction->IsRestrictParameter((size)iParameter);
		}
		else if (psDefinition && (psDefinition->uiOpcode == IRO_Load) && psDefinition->asArguments[0].IsSymbol() && mpcModule->IsRestrict(psDefinition->asArguments[0].Index()))
		{
			sBase.eKind = IRPB_RestrictSymbol;
			sBase.uiIndex = psDefinition->asArguments[0].Index();
			sBase.bRestrict = true;
		}
		else
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}

		if (!bFound)
		{
			*psBase = sBase;
			bFound = true;
		}
		else if ((psBase->eKind != sBase.eKind) || (psBase->uiIndex != sBase.uiIndex))
		{
			psBase->eKind = IRPB_Unknown;
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// False only if the two accesses can't touch the same bytes: the same root
// at offsets that don't overlap, different symbols, different slots or
// argaddrs, or a slot and a symbol.  What is reached through a restrict
// pointer (a restrict symbol's value or a restrict parameter, see
// PointerBase) is no symbol's or slot's, and isn't reached through a
// pointer with another base when either base is restrict and a parameter,
// or both are restrict.  A restrict symbol and a plain parameter may alias:
// the caller may have passed the symbol's value.
//
//////////////////////////////////////////////////////////////////////////
bool CIRAddresses::MayAlias(SIRAvailable* psAvailable, SIRReference sRoot, int64 lliOffset, int iBytes)
{
	bool			bFrame1;
	bool			bFrame2;
	SIRPointerBase	sBase1;
	SIRPointerBase	sBase2;
	bool			bRestrict1;
	bool			bRestrict2;

	if (psAvailable->sRoot.Equals(sRoot))
	{
		return (psAvailable->lliOffset < lliOffset + iBytes) && (lliOffset < psAvailable->lliOffset + mpcModule->GetSize(psAvailable->sType));
	}
	bFrame1 = IsFrameRoot(psAvailable->sRoot) || psAvailable->sRoot.IsSymbol();
	bFrame2 = IsFrameRoot(sRoot) || sRoot.IsSymbol();
	if (bFrame1 && bFrame2)
	{
		return false;
	}

	sBase1.eKind = IRPB_Unknown;
	sBase2.eKind = IRPB_Unknown;
	if (!bFrame1)
	{
		PointerBase(psAvailable->sRoot, &sBase1);
	}
	if (!bFrame2)
	{
		PointerBase(sRoot, &sBase2);
	}
	bRestrict1 = (sBase1.eKind != IRPB_Unknown) && sBase1.bRestrict;
	bRestrict2 = (sBase2.eKind != IRPB_Unknown) && sBase2.bRestrict;
	if ((bRestrict1 && bFrame2) || (bRestrict2 && bFrame1))
	{
		return false;
	}
	if ((sBase1.eKind == IRPB_Unknown) || (sBase2.eKind == IRPB_Unknown))
	{
		return true;
	}
	if ((sBase1.eKind == sBase2.eKind) && (sBase1.uiIndex == sBase2.uiIndex))
	{
		return true;
	}
	if ((bRestrict1 && bRestrict2) || (bRestrict1 && (sBase1.eKind == IRPB_Parameter)) || (bRestrict2 && (sBase2.eKind == IRPB_Parameter)))
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sAddress (through ptradds, any offset) is in the function's own
// frame: a slot or a parameter's argaddr, gone when it returns.  Needs
// FindDefinitions.
//
//////////////////////////////////////////////////////////////////////////
bool CIRAddresses::IsOwnFrame(SIRReference sAddress)
{
	SIRInstruction*	psDefinition;
	int				iSteps;

	for (iSteps = 0; iSteps < IR_FRAME_STEPS; iSteps++)
	{
		psDefinition = mpcContext->mcDefinitions.Instruction(sAddress);
		if (psDefinition == NULL)
		{
			return false;
		}
		if ((psDefinition->uiOpcode == IRO_Slot) || (psDefinition->uiOpcode == IRO_ArgAddress))
		{
			return true;
		}
		if (psDefinition->uiOpcode != IRO_PtrAdd)
		{
			return false;
		}
		sAddress = psDefinition->asArguments[0];
	}
	return false;
}
