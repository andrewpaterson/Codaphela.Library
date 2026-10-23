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
#include "CTIdentifier.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// Objects as values.  An object argument is copied onto the stack in its
// argument's place, and the called function destroys it.  A function that returns
// an object gets a hidden argument, pushed last so it is the first: the address to
// construct its result at, which it also returns in A.  The caller passes the
// address of a temporary: each expression that makes an object (a call, an
// operator, Class(arguments)) has its own place in the frame, worked out before
// the function's code, and temporaries that need destroying are destroyed at the
// end of their full expression.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// Gives each expression in the function that makes an object a place in the frame
// for it.  The locals must have their places first, for the expressions' types.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AllocateTemporaries(CCPPSyntax* pcNode)
{
	SCPP65Type*		psType;
	SCPP65Slot		sSlot;
	size			i;

	if (!pcNode)
	{
		return;
	}
	if (MakesObject(pcNode) || (pcNode->GetKind() == CPPS_ConstructExpression))
	{
		psType = TypeOf(pcNode);
		if (psType && psType->IsClass() && (mcTemporarySlots.Get((int64)(size)pcNode) == NULL))
		{
			miFrameBytes = (miFrameBytes + 3) & ~3;
			sSlot.iOffset = miFrameBytes;
			mcTemporarySlots.Put((int64)(size)pcNode, &sSlot);
			miFrameBytes += psType->iBytes;
		}
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		AllocateTemporaries(pcNode->GetChild(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the address of the temporary for the object pcNode makes in A.  One that
// needs destroying is destroyed at the end of the full expression.  Outside a
// function (a global's initialiser) or for an expression from elsewhere (a default
// argument) the temporary is data.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::TemporaryAddress(CCPPSyntax* pcNode, SCPP65Type* psType)
{
	SCPP65Slot*		psSlot;
	SCPP65Pending*	psPending;
	CChars			szLabel;

	//Made where the object it initialises is.
	if ((miTypeOf == 0) && mpcElide && (mpcElide == pcNode))
	{
		mpcElide = NULL;
		switch (meElide)
		{
		case CPP65E_Result:
			LoadResultAddress();
			break;
		case CPP65E_Stack:
			Line("cpy A, S")->AppendNewLine();
			Line("add A, ")->Append(1 + miDepth - miElideDepth)->AppendNewLine();
			break;
		default:
			LoadAddress(&msElide);
			break;
		}
		return true;
	}

	psSlot = mpsFunction ? mcTemporarySlots.Get((int64)(size)pcNode) : NULL;
	if (psSlot)
	{
		Line("cpy A, V")->AppendNewLine();
		Line("add A, ")->Append(1 + psSlot->iOffset)->AppendNewLine();
		if (psType->psClass->psDestructor && (miTypeOf == 0))
		{
			psPending = macPendingTemporaries.Add();
			psPending->iOffset = psSlot->iOffset;
			psPending->psType = psType;
		}
		return true;
	}
	if (miTypeOf > 0)
	{
		Line("cpy A, 0")->AppendNewLine();
		return true;
	}
	if (psType->psClass->psDestructor)
	{
		return Error(pcNode, "Temporary objects that need destroying are not supported here by the 65C64 generator yet");
	}

	szLabel.Init("T");
	szLabel.Append(miDataTemporaries);
	miDataTemporaries++;
	BeginData(&szLabel, 4, false, NULL);
	AppendZeros(psType->iBytes);
	Line("cpy A, ")->Append(szLabel)->AppendNewLine();
	szLabel.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the temporaries of a full expression, last first.  bKeep keeps A (and
// all 8 bytes when bWide).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DestroyTemporaries(bool bKeep, bool bWide)
{
	SCPP65Pending*	psPending;
	SCPP65Place		sPlace;
	size			i;

	if (macPendingTemporaries.NumElements() == 0)
	{
		return;
	}
	if (bKeep)
	{
		PushWide("A", bWide);
	}
	for (i = macPendingTemporaries.NumElements(); i > 0; i--)
	{
		psPending = macPendingTemporaries.Get(i - 1);
		sPlace.eKind = CPP65PK_Frame;
		sPlace.psType = psPending->psType;
		sPlace.iOffset = psPending->iOffset;
		sPlace.lliName = -1;
		sPlace.iString = -1;
		sPlace.bInternal = false;
		sPlace.psClass = NULL;
		Destroy(&sPlace);
	}
	macPendingTemporaries.ReInit();
	if (bKeep)
	{
		PullWide("A", bWide);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the class declares a constructor that takes one of its objects (a copy
// constructor).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::HasCopyConstructor(SCPP65Class* psClass)
{
	return GetCopyConstructor(psClass) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class's copy constructor (taking one of its objects by reference, any other
// parameters having defaults), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::GetCopyConstructor(SCPP65Class* psClass)
{
	SCPP65Function*	psFunction;
	SCPP65Type*		psParameter;
	size			i;

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || (psFunction->eKind != CPP65FK_Constructor) || psFunction->bImplicit || (psFunction->mapsParameters.NumElements() == 0))
		{
			continue;
		}
		psParameter = psFunction->mapsParameters.GetPtr(0);
		if (psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->psClass == psClass) && (psFunction->mapsParameters.NumElements() == 1))
		{
			return psFunction;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if copying the class's objects copies their bytes: neither it nor its bases
// and fields have copy constructors.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsTriviallyCopyable(SCPP65Class* psClass)
{
	SCPP65Type*		psType;
	size			i;

	if (HasCopyConstructor(psClass))
	{
		return false;
	}
	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		if (!IsTriviallyCopyable(psClass->macBases.Get(i)->psClass))
		{
			return false;
		}
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psType = psClass->macFields.Get(i)->psType;
		psType = psType->IsArray() ? psType->psBase : psType;
		if (psType->IsClass() && !IsTriviallyCopyable(psType->psClass))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copy constructs the object at psPlace from pcSource (of its class or one derived
// from it): with the class's copy constructor, or bytes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CopyConstruct(SCPP65Place* psPlace, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Class*	psClass;
	CArraySyntax	apcArguments;
	bool			bResult;

	psClass = psPlace->psType->psClass;
	if (HasCopyConstructor(psClass))
	{
		apcArguments.Init();
		apcArguments.Add(pcSource);
		bResult = ConstructClass(psPlace, psClass, &apcArguments, pcNode);
		apcArguments.Kill();
		return bResult;
	}
	if (!IsTriviallyCopyable(psClass))
	{
		return CopyMembers(psPlace, pcSource, pcNode);
	}
	return CopyObject(psPlace, pcSource, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// The implicit copy constructor of a class whose bases or fields have copy
// constructors: the bytes are copied, then those bases and fields are copied again
// with their copy constructors.  Both addresses wait on the stack.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CopyMembers(SCPP65Place* psPlace, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Place		sSource;
	SCPP65Class*	psClass;
	int				iDestinationDepth;
	int				iSourceDepth;
	int				iOffset;
	int				iChunk;

	psClass = psPlace->psType->psClass;
	iDestinationDepth = -1;
	if (psPlace->eKind == CPP65PK_Address)
	{
		Push("A");
		iDestinationDepth = miDepth;
	}
	if (!Place(pcSource, &sSource))
	{
		return false;
	}
	LoadAddress(&sSource);
	Push("A");
	iSourceDepth = miDepth;
	if (iDestinationDepth == -1)
	{
		LoadAddress(psPlace);
		Push("A");
		iDestinationDepth = miDepth;
	}

	Line("cpy B, [")->Append(1 + miDepth - iSourceDepth)->Append(" + S]")->AppendNewLine();
	Line("cpy A, [")->Append(1 + miDepth - iDestinationDepth)->Append(" + S]")->AppendNewLine();
	for (iOffset = 0; iOffset < psClass->psType->iBytes; iOffset += iChunk)
	{
		iChunk = (psClass->psType->iBytes - iOffset >= 4) ? 4 : ((psClass->psType->iBytes - iOffset >= 2) ? 2 : 1);
		if (iChunk < 4)
		{
			Line("wth Y, ")->Append(iChunk)->AppendNewLine();
		}
		Line("cpy Y, [")->Append(iOffset)->Append(" + B]")->AppendNewLine();
		Line("cpy [")->Append(iOffset)->Append(" + A], Y")->AppendNewLine();
		if (iChunk < 4)
		{
			Line("wth Y, 4")->AppendNewLine();
		}
	}
	if (IsPolymorphic(psClass))
	{
		StoreVtables(psClass, "Y", "A");
	}

	CopyParts(psClass, 0, iDestinationDepth, iSourceDepth);
	Line("add S, 8")->AppendNewLine();
	miDepth -= 8;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies the bases and fields of the class (at iOffset in the objects whose
// addresses are on the stack) that have copy constructors with them.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::CopyParts(SCPP65Class* psClass, int iOffset, int iDestinationDepth, int iSourceDepth)
{
	SCPP65Base*		psBase;
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;
	int				j;
	int				iCount;

	for (i = 0; i < psClass->macBases.NumElements(); i++)
	{
		psBase = psClass->macBases.Get(i);
		CopyPart(psBase->psClass, iOffset + psBase->iOffset, iDestinationDepth, iSourceDepth);
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		psType = psField->psType;
		iCount = 1;
		if (psType->IsArray())
		{
			iCount = psType->iCount;
			psType = psType->psBase;
		}
		if (!psType->IsClass())
		{
			continue;
		}
		for (j = 0; j < iCount; j++)
		{
			CopyPart(psType->psClass, iOffset + psField->iOffset + j * psType->iBytes, iDestinationDepth, iSourceDepth);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A part (base or field) of class psClass at iOffset: copied with its copy
// constructor, or its own parts, or left as its bytes.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::CopyPart(SCPP65Class* psClass, int iOffset, int iDestinationDepth, int iSourceDepth)
{
	SCPP65Function*	psConstructor;

	psConstructor = GetCopyConstructor(psClass);
	if (psConstructor)
	{
		Line("cpy A, [")->Append(1 + miDepth - iSourceDepth)->Append(" + S]")->AppendNewLine();
		if (iOffset != 0)
		{
			Line("add A, ")->Append(iOffset)->AppendNewLine();
		}
		Push("A");
		Line("cpy X, [")->Append(1 + miDepth - iDestinationDepth)->Append(" + S]")->AppendNewLine();
		if (iOffset != 0)
		{
			Line("add X, ")->Append(iOffset)->AppendNewLine();
		}
		ChangeX();
		Call(psConstructor, false, 4);
	}
	else if (!IsTriviallyCopyable(psClass))
	{
		CopyParts(psClass, iOffset, iDestinationDepth, iSourceDepth);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The place of a function's hidden result address, in its arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::LoadResultAddress(void)
{
	Line("cpy A, [")->Append(1 + miFrameBytes + 8)->Append(" + V]")->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// return value; in a function that returns an object: the value is constructed at
// the hidden result address, which is returned in A.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ReturnObject(CCPPSyntax* pcValue, CCPPSyntax* pcNode)
{
	SCPP65Place		sPlace;

	if (!pcValue)
	{
		return Error(pcNode, "The function must return a value");
	}
	LoadResultAddress();
	sPlace.eKind = CPP65PK_Address;
	sPlace.psType = mpsFunction->psReturn;
	sPlace.iOffset = 0;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.bInternal = false;
	sPlace.psClass = NULL;
	meNextElide = CPP65E_Result;
	if (!Construct(&sPlace, NULL, pcValue, pcNode))
	{
		meNextElide = CPP65E_None;
		return false;
	}
	meNextElide = CPP65E_None;
	LoadResultAddress();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Class(arguments), Class{arguments} and Class() as a value: the object is made in
// its temporary, whose address is left in A.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::TemporaryObject(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	SCPP65Place		sPlace;
	SCPP65Slot*		psSlot;
	CArraySyntax	apcNone;
	bool			bResult;

	if (!CompleteClass(psType->psClass, pcNode))
	{
		return NULL;
	}
	psSlot = mpsFunction ? mcTemporarySlots.Get((int64)(size)pcNode) : NULL;
	sPlace.psType = psType;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.bInternal = false;
	sPlace.psClass = NULL;
	if (psSlot)
	{
		//In the frame: constructed in place, then its address.
		sPlace.eKind = CPP65PK_Frame;
		sPlace.iOffset = psSlot->iOffset;
		apcNone.Init();
		bResult = Construct(&sPlace, pcInitialiser ? NULL : &apcNone, pcInitialiser, pcNode);
		apcNone.Kill();
		if (!bResult)
		{
			return NULL;
		}
		return TemporaryAddress(pcNode, psType) ? psType : NULL;
	}

	if (!TemporaryAddress(pcNode, psType))
	{
		return NULL;
	}
	sPlace.eKind = CPP65PK_Address;
	sPlace.iOffset = 0;
	Push("A");
	apcNone.Init();
	bResult = Construct(&sPlace, pcInitialiser ? NULL : &apcNone, pcInitialiser, pcNode);
	apcNone.Kill();
	Pull("A");
	return bResult ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object of the class (a call, or an operator, that
// returns one): it can be made where the object it initialises is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsMadeObject(CCPPSyntax* pcExpression, SCPP65Class* psClass)
{
	SCPP65Type*	psType;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || !MakesObject(pcExpression))
	{
		return false;
	}
	psType = TypeOf(pcExpression);
	return psType && psType->IsClass() && (psType->psClass == psClass);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is one that may make an object: a call, or an operator
// with an object operand (an operator call; *pointer and the rest are not).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MakesObject(CCPPSyntax* pcExpression)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;

	switch (pcExpression->GetKind())
	{
	case CPPS_CallExpression:
		return true;
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		return (pcBinary->meOperator != CT_Comma) && (pcBinary->meOperator != CT_Assignment) && (IsObject(pcBinary->mpcLeft) || IsObject(pcBinary->mpcRight));
	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		return IsObject(pcUnary->mpcOperand);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the object at psPlace with an object made by pcInitialiser, which is
// made there (as C++17 requires): its hidden result address is the place's.  An
// Address place is reached as eElide says.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConstructFromMade(SCPP65Place* psPlace, CCPPSyntax* pcInitialiser, ECPP65Elide eElide, int iDepth, CCPPSyntax* pcNode)
{
	CCPPSyntax*		pcSaved;
	SCPP65Place		sSaved;
	ECPP65Elide		eSaved;
	int				iSavedDepth;
	bool			bResult;

	pcSaved = mpcElide;
	sSaved = msElide;
	eSaved = meElide;
	iSavedDepth = miElideDepth;
	mpcElide = SkipParentheses(pcInitialiser);
	msElide = *psPlace;
	meElide = eElide;
	miElideDepth = iDepth;
	bResult = Expression(pcInitialiser) != NULL;
	if (mpcElide && (miTypeOf == 0))
	{
		bResult = Error(pcNode, "The object could not be made in place by the 65C64 generator");
	}
	mpcElide = pcSaved;
	msElide = sSaved;
	meElide = eSaved;
	miElideDepth = iSavedDepth;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an object (or a reference to one).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcExpression);
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
	}
	return psType && psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// left op right, or op operand when pcRight is NULL, with an object operand: the
// left's member operator, or a free operator function.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::OperatorExpression(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode)
{
	SCPP65Type*			psType;
	SCPP65Function*		psFunction;
	CArraySyntax		apcArguments;
	int64				lliName;
	int					iBytes;
	size				uiErrors;

	lliName = GetOperatorName(eOperator, false);
	psType = TypeOf(pcLeft);
	if (psType && psType->IsReference())
	{
		psType = psType->psBase;
	}
	apcArguments.Init();
	if (psType && psType->IsClass() && CompleteClass(psType->psClass, pcNode) && HasMember(psType->psClass, lliName))
	{
		if (pcRight)
		{
			apcArguments.Add(pcRight);
		}
		uiErrors = mcErrors.NumErrors();
		BeginQuiet();
		psFunction = FindFunction(psType->psClass, lliName, CPP65FK_Function, &apcArguments, true, pcNode);
		EndQuiet();
		mcErrors.Truncate(uiErrors);
		if (psFunction)
		{
			psType = OperatorCall(pcLeft, eOperator, &apcArguments, pcNode);
			apcArguments.Kill();
			return psType;
		}
		apcArguments.ReInit();
	}

	//operator op(left, right), declared or from a function template.
	apcArguments.Add(pcLeft);
	if (pcRight)
	{
		apcArguments.Add(pcRight);
	}
	psFunction = FindFreeOperator(lliName, &apcArguments, pcNode);
	if (!psFunction)
	{
		Error(pcNode, "No operator matches the operands");
		apcArguments.Kill();
		return NULL;
	}
	iBytes = PushArguments(psFunction, &apcArguments, pcNode);
	apcArguments.Kill();
	if (iBytes < 0)
	{
		return NULL;
	}
	Call(psFunction, false, iBytes);
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator function (not a member) for the arguments: a declared one, or an
// instance of a function template of that name.  NULL, quietly, if none matches.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindFreeOperator(int64 lliName, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Function*					psFunction;
	SCPP65Template*					psTemplate;
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxName*					pcName;
	size							uiErrors;
	size							i;
	bool							bDeclared;

	bDeclared = false;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if (!psFunction->psClass && (psFunction->lliName == lliName) && !psFunction->psTemplate)
		{
			bDeclared = true;
		}
	}

	uiErrors = mcErrors.NumErrors();
	psFunction = NULL;
	BeginQuiet();
	if (bDeclared)
	{
		psFunction = FindFunction(NULL, lliName, CPP65FK_Function, papcArguments, false, pcNode);
	}
	for (i = 0; !psFunction && (i < mapsTemplates.NumElements()); i++)
	{
		psTemplate = mapsTemplates.GetPtr(i);
		if (psTemplate->bMember || !psTemplate->pcDefinition || (psTemplate->pcDefinition->GetKind() != CPPS_FunctionDefinition))
		{
			continue;
		}
		pcDefinition = (CCPPSyntaxFunctionDefinition*)psTemplate->pcDefinition;
		pcName = pcDefinition->mpcDeclarator ? pcDefinition->mpcDeclarator->GetName() : NULL;
		if (pcName && (pcName->macpComponents.NumElements() == 1) && (GetFunctionName(pcName->GetLast()) == lliName))
		{
			psFunction = InstantiateFunction(psTemplate, NULL, papcArguments, pcNode);
		}
	}
	EndQuiet();
	mcErrors.Truncate(uiErrors);
	return psFunction;
}

