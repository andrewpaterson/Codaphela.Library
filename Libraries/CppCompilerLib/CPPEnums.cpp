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
#include "CTInt.h"
#include "CPPUnit.h"
#include "CPPConstants.h"
#include "CPPEnums.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit's diagnostics and types, asked for once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPEnums::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcTypes = pcUnit->GetTypes();
	mcEnumeratorValues.Init();
	mcEnumeratorBodyTypes.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPEnums::Kill(void)
{
	mcEnumeratorBodyTypes.Kill();
	mcEnumeratorValues.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An enum's type (pcEnum's, or an unnamed enum's from pcSpecifier).  Its
// underlying type is the one given ("enum E : uint8"), int for a scoped enum,
// or else [dcl.enum] the first of int, unsigned int, long, unsigned long and
// long long that holds all its enumerators: TRIERROR = 0xffffffff makes
// TRISTATE an unsigned long on the W65C816, where int is 16 bits.
// pcSpecifier can be NULL or a forward declaration.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPEnums::EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier)
{
	SCPP65Type*				psType;
	SCPP65Type*				psBase;
	CCPPSyntaxEnumerator*	pcEnumerator;
	CCPPSymbol**			ppcMember;
	SMapIterator			sIter;
	CCPPSyntax*				pcDeclaration;
	int64					lliValue;
	int64					lliMin;
	int64					lliMax;
	int						iBytes;
	bool					bSigned;
	bool					bNonNegative;
	size					i;

	bNonNegative = false;
	if (pcEnum)
	{
		psType = mpcTypes->FindEnum(pcEnum);
		if (psType)
		{
			return psType;
		}
	}

	//The specifier with the enumerators: the symbol's declaration, or the parent of one of its enumerators.
	if ((!pcSpecifier || !pcSpecifier->mbBody) && pcEnum)
	{
		pcSpecifier = NULL;
		pcDeclaration = pcEnum->mpcDeclaration;
		if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_EnumSpecifier) && ((CCPPSyntaxEnumSpecifier*)pcDeclaration)->mbBody)
		{
			pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcDeclaration;
		}
		else if (pcEnum->mpcMembers && pcEnum->mpcMembers->mcSymbols.StartIteration(&sIter, NULL, NULL, (void**)&ppcMember, NULL) && ppcMember && *ppcMember)
		{
			pcDeclaration = (*ppcMember)->mpcDeclaration;
			if (pcDeclaration && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_EnumSpecifier))
			{
				pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcDeclaration->mpcParent;
			}
		}
	}

	iBytes = mpcTypes->msInt.iBytes;
	bSigned = true;
	psBase = NULL;
	if (pcSpecifier && pcSpecifier->mpcBase)
	{
		//Fixed: enum E : short is a short underneath, though int is as wide.  Each value given is one of that type's.
		//[dcl.enum] 5
		psBase = mpcUnit->GetDeclarators()->GetTypeIdType(pcSpecifier->mpcBase);
		psBase = (psBase && psBase->IsInteger()) ? psBase->Unqualified() : NULL;
		for (i = 0; psBase && (i < pcSpecifier->macpEnumerators.NumElements()); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && mpcUnit->GetFolder()->Constant(pcEnumerator->mpcValue, &lliValue))
			{
				CheckEnumeratorFits(pcSpecifier, pcEnumerator, lliValue);
			}
			SetEnumeratorBodyType(pcSpecifier, i, psBase);
		}
		ForgetEnumeratorBodyTypes(pcSpecifier);
	}
	else if (pcSpecifier && !pcSpecifier->mbScoped)
	{
		lliValue = 0;
		lliMin = 0;
		lliMax = 0;
		bNonNegative = true;
		for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && !mpcUnit->QuietConstant(pcEnumerator->mpcValue, &lliValue))
			{
				//One that can't be worked out here (sizeof(T) in a class template's enum, with no T) leaves it an int:
				//what is wrong with it is said where it is checked (CheckEnumerators).
				break;
			}
			SetEnumeratorBodyType(pcSpecifier, i, NULL);
			lliMin = (lliValue < lliMin) ? lliValue : lliMin;
			lliMax = (lliValue > lliMax) ? lliValue : lliMax;
			lliValue++;
		}
		ForgetEnumeratorBodyTypes(pcSpecifier);
		bNonNegative = (i == pcSpecifier->macpEnumerators.NumElements()) && (lliMin >= 0);

		if ((lliMin >= mpcTypes->msInt.Least()) && (lliMax <= mpcTypes->msInt.Most()))
		{
			//int.
		}
		else if ((lliMin >= 0) && (lliMax <= mpcTypes->msUnsignedInt.Most()))
		{
			bSigned = false;
		}
		else if ((lliMin >= CPPIntegerLeast(4, true)) && (lliMax <= CPPIntegerMost(4, true)))
		{
			iBytes = 4;
		}
		else if ((lliMin >= 0) && (lliMax <= CPPIntegerMost(4, false)))
		{
			iBytes = 4;
			bSigned = false;
		}
		else
		{
			iBytes = 8;
		}
	}

	if (pcEnum)
	{
		psType = mpcTypes->Enum(pcEnum, psBase ? psBase : mpcTypes->Integer(iBytes, bSigned), pcSpecifier && pcSpecifier->mbScoped);
		psType->bNonNegativeEnum = bNonNegative;
		mpcUnit->GetObserver()->EnumMade(mpcUnit, psType);
		return psType;
	}
	return psBase ? psBase : mpcTypes->Integer(iBytes, bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator's type: its enum's (an unnamed enum's is its underlying
// type).  While EnumType is working the enum's out, the one its value has
// (SetEnumeratorBodyType), as before the enum's closing brace.
// [dcl.enum] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPEnums::EnumeratorType(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*					pcEnumerator;
	CCPPSyntaxEnumSpecifier*	pcSpecifier;
	SCPP65Type**				ppsBodyType;

	pcEnumerator = pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return &mpcTypes->msInt;
	}
	ppsBodyType = mcEnumeratorBodyTypes.Get((CCPPSyntaxEnumerator*)pcEnumerator);
	if (ppsBodyType)
	{
		return *ppsBodyType;
	}
	pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;
	return EnumType(pcSpecifier->mpcSymbol, pcSpecifier);
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator is one more than the one before it unless it is given a
// value: the nearest one at or before it that is given a value (or the
// first, 0) plus how far before it that one is.  Only that value is worked
// out (GivenEnumeratorValue).  Working out every value before it, each of
// which can name the one before it (urlmon.h's BINDSTATUS_CONNECTING =
// (BINDSTATUS_FINDINGRESOURCE + 1) and so on 78 times), doubled the work
// with each enumerator.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPEnums::EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue)
{
	CCPPSyntaxEnumerator*		pcEnumerator;
	CCPPSyntaxEnumerator*		pcOther;
	CCPPSyntaxEnumSpecifier*	pcEnum;
	size						i;
	size						uiIndex;
	size						uiValued;
	size						uiNumEnumerators;
	int64						lliValue;

	pcEnumerator = (CCPPSyntaxEnumerator*)pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return false;
	}
	pcEnum = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;

	uiNumEnumerators = pcEnum->macpEnumerators.NumElements();
	for (uiIndex = 0; uiIndex < uiNumEnumerators; uiIndex++)
	{
		if (pcEnum->macpEnumerators.GetPtr(uiIndex) == pcEnumerator)
		{
			break;
		}
	}
	if (uiIndex == uiNumEnumerators)
	{
		return false;
	}

	//Without one given a value at or before it, it is as far from the first as the first's 0.
	lliValue = 0;
	uiValued = 0;
	for (i = uiIndex + 1; i > 0; i--)
	{
		pcOther = (CCPPSyntaxEnumerator*)pcEnum->macpEnumerators.GetPtr(i - 1);
		if (pcOther->mpcValue)
		{
			uiValued = i - 1;
			if (!GivenEnumeratorValue(pcEnum, uiValued, &lliValue))
			{
				return false;
			}
			break;
		}
	}
	*plliValue = lliValue + (int64)(uiIndex - uiValued);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value an enumerator is given in an enum with a fixed underlying type
// (enum E : unsigned char) is one of that type's: an error if not.
// [dcl.enum] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPEnums::CheckEnumeratorFits(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64 lliValue)
{
	SCPP65Type*	psBase;

	psBase = pcEnum->mpcBase ? mpcUnit->GetDeclarators()->GetTypeIdType(pcEnum->mpcBase) : NULL;
	if (psBase && !mpcUnit->GetInstantiator()->ConstantFits(psBase->Unqualified(), lliValue))
	{
		mpcDiagnostics->ReportOnce(pcEnumerator->mpcValue, "The constant's value isn't one of the type it is converted to");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An enum's enumerators when it is declared (pcType, a declaration's
// type, may be anything else): a value given that is definitely no
// constant (it overflows, divides by zero, takes too long ...) is an error
// even if nothing uses it.  One the compiler can't work out yet is left to
// where it is used.  [dcl.enum] 2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPEnums::CheckEnumerators(CCPPSyntax* pcType)
{
	CCPPSyntaxEnumSpecifier*	pcSpecifier;
	CCPPSyntaxEnumerator*		pcEnumerator;
	const char*					szReason;
	int64						lliValue;
	size						uiErrors;
	size						i;
	bool						bConstant;
	bool						bResult;

	if (!pcType || (pcType->GetKind() != CPPS_EnumSpecifier) || !((CCPPSyntaxEnumSpecifier*)pcType)->mbBody)
	{
		return true;
	}
	pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcType;
	bResult = true;
	for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
	{
		pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
		if (!pcEnumerator->mpcValue)
		{
			continue;
		}
		mpcDiagnostics->BeginQuiet();
		bConstant = mpcUnit->GetFolder()->Constant(pcEnumerator->mpcValue, &lliValue);
		mpcDiagnostics->EndQuiet();
		szReason = mpcUnit->GetConstants()->GetReason();
		if (!bConstant && (szReason[0] != '\0') && (strncmp(szReason, "It uses what", 12) != 0) && (strncmp(szReason, "Its initialiser uses what", 25) != 0))
		{
			bResult &= mpcUnit->GetFolder()->ConstantError(pcEnumerator->mpcValue, "An enumerator's value must be a constant");
		}
	}
	if (bResult && pcSpecifier->mpcSymbol && pcSpecifier->mpcBase)
	{
		//A fixed underlying type holds each value, whether or not the enum is used (EnumType says where one doesn't).
		//[dcl.enum] 5
		uiErrors = mpcDiagnostics->GetErrors()->NumErrors();
		EnumType(pcSpecifier->mpcSymbol, pcSpecifier);
		bResult = mpcDiagnostics->GetErrors()->NumErrors() == uiErrors;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The value enumerator uiEnumerator of pcEnum is given (it has one).  In an
// enum outside every template its value doesn't depend on the bindings it
// is asked in, so it is folded once and kept, and the values given before
// it are folded first, in order: one that names the enumerator before it
// then finds that value kept.  Folded within each other instead, urlmon.h's
// 78 BINDSTATUS_s (each the one before + 1) went 78 folds deep, which ran
// Debug's stack out.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPEnums::GivenEnumeratorValue(CCPPSyntaxEnumSpecifier* pcEnum, size uiEnumerator, int64* plliValue)
{
	CCPPSyntaxEnumerator*	pcEnumerator;
	CCPPSyntaxEnumerator*	pcBefore;
	int64*					plliKept;
	int64					lliBefore;
	size					i;

	pcEnumerator = (CCPPSyntaxEnumerator*)pcEnum->macpEnumerators.GetPtr(uiEnumerator);
	if (!IsOutsideTemplates(pcEnum))
	{
		return FoldEnumeratorValue(pcEnum, pcEnumerator, plliValue);
	}

	plliKept = mcEnumeratorValues.Get(pcEnumerator);
	if (plliKept)
	{
		*plliValue = *plliKept;
		return true;
	}
	for (i = 0; i < uiEnumerator; i++)
	{
		pcBefore = (CCPPSyntaxEnumerator*)pcEnum->macpEnumerators.GetPtr(i);
		if (pcBefore->mpcValue && !mcEnumeratorValues.HasKey(pcBefore) && FoldEnumeratorValue(pcEnum, pcBefore, &lliBefore))
		{
			mcEnumeratorValues.Put(pcBefore, &lliBefore);
		}
	}
	if (!FoldEnumeratorValue(pcEnum, pcEnumerator, plliValue))
	{
		return false;
	}
	mcEnumeratorValues.Put(pcEnumerator, plliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value pcEnumerator (of pcEnum) is given, folded, and checked against
// the enum's fixed type.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPEnums::FoldEnumeratorValue(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64* plliValue)
{
	if (!mpcUnit->GetFolder()->Constant(pcEnumerator->mpcValue, plliValue))
	{
		return false;
	}
	CheckEnumeratorFits(pcEnum, pcEnumerator, *plliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is in no template declaration and no lambda (a generic
// lambda is a template too): what it means can't depend on bindings.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPEnums::IsOutsideTemplates(CCPPSyntax* pcNode)
{
	ECPPSyntax	eKind;

	for (; pcNode; pcNode = pcNode->mpcParent)
	{
		eKind = pcNode->GetKind();
		if ((eKind == CPPS_TemplateDeclaration) || (eKind == CPPS_LambdaExpression))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Enumerator uiEnumerator of pcSpecifier (its value, if it has one, just
// folded) has its type until the enum's is made: psFixed (the underlying
// type given), else its value's, else the one before it's (int for the
// first).  The enumerators after it, whose values may name it, then type
// it without working out the enum's type within the enum's type.
//
//////////////////////////////////////////////////////////////////////////
void CCPPEnums::SetEnumeratorBodyType(CCPPSyntaxEnumSpecifier* pcSpecifier, size uiEnumerator, SCPP65Type* psFixed)
{
	CCPPSyntaxEnumerator*	pcEnumerator;
	SCPP65Type**			ppsBefore;
	SCPP65Type*				psType;

	pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(uiEnumerator);
	psType = psFixed;
	if (!psType && pcEnumerator->mpcValue)
	{
		psType = mpcUnit->GetFolder()->ConstantTypeOf(pcEnumerator->mpcValue);
	}
	if (!psType && (uiEnumerator > 0))
	{
		ppsBefore = mcEnumeratorBodyTypes.Get((CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(uiEnumerator - 1));
		psType = ppsBefore ? *ppsBefore : NULL;
	}
	if (!psType)
	{
		psType = &mpcTypes->msInt;
	}
	mcEnumeratorBodyTypes.Put((CCPPSyntaxEnumerator*)pcEnumerator, &psType);
}


//////////////////////////////////////////////////////////////////////////
//
// pcSpecifier's enumerators have its type again (EnumType has made it).
//
//////////////////////////////////////////////////////////////////////////
void CCPPEnums::ForgetEnumeratorBodyTypes(CCPPSyntaxEnumSpecifier* pcSpecifier)
{
	size	i;

	for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
	{
		mcEnumeratorBodyTypes.Remove((CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i));
	}
}
