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
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPPUnit.h"
#include "CPPGlobals.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit's diagnostics, declarators, context, types and data, asked for
// once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPGlobals::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mpcDeclarators = pcUnit->GetDeclarators();
	mpcContext = pcUnit->GetContext();
	mpcTypes = pcUnit->GetTypes();
	mpcData = pcUnit->GetData();
	mapcStaticLocals.Init();
	miStatics = 0;
	masExitFinalisers.Init();
	mbExitLists = false;
	mbExitListsWritten = false;
	mbDynamicCast = false;
	mbDynamicCastWritten = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPGlobals::Kill(void)
{
	CCPPUnitData::KillRoutines(&masExitFinalisers);
	masExitFinalisers.Kill();
	mpcUnit->FreeVariables(&mapcStaticLocals);
	mapcStaticLocals.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// What a closure's function finds a static local of the function it is in
// by (CCPPClosures::EnclosingStatic), and what the unit uses: R_RunExitLists
// and R_DynamicCast are written once the unit's functions are, if anything
// uses them.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPPVariables* CCPPGlobals::GetStaticLocals(void) { return &mapcStaticLocals; }
void CCPPGlobals::UseExitLists(void) { mbExitLists = true; }
void CCPPGlobals::UseDynamicCast(void) { mbDynamicCast = true; }


//////////////////////////////////////////////////////////////////////////
//
// Function declarations, classes, typedefs, enums and global variables.  Global
// variables are data with constant initialisers (zero without one).  Global objects
// are zero and made by their R_Initialise before main, and destroyed (last first) by
// their R_Finalise after it.  A qualified name defines a static data member.  Function
// and extern declarations the compiler cannot do are only errors when used.
// pcDefining is the variable being defined when the declaration is a
// template's, written once and defined for each instance (a class
// template's static member, a variable template): NULL for any other.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration, CCPPVariable* pcDefining)
{
	size						i;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntax*					pcInitialiser;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPVariable*				pcVariable;
	CCPPFunction*				pcFunction;
	SCPP65Type*					psType;
	SCPP65Type*					psElement;
	CChars						szLabel;
	CCPPSyntaxName*				pcName;
	CCPPClass*				pcClass;
	int64						lliName;
	bool						bResult;
	CCPPErrors*					pcErrors;
	bool						bBss;
	bool						bInitialise;
	CChars						szWritten;
	CChars::Count				iDataLength;
	bool						bConstantData;
	const char*					szOptions;
	CChars						szBound;
	int							iBound;
	ECPPBoundTemporary			eBound;
	bool						bThread;
	bool						bDestroyed;

	pcSpecifiers = pcDeclaration->mpcSpecifiers;
	if (pcSpecifiers && !mpcUnit->CheckEnumerators(pcSpecifiers->mpcType))
	{
		return false;
	}
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return mpcUnit->CheckTypedefs(pcDeclaration);
	}
	if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
	{
		//The class is laid out when it is first needed whole.
		mpcDiagnostics->BeginQuiet();
		mpcDeclarators->GetBaseType(pcSpecifiers, pcDeclaration);
		mpcDiagnostics->EndQuiet();
		if (!mpcUnit->ClassStaticAsserts((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType) || !mpcUnit->GetClasses()->CheckVirtSpecifiers((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType))
		{
			return false;
		}
		if ((pcDeclaration->macpDeclarators.NumElements() == 0) && !((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType)->mpcName)
		{
			return AnonymousGlobal((CCPPSyntaxClassSpecifier*)pcSpecifiers->mpcType);
		}
	}

	bResult = true;
	for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
		if (pcInit->mpcDeclarator->IsFunction() || pcInit->mpcDeclarator->IsFunctionByType(pcSpecifiers))
		{
			pcFunction = mpcUnit->GetFunctionDeclarer()->DeclareQuietly(pcSpecifiers, pcInit->mpcDeclarator, NULL, CPPDA_Declaration, &pcErrors);
			if (!pcFunction && mpcUnit->GetFunctionDeclarer()->DeclaresAllocation(pcInit->mpcDeclarator))
			{
				//What is wrong with an operator new or delete is said where it is declared: a program replaces the
				//library's by declaring its own, and a wrong one would be called by none of its news.
				mpcDiagnostics->Report(pcErrors);
				bResult = false;
			}
			continue;
		}

		lliName = mpcUnit->GetName(pcInit->mpcDeclarator);
		pcName = pcInit->mpcDeclarator->GetName();
		pcVariable = pcInit->mpcSymbol ? (pcDefining ? pcDefining : mpcUnit->GetVariable(pcInit->mpcSymbol)) : NULL;
		if (!pcVariable && pcInit->mpcSymbol && !pcDefining)
		{
			//int x; after extern int x; is the same variable.
			pcVariable = RedeclaredVariable(pcInit->mpcSymbol);
		}
		if ((lliName == -1) && pcName && pcName->IsQualifiedByClass())
		{
			//int CClass::siStatic = 0; defines a static data member, so the class is laid out.  The member is the
			//class's own: each instance of a class template has one of the symbol (template<> int S<int>::k = 5;).
			pcClass = pcDefining ? pcDefining->mpcClass : mpcUnit->GetNames()->GetComponentClass(pcName->GetQualifier(), NULL, pcInit);
			if (pcClass)
			{
				mpcUnit->GetClasses()->CompleteClass(pcClass, pcInit);
				pcVariable = (pcInit->mpcSymbol && !pcDefining) ? mpcUnit->GetInstantiator()->OwnStatic(pcClass, pcInit->mpcSymbol) : pcVariable;
			}
		}
		if (!pcInit->mpcSymbol || ((lliName == -1) && pcName && pcName->IsQualifiedByClass() && !(pcVariable && pcVariable->mpcClass)))
		{
			bResult &= mpcDiagnostics->Error(pcInit, "Only variables with simple names are supported by the compiler yet");
			continue;
		}
		bResult &= mpcUnit->GetExceptions()->CheckPointerExceptions(pcInit);

		//extern without an initialiser only declares; extern int x = 1; defines x.  [basic.def]
		if (((pcSpecifiers->muiFlags & CPPDS_EXTERN) || mpcContext->IsLinkageExtern()) && !pcInit->mpcInitializer)
		{
			mpcDiagnostics->BeginQuiet();
			psType = mpcDeclarators->GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			mpcDiagnostics->EndQuiet();
			if (psType && pcVariable && !CheckRedeclaredType(pcVariable->mpsType, psType, pcInit))
			{
				bResult = false;
				continue;
			}
			if (psType && !pcVariable)
			{
				pcVariable = mpcUnit->AddVariable(mpcUnit->GetGlobalVariables(), pcInit->mpcSymbol, psType, CPPVS_Global, 0);
			}
			if (pcVariable)
			{
				pcVariable->mbRestrict |= mpcUnit->IsRestrictPointer(pcInit->mpcDeclarator);
			}
			continue;
		}

		//const int CClass::k; has the initialiser the class gave it, and const int CClass::k = 5; is what k is from here.
		pcInitialiser = pcInit->mpcInitializer;
		if (!pcInitialiser && pcVariable)
		{
			pcInitialiser = pcVariable->mpcInClassInitialiser;
		}
		else if (pcVariable && pcVariable->mpcClass)
		{
			pcVariable->mpcOutsideInitialiser = pcInitialiser;
		}

		psType = mpcDeclarators->GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInitialiser);
		if (!psType)
		{
			bResult = false;
			continue;
		}
		if (psType->IsReference())
		{
			pcInitialiser = mpcUnit->GetBinder()->BoundInitialiser(psType, pcInitialiser);
		}
		psElement = psType->ElementType();
		if (psElement->IsClass() && !mpcUnit->GetClasses()->CompleteObjectClass(psElement->pcClass, pcInit))
		{
			bResult = false;
			continue;
		}

		if (pcVariable && !pcVariable->mpcClass && !CheckRedeclaredType(pcVariable->mpsType, psType, pcInit))
		{
			bResult = false;
			continue;
		}
		if (!pcVariable)
		{
			pcVariable = mpcUnit->AddVariable(mpcUnit->GetGlobalVariables(), pcInit->mpcSymbol, psType, CPPVS_Global, 0);
		}
		pcVariable->mbInternal |= !pcVariable->mpcClass && ((pcSpecifiers->muiFlags & CPPDS_STATIC) != 0);
		pcVariable->mbRestrict |= mpcUnit->IsRestrictPointer(pcInit->mpcDeclarator);
		if (pcVariable->mbDefined)
		{
			bResult &= mpcDiagnostics->Error(pcInit, "The variable is defined twice");
			continue;
		}
		pcVariable->mbDefined = true;
		pcVariable->mpsType = psType;
		CompleteDeclaredArrays(pcVariable);
		if (!mpcUnit->CheckDeclared(psType, pcInit, pcInitialiser))
		{
			bResult = false;
			continue;
		}
		if ((pcSpecifiers->muiFlags & CPPDS_CONSTEXPR) && !mpcUnit->GetConstants()->CheckVariable(pcVariable, pcInit))
		{
			bResult = false;
			continue;
		}

		szLabel.Init();
		mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
		bConstantData = (pcSpecifiers->muiFlags & CPPDS_CONSTEXPR) != 0;
		bBss = !bConstantData && IsBss(pcInitialiser, psType);
		szOptions = pcDefining ? ", comdat" : (mpcUnit->HasUsedAttribute(pcInit->mpcDeclarator) ? ", keep" : NULL);
		szBound.Init();
		eBound = psType->IsReference() ? BoundTemporary(psType, pcInitialiser, &szLabel, !pcVariable->mbInternal, szOptions, &szBound, &iBound) : CPPBT_None;
		if (eBound == CPPBT_Error)
		{
			bResult = false;
			szBound.Kill();
			szLabel.Kill();
			continue;
		}
		bResult &= eBound != CPPBT_BoundWithError;
		mpcData->BeginData(&szLabel, VariableDataAlignment(pcVariable, psElement->IsClass()), !pcVariable->mbInternal, szOptions, bBss);
		if (mpcUnit->HasLineTable())
		{
			mpcUnit->GetDebugWriter()->AppendGlobal(mpcData->GetText(), pcVariable, szLabel.Text(), false);
		}

		//GlobalInitialiser writes the constant data.  What isn't constant is worked out by the global's own R_Initialise,
		//and an object is destroyed by its R_Finalise: the IR front end writes them (GlobalStartUpRoutines).
		bThread = (pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) != 0;
		bDestroyed = psElement->IsClass() && (psElement->pcClass->mpcDestructor != NULL);
		mpcContext->EnterNoThis();
		if (bConstantData && mpcUnit->GetConstants()->AppendData(pcVariable, mpcData))
		{
			//A constexpr variable's value, worked out when it was defined: its data, with nothing to run.
		}
		else if (eBound != CPPBT_None)
		{
			//A reference to the temporary it is bound to (BoundTemporary).
			mpcData->AppendAddress(&szBound, iBound, psType->iBytes);
		}
		else if (bBss)
		{
			mpcData->AppendSpace(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, psElement->IsClass(), bDestroyed, bThread);
		}
		else if (!psElement->IsClass() && !psType->IsReference())
		{
			bInitialise = false;
			iDataLength = mpcData->GetText()->Length();
			bResult &= GlobalInitialiser(psType, pcInitialiser, pcInit, &bInitialise);
			if (bInitialise && bResult && !bThread && psElement->IsPointer())
			{
				//A pointer to an object of static storage, or a table of them, is a constant the interpreter works out.
				szWritten.Init();
				szWritten.AppendSubString(mpcData->GetText(), iDataLength, mpcData->GetText()->Length());
				mpcData->GetText()->SetLength(iDataLength);
				bInitialise = !mpcUnit->GetConstants()->AppendConstantInitialisation(pcVariable, mpcData);
				if (bInitialise)
				{
					mpcData->GetText()->Append(&szWritten);
				}
				szWritten.Kill();
			}
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, bInitialise, false, false);
		}
		else if (!bThread && mpcUnit->GetConstants()->AppendConstantInitialisation(pcVariable, mpcData))
		{
			//An object made by a constexpr constructor from constants, or a reference bound to an object of static
			//storage: initialised before anything runs, as data, though not constexpr.  [basic.start.init] 2
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, false, bDestroyed, false);
		}
		else
		{
			mpcData->AppendZeros(psType->iBytes);
			bResult &= GlobalStartUpRoutines(psType, pcInitialiser, &szLabel, true, bDestroyed, bThread);
		}
		mpcContext->LeaveNoThis();
		mpcData->NextStartUp();
		szBound.Kill();
		szLabel.Kill();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the data for a global of psType: zero, a constant, a list of constants or
// a string.  A char pointer initialised with a string literal is the literal's
// address.  A value that is not a constant is zero data, worked out by the global's
// R_Initialise: *pbInitialise is set when there is one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, bool* pbInitialise)
{
	int64						lliValue;
	CArraySyntax*				papcElements;
	size						i;
	CArrayUint8					auiBytes;
	bool						bResult;
	double						dValue;

	pcInitialiser = mpcUnit->GetDesugarings()->BracedFor(psType, pcInitialiser);
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer))
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : pcInitialiser;
	}
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psType->IsScalar())
	{
		papcElements = mpcUnit->GetInstantiator()->ListElements(pcInitialiser);
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
		if (pcInitialiser && !mpcUnit->CheckBracedValue(pcInitialiser, psType))
		{
			return false;
		}
	}

	if (psType->IsArray())
	{
		bResult = true;
		pcInitialiser = mpcUnit->StringInitialiser(pcInitialiser, psType->psBase);
		if (!pcInitialiser)
		{
			i = 0;
		}
		else if (mpcUnit->IsStringLiteral(pcInitialiser) && (psType->psBase->iBytes == mpcUnit->GetStrings()->StringElementBytes(pcInitialiser)))
		{
			auiBytes.Init();
			bResult = mpcUnit->GetStrings()->DecodeString(pcInitialiser, &auiBytes);
			for (i = 0; i < auiBytes.NumElements() / psType->psBase->iBytes; i++)
			{
				mpcData->AppendData(psType->psBase->iBytes, mpcUnit->GetStrings()->StringElement(&auiBytes, psType->psBase->iBytes, i));
			}
			auiBytes.Kill();
			if (i > (size)psType->iCount)
			{
				return mpcDiagnostics->Error(pcInitialiser, "The string is longer than the array");
			}
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcElements = mpcUnit->GetInstantiator()->ListElements(pcInitialiser);
			if (papcElements->NumElements() > (size)psType->iCount)
			{
				return mpcDiagnostics->Error(pcInitialiser, "Too many initialisers for the array");
			}
			for (i = 0; i < papcElements->NumElements(); i++)
			{
				bResult &= GlobalInitialiser(psType->psBase, papcElements->GetPtr(i), pcNode, pbInitialise);
			}
		}
		else
		{
			return mpcDiagnostics->Error(pcInitialiser, "An array needs a list of initialisers");
		}

		for (; i < (size)psType->iCount; i++)
		{
			GlobalInitialiser(psType->psBase, NULL, pcNode, pbInitialise);
		}
		return bResult;
	}

	if (!psType->IsScalar())
	{
		return mpcDiagnostics->Error(pcNode, "This type of global variable is not supported by the compiler yet");
	}
	lliValue = 0;
	if (pcInitialiser && psType->IsFloat() && mpcUnit->GetFolder()->FloatConstant(pcInitialiser, &dValue))
	{
		mpcData->AppendData(psType->iBytes, mpcUnit->GetFolder()->FloatBits(psType, dValue));
		return true;
	}
	if (pcInitialiser && psType->IsPointer() && mpcUnit->IsStringLiteral(pcInitialiser) && (psType->psBase->iBytes == mpcUnit->GetStrings()->StringElementBytes(pcInitialiser)))
	{
		auiBytes.Init();
		bResult = mpcUnit->GetStrings()->DecodeString(pcInitialiser, &auiBytes);
		if (bResult)
		{
			//AddString ends it with one zero byte.
			auiBytes.GrowToAtLeastNumElements(auiBytes.NumElements() + psType->psBase->iBytes - 1, true, 0);
			mpcData->GetText()->Append("\t\t.long Str")->Append(mpcData->AddString((char*)auiBytes.GetData(), auiBytes.NumElements()))->AppendNewLine();
		}
		auiBytes.Kill();
		return bResult;
	}
	if (pcInitialiser && (psType->IsFloat() || !mpcUnit->GetFolder()->Constant(pcInitialiser, &lliValue)))
	{
		//Not a constant: R_Initialise works it out before main, or for a static local its declaration the first time it
		//is reached.  [basic.start.init], [stmt.dcl] 4
		mpcData->AppendData(psType->iBytes, 0);
		if (!mpcUnit->GetTypeChecker()->ImplicitConvert(mpcUnit->GetTypeChecker()->ValueFor(pcInitialiser, psType, false), psType, pcInitialiser))
		{
			return false;
		}
		*pbInitialise = true;
		return true;
	}
	if (pcInitialiser)
	{
		mpcUnit->GetWarner()->ConversionOverflow(pcInitialiser, psType);
	}
	mpcData->AppendData(psType->iBytes, lliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An anonymous union at namespace scope: one object, zero, whose label is
// its first member's, and a global variable for each member at its offset
// in it, internal as the union is (static or in an unnamed namespace).
// [class.union] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::AnonymousGlobal(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	CCPPClass*		pcClass;
	SCPPField*		psField;
	CCPPVariable*	pcVariable;
	CCPPVariable*	pcObject;
	CChars			szLabel;
	size			i;

	pcClass = mpcUnit->GetClasses()->GetUnnamedClass(pcSpecifier);
	if (!pcClass || !mpcUnit->GetClasses()->CompleteClass(pcClass, pcSpecifier) || !pcClass->mbUnion)
	{
		return mpcDiagnostics->Error(pcSpecifier, "This anonymous class is not supported by the compiler yet");
	}
	pcObject = NULL;
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		if (!psField->pcSymbol || psField->IsBitField())
		{
			return mpcDiagnostics->Error(pcSpecifier, "This anonymous union's member is not supported by the compiler yet");
		}
		pcVariable = mpcUnit->AddVariable(mpcUnit->GetGlobalVariables(), psField->pcSymbol, psField->psType, CPPVS_Global, psField->iOffset);
		pcVariable->mpcAnonymous = pcClass;
		pcVariable->mbInternal = true;
		pcVariable->mbDefined = true;
		pcObject = pcObject ? pcObject : pcVariable;
		pcVariable->mpcAnonymousObject = pcObject;
	}
	if (!pcObject)
	{
		return true;
	}
	szLabel.Init();
	mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcObject);
	mpcData->BeginData(&szLabel, DataAlignment(pcClass->mpsType, true), false, NULL, IsBss(NULL, pcClass->mpsType));
	if (IsBss(NULL, pcClass->mpsType))
	{
		mpcData->AppendSpace(pcClass->mpsType->iBytes);
	}
	else
	{
		mpcData->AppendZeros(pcClass->mpsType->iBytes);
	}
	mpcData->NextStartUp();
	szLabel.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable an earlier declaration of a namespace scope variable's name
// in its scope made: extern int x; int x; is one variable, which can be
// defined once.  NULL if there is none, and in C, where int x; int x; are
// tentative definitions of one.  [basic.def.odr] 1, [basic.link] 9
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPGlobals::RedeclaredVariable(CCPPSymbol* pcSymbol)
{
	CCPPSymbol*		pcOther;
	CCPPVariable*	pcVariable;

	if (mpcUnit->IsCSource() || (pcSymbol->meKind != CPPSK_Variable) || !pcSymbol->mpcScope || (pcSymbol->mpcScope->meKind != CPPSC_Namespace))
	{
		return NULL;
	}
	for (pcOther = pcSymbol->mpcScope->FindHere(pcSymbol->mlliName); pcOther; pcOther = pcOther->mpcNext)
	{
		if ((pcOther != pcSymbol) && (pcOther->mpcScope == pcSymbol->mpcScope) && (pcOther->meKind == CPPSK_Variable))
		{
			pcVariable = mpcUnit->GetVariable(pcOther);
			if (pcVariable && (pcVariable->meStorage == CPPVS_Global) && !pcVariable->mpcClass)
			{
				return pcVariable;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable declared again is declared with its type (SameVariableType):
// an error at pcNode (and false) if not.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::CheckRedeclaredType(SCPP65Type* psDeclared, SCPP65Type* psType, CCPPSyntax* pcNode)
{
	return SameVariableType(psDeclared, psType) || mpcDiagnostics->Error(pcNode, "The variable is declared again with another type");
}


//////////////////////////////////////////////////////////////////////////
//
// Whether two declarations of a variable give it one type: the same, but
// for an array's bound, which one may leave out.  [basic.link] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::SameVariableType(SCPP65Type* psFirst, SCPP65Type* psSecond)
{
	if (!psFirst || !psSecond || psFirst->Same(psSecond))
	{
		return true;
	}
	if (psFirst->IsArray() && psSecond->IsArray())
	{
		return psFirst->psBase->Same(psSecond->psBase);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// extern int a[]; declared an array of unknown bound that the definition
// of pcDefined, int a[3], has now completed: the earlier declaration's
// variable, which what was written before the definition names, is of the
// same array.  [dcl.array] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPGlobals::CompleteDeclaredArrays(CCPPVariable* pcDefined)
{
	CCPPSymbol*		pcSymbol;
	CCPPVariable*	pcDeclared;

	if (!pcDefined->mpsType->IsArray() || !pcDefined->mpcSymbol->mpcScope)
	{
		return;
	}
	for (pcSymbol = pcDefined->mpcSymbol->mpcScope->FindHere(pcDefined->mpcSymbol->mlliName); pcSymbol; pcSymbol = pcSymbol->mpcNext)
	{
		pcDeclared = (pcSymbol != pcDefined->mpcSymbol) ? mpcUnit->GetVariable(pcSymbol) : NULL;
		if (pcDeclared && pcDeclared->mpsType->IsArray() && (pcDeclared->mpsType->iCount == 0) && pcDeclared->mpsType->psBase->Same(pcDefined->mpsType->psBase))
		{
			pcDeclared->mpsType = pcDefined->mpsType;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A global reference (pszLabel's) that pcInitialiser binds to a temporary
// (const long& r = 7, BindsToTemporary): the temporary is a global of its
// own, pszBound (pszLabel and __Bound), made from the initialiser and
// destroyed as a global of its type is.  Its data goes before the
// reference's, which is its address, or its base's *piOffset into it.
// What it did (ECPPBoundTemporary): nothing, for any other initialiser;
// bound it, whose start-up routines may have said an error; or nothing,
// after an error.  [class.temporary] 6
//
//////////////////////////////////////////////////////////////////////////
ECPPBoundTemporary CCPPGlobals::BoundTemporary(SCPP65Type* psReference, CCPPSyntax* pcInitialiser, CChars* pszLabel, bool bGlobal, const char* szOptions, CChars* pszBound, int* piOffset)
{
	SCPP65Type*	psType;
	CCPPSyntax*	pcValue;
	bool		bClass;
	bool		bRoutines;

	pcValue = pcInitialiser;
	if (pcValue && (pcValue->GetKind() == CPPS_ParenthesisedInitializer) && (mpcUnit->GetInstantiator()->ListElements(pcValue)->NumElements() == 1))
	{
		pcValue = mpcUnit->GetInstantiator()->ListElements(pcValue)->GetPtr(0);
	}
	if (!pcValue || (pcValue->GetKind() == CPPS_InitializerList))
	{
		return CPPBT_None;
	}
	psType = psReference->psBase->Unqualified();
	if (!mpcUnit->GetBinder()->BindsToTemporary(psReference, pcValue))
	{
		//const S& r = make() and const S& r = D(): the object made for the reference, which may be of a class derived from S.
		psType = mpcUnit->GetTypeChecker()->TypeOf(pcValue);
		if (!psReference->psBase->IsClass() || !psType || !psType->IsClass() || mpcUnit->GetTypeChecker()->IsObjectLvalue(pcValue))
		{
			return CPPBT_None;
		}
	}

	bClass = psType->IsClass();
	if (bClass && !mpcUnit->GetClasses()->CompleteObjectClass(psType->pcClass, pcInitialiser))
	{
		return CPPBT_Error;
	}
	*piOffset = 0;
	if (bClass && (psType->pcClass != psReference->psBase->pcClass))
	{
		mpcUnit->GetClasses()->CompleteOffset(psType->pcClass, psReference->psBase->pcClass, piOffset);
	}
	mpcUnit->GetMangler()->AppendBoundLabel(pszBound, pszLabel->Text());
	mpcData->BeginData(pszBound, DataAlignment(psType, bClass), bGlobal, szOptions, false);
	mpcData->AppendZeros(psType->iBytes);
	bRoutines = GlobalStartUpRoutines(psType, pcInitialiser, pszBound, true, bClass && (psType->pcClass->mpcDestructor != NULL), false);
	mpcData->NextStartUp();
	return bRoutines ? CPPBT_Bound : CPPBT_BoundWithError;
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise and R_Finalise are written by the IR front end (CCPP65IRFrontEnd::GlobalRoutine).  bConstruct when it needs constructing, bDestroy when it needs
// destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::GlobalStartUpRoutines(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, bool bConstruct, bool bDestroy, bool bThread)
{
	CChars	szFinaliser;
	CChars	szNode;
	bool	bResult;
	bool	bListed;

	//One made when the program starts is put on an exit list once it has been (bThread: a thread_local's), so objects
	//are destroyed in the reverse of the order they were made, with the atexit functions between them; one made in the
	//data, before any of those, is in the fini table, run after the lists.  [basic.start.term] 1, 3
	bListed = bConstruct && bDestroy;
	szFinaliser.Init();
	mpcUnit->GetMangler()->AppendStartUpLabel(&szFinaliser, true, mpcData->GetStartUp());
	szNode.Init();
	mpcUnit->GetMangler()->AppendExitNodeLabel(&szNode, pszData->Text());
	bResult = true;
	if (bConstruct)
	{
		//One on an exit list is put there by its R_Initialise.
		bResult &= StartUpRoutine(false, psType, pcInitialiser, pszData, bListed ? &szNode : NULL, &szFinaliser, true, bThread);
	}
	if (bDestroy)
	{
		//One on an exit list is destroyed from it; else from the fini table.
		bResult &= StartUpRoutine(true, psType, pcInitialiser, pszData, NULL, &szFinaliser, !bListed, bThread);
	}
	if (bListed)
	{
		mpcData->AppendExitNode(&szNode, pszData, ExitNodeBytes());
	}
	szNode.Kill();
	szFinaliser.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A global's R_Initialise, or with bFinalise its R_Finalise (written by
// the IR front end, CCPP65IRFrontEnd::GlobalRoutine), and with bEntry its
// entry in the init or fini table.  pszNode is the exit list node an
// R_Initialise puts it on, with pszFinaliser the routine that destroys it
// from there; NULL for none.  False after an error.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::StartUpRoutine(bool bFinalise, SCPP65Type* psType, CCPPSyntax* pcInitialiser, CChars* pszData, CChars* pszNode, CChars* pszFinaliser, bool bEntry, bool bThread)
{
	CChars	szLabel;
	bool	bResult;

	szLabel.Init();
	mpcUnit->GetMangler()->AppendStartUpLabel(&szLabel, bFinalise, mpcData->GetStartUp());
	CCPPUnitData::AppendSection(mpcUnit->GetCode(), "text", &szLabel, NULL);
	bResult = true;
	if (mpcUnit->GetIRFrontEnd()->GlobalRoutine(szLabel.Text(), pszData->Text(), psType, pcInitialiser, bFinalise, pszNode ? pszNode->Text() : NULL, pszFinaliser->Text(), bThread))
	{
		if (bEntry)
		{
			mpcData->AppendStartUpEntry(bFinalise ? "fini" : "init", &szLabel, pszData, mpcUnit->GetOptions()->mbKeepUnusedObjects);
		}
	}
	else
	{
		bResult = mpcDiagnostics->ErrorAt(pcInitialiser ? pcInitialiser->miFirstToken : -1, "This global's construction or destruction uses C++ the IR front end doesn't support yet");
	}
	szLabel.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A global or static local with no initialiser starts as zeros: on a
// target whose start up clears bss (CCPPTarget::HasZeroedBss) it goes
// there, and takes no space in the program's image.  An object is cleared
// there too, before its constructor runs from the init table.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::IsBss(CCPPSyntax* pcInitialiser, SCPP65Type* psType)
{
	return mpcUnit->GetTarget()->HasZeroedBss() && !pcInitialiser && !psType->IsReference() && (psType->iBytes > 0);
}


//////////////////////////////////////////////////////////////////////////
//
// 4 for objects and data of 4 bytes or more, 2 for 2 bytes, otherwise 1;
// less where the target aligns objects to less.
//
//////////////////////////////////////////////////////////////////////////
int CCPPGlobals::DataAlignment(SCPP65Type* psType, bool bClass)
{
	int		iAlign;

	iAlign = ((psType->iBytes >= 4) || bClass) ? 4 : ((psType->iBytes == 2) ? 2 : 1);
	return CCPPClassLayout::PackAlignment(iAlign, mpcUnit->GetTarget()->GetObjectAlignmentLimit());
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's data's alignment: DataAlignment, or more where its type or
// alignas asks it (VariableAlignment).
//
//////////////////////////////////////////////////////////////////////////
int CCPPGlobals::VariableDataAlignment(CCPPVariable* pcVariable, bool bClass)
{
	int		iAlign;
	int		iAlignas;
	int		iVariable;

	iAlign = DataAlignment(pcVariable->mpsType, bClass);
	iVariable = mpcUnit->GetClasses()->VariableAlignment(pcVariable, &iAlignas);
	return (iVariable > iAlign) ? iVariable : iAlign;
}


//////////////////////////////////////////////////////////////////////////
//
// extern int k; in a block declares the variable of the namespace it is in,
// defined elsewhere (before or after the function, or in another unit): a
// variable of the function's with the global's label.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::ExternLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	SCPP65Type*		psType;
	CCPPVariable*	pcGlobal;

	if (pcInit->mpcInitializer)
	{
		return mpcDiagnostics->Error(pcInit, "An extern variable declared in a block can't be initialised there");
	}
	psType = mpcDeclarators->GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, NULL);
	if (!psType)
	{
		return false;
	}
	//It names the variable of the namespace it is in, which has one type.  [basic.link] 6, 10
	pcGlobal = NamespaceVariable(pcInit->mpcSymbol);
	if (pcGlobal && !CheckRedeclaredType(pcGlobal->mpsType, psType, pcInit))
	{
		return false;
	}
	mpcUnit->AddVariable(mpcUnit->GetLocals(), pcInit->mpcSymbol, psType, CPPVS_Global, 0);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The variable of the innermost namespace a block's extern declaration is
// in that it names, if that namespace declares one.  [basic.link] 6
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPGlobals::NamespaceVariable(CCPPSymbol* pcSymbol)
{
	CCPPScope*		pcNamespace;
	CCPPSymbol*		pcOther;
	CCPPVariable*	pcVariable;

	pcNamespace = pcSymbol->mpcScope ? pcSymbol->mpcScope->GetEnclosingNamespace() : NULL;
	if (!pcNamespace)
	{
		return NULL;
	}
	for (pcOther = pcNamespace->FindHere(pcSymbol->mlliName); pcOther; pcOther = pcOther->mpcNext)
	{
		if ((pcOther->mpcScope == pcNamespace) && (pcOther->meKind == CPPSK_Variable))
		{
			pcVariable = mpcUnit->GetVariable(pcOther);
			if (pcVariable)
			{
				return pcVariable;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A static local variable is data with its own label, initialised in the
// data.  An object, a reference, or a variable whose initialiser isn't a
// constant is initialised the first time its declaration is reached
// (CCPP65IRFrontEnd::Declaration), with a guard byte beside it that says
// whether it has been (mbGuarded).  An object whose class needs a
// destructor is destroyed when the program ends: once made it is pushed
// on an exit list (the thread's for a thread_local), with the R_Finalise
// WriteExitRoutines writes.  [stmt.dcl] 4, [basic.start.term] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	CCPPVariable*		pcVariable;
	CCPPVariable*		pcKept;
	SCPP65Type*			psType;
	SCPP65Type*			psElement;
	CChars				szLabel;
	CChars				szNode;
	bool				bResult;
	bool				bInitialise;
	bool				bObject;
	CCPPFunction*		pcShared;
	const char*			szOptions;
	size				uiData;
	bool				bFirst;
	size				i;
	int					iNumber;

	psType = mpcDeclarators->GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	if (!psType)
	{
		return false;
	}
	psElement = psType->ElementType();
	bObject = psElement->IsClass() || psType->IsReference();
	if (psElement->IsClass() && !mpcUnit->GetClasses()->CompleteClass(psElement->pcClass, pcInit))
	{
		return false;
	}

	//In a function every unit may write it is one object: labelled for the function and its number there, its data COMDAT.
	pcShared = (mpcContext->GetFunction() && mpcContext->GetFunction()->IsShared()) ? mpcContext->GetFunction() : NULL;
	iNumber = miStatics;
	if (pcShared)
	{
		iNumber = 0;
		for (i = 0; i < mpcUnit->GetLocals()->NumElements(); i++)
		{
			iNumber += (mpcUnit->GetLocals()->GetPtr(i)->meStorage == CPPVS_Static) ? 1 : 0;
		}
	}
	szOptions = pcShared ? ", comdat" : NULL;
	pcVariable = mpcUnit->AddVariable(mpcUnit->GetLocals(), pcInit->mpcSymbol, psType, CPPVS_Static, iNumber);
	miStatics++;
	pcVariable->mbDefined = true;
	pcVariable->mbThreadLocal = (pcSpecifiers->muiFlags & CPPDS_THREAD_LOCAL) != 0;
	pcVariable->mpcSharedIn = pcShared;
	pcKept = mpcUnit->GetClosures()->EnclosingStatic(pcInit->mpcSymbol);
	if (!pcKept)
	{
		pcKept = mpcUnit->AddVariable(&mapcStaticLocals, pcInit->mpcSymbol, psType, CPPVS_Static, iNumber);
	}
	pcKept->miOffset = iNumber;
	pcKept->mpsType = psType;
	pcKept->mbDefined = true;
	pcKept->mbThreadLocal = pcVariable->mbThreadLocal;
	pcKept->mpcSharedIn = pcShared;
	szLabel.Init();
	mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
	//A function written again in the unit has its static locals again: a shared one's data is the unit's once.
	bFirst = !pcShared || mpcUnit->Emit(&szLabel);
	uiData = mpcData->GetText()->Length();
	mpcData->BeginData(&szLabel, VariableDataAlignment(pcVariable, psElement->IsClass()), pcShared != NULL, szOptions, bObject || IsBss(pcInit->mpcInitializer, psType));
	szLabel.Kill();
	bResult = true;
	bInitialise = false;
	if (bObject || IsBss(pcInit->mpcInitializer, psType))
	{
		mpcData->AppendSpace(psType->iBytes);
	}
	else
	{
		//Constant data if the initialiser is a constant; else it is worked out where the function first comes to it,
		//with the function's this.  [stmt.dcl] 4
		bResult = GlobalInitialiser(psType, pcInit->mpcInitializer, pcInit, &bInitialise);
	}

	pcVariable->mbGuarded = bResult && (bObject || bInitialise);
	if (pcVariable->mbGuarded)
	{
		szLabel.Init();
		mpcUnit->GetMangler()->AppendGuardLabel(&szLabel, pcVariable);
		mpcData->BeginData(&szLabel, 1, pcShared != NULL, szOptions, true);
		mpcData->AppendSpace(1);
		szLabel.Kill();
	}
	if (pcVariable->mbGuarded && psElement->IsClass() && psElement->pcClass->NeedsDestructor())
	{
		pcVariable->mbExitNode = true;
		szLabel.Init();
		mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
		if (bFirst)
		{
			CCPPUnitData::AddRoutine(&masExitFinalisers, szLabel.Text(), psType, pcShared != NULL);
		}
		szNode.Init();
		mpcUnit->GetMangler()->AppendExitNodeLabel(&szNode, szLabel.Text());
		mpcData->BeginData(&szNode, CPP65_EXIT_NODE_ALIGNMENT, pcShared != NULL, szOptions, true);
		mpcData->AppendSpace(ExitNodeBytes());
		szNode.Kill();
		szLabel.Kill();
		UseExitLists();
	}
	if (!bFirst)
	{
		mpcData->GetText()->SetLength(uiData);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The bytes of an exit list node: the next node and the routine that
// destroys its object, two pointers (CPP65_EXIT_NODE_ALIGNMENT aligned).
//
//////////////////////////////////////////////////////////////////////////
int CCPPGlobals::ExitNodeBytes(void)
{
	return 2 * mpcTypes->Pointer(&mpcTypes->msVoid)->iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Once the unit's functions are written: the R_Finalise of each static
// local destroyed at exit that they reach (StaticLocal), and the exit
// lists' runner if anything is pushed on them (R_RunExitLists, a COMDAT
// section any unit may write).  True if it wrote anything (which may
// queue destructors to generate).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::WriteExitRoutines(void)
{
	CChars				szFinaliser;
	CChars				szLabel;
	SCPPUnitRoutine*	psRoutine;
	size				i;
	bool				bWritten;

	bWritten = false;
	for (i = 0; i < masExitFinalisers.NumElements(); i++)
	{
		psRoutine = masExitFinalisers.Get(i);
		szFinaliser.Init();
		mpcUnit->GetMangler()->AppendStartUpLabel(&szFinaliser, true, psRoutine->szLabel.Text());
		CCPPUnitData::AppendRoutineSection(mpcUnit->GetCode(), &szFinaliser, psRoutine->bShared);
		if (!mpcUnit->GetIRFrontEnd()->GlobalRoutine(szFinaliser.Text(), psRoutine->szLabel.Text(), psRoutine->psType, NULL, true, NULL, NULL, false, !psRoutine->bShared))
		{
			mpcDiagnostics->ErrorAt(-1, "This static local's destruction uses C++ the IR front end doesn't support yet");
		}
		szFinaliser.Kill();
		bWritten = true;
	}
	CCPPUnitData::KillRoutines(&masExitFinalisers);

	if (mbExitLists && !mbExitListsWritten)
	{
		mbExitListsWritten = true;
		szLabel.Init("R_RunExitLists");
		if (mpcUnit->Emit(&szLabel))
		{
			CCPPUnitData::AppendRoutineSection(mpcUnit->GetCode(), &szLabel, true);
			mpcUnit->GetIRFrontEnd()->ExitListsRoutine(szLabel.Text());
			bWritten = true;
		}
		szLabel.Kill();
	}
	return bWritten;
}


//////////////////////////////////////////////////////////////////////////
//
// R_DynamicCast, the routine the unit's dynamic_casts call (UseDynamicCast),
// in a section the linker keeps one of, once.  True if it was written now.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPGlobals::WriteDynamicCast(void)
{
	CChars	szLabel;
	bool	bWritten;

	if (!mbDynamicCast || mbDynamicCastWritten)
	{
		return false;
	}
	mbDynamicCastWritten = true;
	bWritten = false;
	szLabel.Init(CPP65_DYNAMIC_CAST);
	if (mpcUnit->Emit(&szLabel))
	{
		CCPPUnitData::AppendRoutineSection(mpcUnit->GetCode(), &szLabel, true);
		mpcUnit->GetIRFrontEnd()->DynamicCastRoutine(szLabel.Text());
		bWritten = true;
	}
	szLabel.Kill();
	return bWritten;
}
