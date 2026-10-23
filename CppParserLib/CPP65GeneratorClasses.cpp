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
#include <stdlib.h>
#include "CTIdentifier.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
// The class a symbol names.  A class used before it is defined (through a pointer)
// is made incomplete and filled in by DefineClass.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::GetClass(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	SCPP65Class*	psClass;
	size			i;

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (psClass->pcSymbol == pcSymbol)
		{
			return psClass;
		}
	}

	psClass = (SCPP65Class*)malloc(sizeof(SCPP65Class));
	psClass->pcSymbol = pcSymbol;
	psClass->lliName = pcSymbol->mlliName;
	psClass->psType = mcTypes.Class(psClass);
	psClass->psBase = NULL;
	psClass->macFields.Init();
	psClass->mapsVirtuals.Init();
	psClass->iVtableOffset = -1;
	psClass->bComplete = false;
	psClass->bConstructors = false;
	psClass->psDestructor = NULL;
	mapsClasses.Add(psClass);
	return psClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::FindClass(int64 lliName)
{
	SCPP65Class*	psClass;
	size			i;

	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		if (psClass->lliName == lliName)
		{
			return psClass;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays out a class, writes its vtable and implicit constructor and destructor, then
// generates the member functions defined in it (which see the whole class).
// Members are read twice: functions and fields first, then the fields get their
// places once it is known whether the class needs a vtable pointer.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Class* CCPP65Generator::DefineClass(CCPPSyntaxClassSpecifier* pcSpecifier)
{
	SCPP65Class*				psClass;
	CCPPSyntaxBaseSpecifier*	pcBase;
	CCPPSyntaxNameComponent*	pcComponent;
	SCPP65Field*				psField;
	SCPP65Function*				psFunction;
	CArraySyntax				apcBodies;
	size						i;
	int							iBytes;
	int							iAlign;
	int							iFieldAlign;
	bool						bResult;

	if (!pcSpecifier->mpcSymbol)
	{
		Error(pcSpecifier, "Classes without names are not supported by the 65C64 generator yet");
		return NULL;
	}
	psClass = GetClass(pcSpecifier->mpcSymbol, pcSpecifier);
	if (psClass->bComplete)
	{
		return psClass;
	}
	if (pcSpecifier->meKey == CT_union)
	{
		Error(pcSpecifier, "Unions are not supported by the 65C64 generator yet");
		return NULL;
	}
	pcComponent = pcSpecifier->mpcName ? pcSpecifier->mpcName->GetLast() : NULL;
	if (pcComponent)
	{
		psClass->lliName = GetComponentName(pcComponent);
	}

	if (pcSpecifier->macpBases.NumElements() > 1)
	{
		Error(pcSpecifier, "Multiple inheritance is not supported by the 65C64 generator yet");
		return NULL;
	}
	if (pcSpecifier->macpBases.NumElements() == 1)
	{
		pcBase = (CCPPSyntaxBaseSpecifier*)pcSpecifier->macpBases.GetPtr(0);
		if (pcBase->mbVirtual || !pcBase->mpcName->mpcSymbol || !pcBase->mpcName->mpcSymbol->IsClass())
		{
			Error(pcBase, "This base class is not supported by the 65C64 generator yet");
			return NULL;
		}
		psClass->psBase = GetClass(pcBase->mpcName->mpcSymbol, pcBase);
		if (!psClass->psBase->bComplete)
		{
			Error(pcBase, "The base class has not been defined");
			return NULL;
		}
	}

	apcBodies.Init();
	bResult = true;
	iBytes = 0;
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		bResult &= ClassMember(psClass, pcSpecifier->macpMembers.GetPtr(i), &iBytes, &apcBodies);
	}

	//A destructor is needed to destroy the base or the fields.
	if (!psClass->psDestructor && ((psClass->psBase && psClass->psBase->psDestructor) || NeedsDestructor(psClass)))
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Destructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psClass->psDestructor = psFunction;
	}
	bResult &= LayoutVirtuals(psClass, pcSpecifier);

	//The base, the vtable pointer, then the fields.
	iBytes = psClass->psBase ? psClass->psBase->psType->iBytes : 0;
	iAlign = (psClass->psBase && (iBytes > 0)) ? 4 : 1;
	if (psClass->psBase && (psClass->psBase->iVtableOffset != -1))
	{
		psClass->iVtableOffset = psClass->psBase->iVtableOffset;
	}
	else if (psClass->mapsVirtuals.NumElements() > 0)
	{
		iBytes = (iBytes + 3) & ~3;
		psClass->iVtableOffset = iBytes;
		iBytes += 4;
		iAlign = 4;
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		iFieldAlign = (psField->psType->iBytes >= 4) || psField->psType->IsArray() || psField->psType->IsClass() ? 4 : psField->psType->iBytes;
		iBytes = (iBytes + iFieldAlign - 1) & ~(iFieldAlign - 1);
		psField->iOffset = iBytes;
		iBytes += psField->psType->iBytes;
		iAlign = (iFieldAlign > iAlign) ? iFieldAlign : iAlign;
	}
	iBytes = (iBytes + iAlign - 1) & ~(iAlign - 1);
	psClass->psType->iBytes = (iBytes > 0) ? iBytes : 1;
	psClass->bComplete = true;

	if (psClass->iVtableOffset != -1)
	{
		AppendVtable(psClass);
	}
	if (bResult)
	{
		bResult = ImplicitMembers(psClass);
	}
	for (i = 0; i < apcBodies.NumElements(); i++)
	{
		bResult &= Function((CCPPSyntaxFunctionDefinition*)apcBodies.GetPtr(i), psClass);
	}
	apcBodies.Kill();
	return bResult ? psClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Declares a member of a class being defined.  Function definitions are kept in
// papcBodies to be generated when the class is complete.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ClassMember(SCPP65Class* psClass, CCPPSyntax* pcMember, int* piBytes, CArraySyntax* papcBodies)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxDeclSpecifiers*		pcSpecifiers;
	SCPP65Field*					psField;
	SCPP65Variable*					psVariable;
	SCPP65Type*						psType;
	size							i;
	bool							bPure;
	bool							bResult;
	int64							lliZero;

	switch (pcMember->GetKind())
	{
	case CPPS_AccessSpecifier:
	case CPPS_EmptyDeclaration:
	case CPPS_StaticAssert:
		return true;

	case CPPS_FunctionDefinition:
		pcFunction = (CCPPSyntaxFunctionDefinition*)pcMember;
		if (!DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, false, psClass, false))
		{
			return false;
		}
		papcBodies->Add(pcFunction);
		return true;

	case CPPS_SimpleDeclaration:
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
		pcSpecifiers = pcDeclaration->mpcSpecifiers;
		if (pcSpecifiers && (pcSpecifiers->muiFlags & (CPPDS_TYPEDEF | CPPDS_FRIEND)))
		{
			return true;
		}
		if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
		{
			return Error(pcMember, "Nested classes are not supported by the 65C64 generator yet");
		}

		bResult = true;
		for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
			if (pcInit->mpcDeclarator->IsFunction())
			{
				//virtual int f() = 0;
				bPure = pcInit->mpcInitializer && Constant(pcInit->mpcInitializer, &lliZero) && (lliZero == 0);
				bResult &= DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, psClass, bPure) != NULL;
				continue;
			}
			if (!pcInit->mpcSymbol)
			{
				bResult &= Error(pcInit, "This member is not supported by the 65C64 generator yet");
				continue;
			}

			psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				bResult = false;
				continue;
			}
			if ((psType->IsClass() || (psType->IsArray() && psType->psBase->IsClass())) && !(psType->IsClass() ? psType : psType->psBase)->psClass->bComplete)
			{
				bResult &= Error(pcInit, "A member's class must be defined before it");
				continue;
			}

			if (pcSpecifiers->muiFlags & CPPDS_STATIC)
			{
				psVariable = (SCPP65Variable*)malloc(sizeof(SCPP65Variable));
				psVariable->pcSymbol = pcInit->mpcSymbol;
				psVariable->psType = psType;
				psVariable->eStorage = CPP65S_Global;
				psVariable->iOffset = 0;
				psVariable->psClass = psClass;
				psVariable->bDefined = false;
				mapsGlobals.Add(psVariable);
				continue;
			}

			psField = psClass->macFields.Add();
			psField->pcSymbol = pcInit->mpcSymbol;
			psField->lliName = pcInit->mpcSymbol->mlliName;
			psField->psType = psType;
			psField->iOffset = 0;
			psField->pcInitialiser = pcInit->mpcInitializer;
		}
		return bResult;

	default:
		return Error(pcMember, "This member is not supported by the 65C64 generator yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The vtable starts as the base's.  A function with the name and parameters of a
// virtual function in it overrides it (and is virtual); other virtual functions are
// added on the end.  A destructor overrides a virtual base destructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LayoutVirtuals(SCPP65Class* psClass, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psVirtual;
	size				i;
	size				j;
	bool				bOverride;

	if (psClass->psBase)
	{
		for (i = 0; i < psClass->psBase->mapsVirtuals.NumElements(); i++)
		{
			psClass->mapsVirtuals.Add(psClass->psBase->mapsVirtuals.GetPtr(i));
		}
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if ((psFunction->psClass != psClass) || psFunction->bStatic || (psFunction->eKind == CPP65FK_Constructor))
		{
			continue;
		}

		bOverride = false;
		for (j = 0; j < psClass->mapsVirtuals.NumElements(); j++)
		{
			psVirtual = psClass->mapsVirtuals.GetPtr(j);
			if ((psVirtual->eKind == psFunction->eKind) && ((psFunction->eKind == CPP65FK_Destructor) || ((psVirtual->lliName == psFunction->lliName) && SameParameters(psVirtual, psFunction))))
			{
				psClass->mapsVirtuals.SetPtr(j, psFunction);
				psFunction->bVirtual = true;
				psFunction->iSlot = (int)j;
				bOverride = true;
				break;
			}
		}
		if (!bOverride && psFunction->bVirtual)
		{
			psFunction->iSlot = (int)psClass->mapsVirtuals.NumElements();
			psClass->mapsVirtuals.Add(psFunction);
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__2: the addresses of the virtual functions.  Pure virtual functions are
// R_Pure, which stops.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendVtable(SCPP65Class* psClass)
{
	SCPP65Function*	psFunction;
	size			i;

	mszData.Append("\t\t.align 4");
	mszData.AppendNewLine();
	AppendClassLabel(&mszData, psClass, "2");
	mszData.Append(':');
	mszData.AppendNewLine();
	mszData.Append("\t\t.long ");
	for (i = 0; i < psClass->mapsVirtuals.NumElements(); i++)
	{
		psFunction = psClass->mapsVirtuals.GetPtr(i);
		if (i > 0)
		{
			mszData.Append(", ");
		}
		if (psFunction->bPure)
		{
			mszData.Append("R_Pure");
			mbPureVirtual = true;
		}
		else
		{
			AppendFunctionLabel(&mszData, psFunction);
		}
	}
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// A field of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindField(SCPP65Class* psClass, int64 lliName)
{
	SCPP65Field*	psField;
	size			i;

	for (; psClass; psClass = psClass->psBase)
	{
		for (i = 0; i < psClass->macFields.NumElements(); i++)
		{
			psField = psClass->macFields.Get(i);
			if (psField->lliName == lliName)
			{
				return psField;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Field* CCPP65Generator::FindFieldSymbol(SCPP65Class* psClass, CCPPSymbol* pcSymbol)
{
	SCPP65Field*	psField;
	size			i;

	for (; psClass; psClass = psClass->psBase)
	{
		for (i = 0; i < psClass->macFields.NumElements(); i++)
		{
			psField = psClass->macFields.Get(i);
			if (psField->pcSymbol == pcSymbol)
			{
				return psField;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A static data member of the class or of its bases.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::FindStatic(SCPP65Class* psClass, int64 lliName)
{
	SCPP65Variable*	psVariable;
	size			i;

	for (; psClass; psClass = psClass->psBase)
	{
		for (i = 0; i < mapsGlobals.NumElements(); i++)
		{
			psVariable = mapsGlobals.GetPtr(i);
			if ((psVariable->psClass == psClass) && (psVariable->pcSymbol->mlliName == lliName))
			{
				return psVariable;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Overload resolution, simplified: the functions of the name (in the class, then its
// bases if bBases) that take the number of arguments, scored by how well each
// argument's type matches its parameter (the same type, then integer to integer and
// related pointers).  Returns NULL with an error if none or more than one is best.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::FindFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, CArraySyntax* papcArguments, bool bBases, CCPPSyntax* pcNode)
{
	SCPP65Function*		psFunction;
	SCPP65Function*		psBest;
	SCPP65Type*			apsArguments[32];
	SCPP65Type*			psParameter;
	SCPP65Type*			psArgument;
	size				uiArguments;
	size				i;
	size				j;
	int					iScore;
	int					iBest;
	int					iMatch;
	bool				bAmbiguous;
	bool				bFound;
	int64				lliValue;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments > 32)
	{
		Error(pcNode, "Too many arguments");
		return NULL;
	}
	for (i = 0; i < uiArguments; i++)
	{
		apsArguments[i] = TypeOf(papcArguments->GetPtr(i));
		if (!apsArguments[i])
		{
			Expression(papcArguments->GetPtr(i));
			return NULL;
		}
	}

	for (;;)
	{
		psBest = NULL;
		iBest = -1;
		bAmbiguous = false;
		bFound = false;
		for (i = 0; i < mapsFunctions.NumElements(); i++)
		{
			psFunction = mapsFunctions.GetPtr(i);
			if ((psFunction->psClass != psClass) || (psFunction->lliName != lliName) || (psFunction->eKind != eKind))
			{
				continue;
			}
			bFound = true;
			if (psFunction->mapsParameters.NumElements() != uiArguments)
			{
				continue;
			}

			iScore = 0;
			for (j = 0; j < uiArguments; j++)
			{
				psParameter = psFunction->mapsParameters.GetPtr(j);
				psArgument = apsArguments[j];
				if (psParameter->Equals(psArgument))
				{
					iMatch = 3;
				}
				else if (psParameter->IsInteger() && psArgument->IsInteger())
				{
					iMatch = 2;
				}
				else if (psParameter->IsPointer() && psArgument->IsPointer())
				{
					iMatch = (psParameter->psBase->IsVoid() || (psParameter->psBase->IsClass() && psArgument->psBase->IsClass() && IsDerived(psArgument->psBase->psClass, psParameter->psBase->psClass))) ? 1 : -1;
				}
				else if (psParameter->IsPointer() && psArgument->IsInteger() && Constant(papcArguments->GetPtr(j), &lliValue) && (lliValue == 0))
				{
					iMatch = 1;
				}
				else
				{
					iMatch = -1;
				}
				if (iMatch < 0)
				{
					iScore = -1;
					break;
				}
				iScore += iMatch;
			}

			if (iScore > iBest)
			{
				psBest = psFunction;
				iBest = iScore;
				bAmbiguous = false;
			}
			else if ((iScore == iBest) && (iScore >= 0))
			{
				bAmbiguous = true;
			}
		}

		if (psBest && !bAmbiguous && (iBest >= 0))
		{
			return psBest;
		}
		if (bAmbiguous)
		{
			Error(pcNode, "The call is ambiguous");
			return NULL;
		}
		if (!bFound && bBases && psClass && psClass->psBase)
		{
			psClass = psClass->psBase;
			continue;
		}
		if (bFound)
		{
			Error(pcNode, "No function matches the arguments");
		}
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psClass is psBase or derives from it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsDerived(SCPP65Class* psClass, SCPP65Class* psBase)
{
	for (; psClass; psClass = psClass->psBase)
	{
		if (psClass == psBase)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether making an object of the class runs code: a constructor, a vtable pointer
// to set, a base or a field that needs it, or a default member initialiser.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::NeedsConstructor(SCPP65Class* psClass)
{
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;

	if (psClass->bConstructors || (psClass->iVtableOffset != -1))
	{
		return true;
	}
	if (psClass->psBase && NeedsConstructor(psClass->psBase))
	{
		return true;
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		if (psField->pcInitialiser)
		{
			return true;
		}
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && NeedsConstructor(psType->psClass))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether destroying an object of the class runs code.  While the class is being
// defined this asks whether its fields need destroying.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::NeedsDestructor(SCPP65Class* psClass)
{
	SCPP65Field*	psField;
	SCPP65Type*		psType;
	size			i;

	if (psClass->psDestructor)
	{
		return true;
	}
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		psType = psField->psType->IsArray() ? psField->psType->psBase : psField->psType;
		if (psType->IsClass() && psType->psClass->psDestructor)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Functions are _name.  Members are _Class__name, constructors _Class__0 and
// destructors _Class__1; the vtable is _Class__2.  An overload has __n on the end.
// C++ names cannot contain __ and cannot start with a digit, so none can clash.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendFunctionLabel(CChars* psz, SCPP65Function* psFunction)
{
	CChars	szName;
	char*	szIdentifier;
	int		iLength;

	if (!psFunction->psClass)
	{
		AppendName(psz, psFunction->lliName);
	}
	else if (psFunction->eKind == CPP65FK_Constructor)
	{
		AppendClassLabel(psz, psFunction->psClass, "0");
	}
	else if (psFunction->eKind == CPP65FK_Destructor)
	{
		AppendClassLabel(psz, psFunction->psClass, "1");
	}
	else
	{
		szIdentifier = mpcTokeniser->GetIdentifier(psFunction->lliName, &iLength);
		szName.Init(szIdentifier, 0, iLength);
		AppendClassLabel(psz, psFunction->psClass, szName.Text());
		szName.Kill();
	}

	if (psFunction->iOverload > 0)
	{
		psz->Append("__");
		psz->Append(psFunction->iOverload);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _Class__part.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendClassLabel(CChars* psz, SCPP65Class* psClass, const char* szPart)
{
	AppendName(psz, psClass->lliName);
	psz->Append("__");
	psz->Append(szPart);
}


//////////////////////////////////////////////////////////////////////////
//
// The constructor and destructor the generator writes for a class that needs them
// but declares none.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ImplicitMembers(SCPP65Class* psClass)
{
	SCPP65Function*	psFunction;
	bool			bResult;

	bResult = true;
	if (!psClass->bConstructors && NeedsConstructor(psClass))
	{
		psFunction = AddFunction(psClass, psClass->lliName, CPP65FK_Constructor, &mcTypes.msVoid);
		psFunction->bImplicit = true;
		psFunction->bDefined = true;
		BeginFunction(psFunction);
		bResult &= ConstructorBody(psFunction, NULL);
		EndFunction(psFunction);
	}

	psFunction = psClass->psDestructor;
	if (psFunction && psFunction->bImplicit)
	{
		psFunction->bDefined = true;
		BeginFunction(psFunction);
		bResult &= SetVtable(NULL, psClass);
		EndFunction(psFunction);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The start of a constructor: the base, the vtable pointer, then the fields from the
// member initialisers, the default member initialisers or their own constructors.
// The body follows.  pcFunction is NULL for an implicit constructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConstructorBody(SCPP65Function* psFunction, CCPPSyntaxFunctionDefinition* pcFunction)
{
	SCPP65Class*					psClass;
	CCPPSyntaxMemberInitializer*	pcMember;
	CCPPSyntaxMemberInitializer*	pcFound;
	CArraySyntax*					papcArguments;
	SCPP65Field*					psField;
	SCPP65Place						sPlace;
	CArraySyntax					apcNone;
	CCPPSyntax*						pcInitialiser;
	CCPPSymbol*						pcSymbol;
	size							i;
	size							j;
	bool							bResult;

	psClass = psFunction->psClass;
	bResult = true;
	apcNone.Init();

	sPlace.eKind = CPP65PK_This;
	sPlace.iOffset = 0;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.psClass = NULL;

	if (pcFunction)
	{
		for (i = 0; i < pcFunction->macpMemberInitializers.NumElements(); i++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)pcFunction->macpMemberInitializers.GetPtr(i);
			pcSymbol = pcMember->mpcName->mpcSymbol;
			if (!pcSymbol || (!(psClass->psBase && (pcSymbol == psClass->psBase->pcSymbol)) && !FindFieldSymbol(psClass, pcSymbol)))
			{
				bResult &= Error(pcMember, "The member initialiser is not a base or a field of the class");
			}
		}
	}

	//The base.
	if (psClass->psBase)
	{
		pcFound = NULL;
		for (i = 0; pcFunction && (i < pcFunction->macpMemberInitializers.NumElements()); i++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)pcFunction->macpMemberInitializers.GetPtr(i);
			if (pcMember->mpcName->mpcSymbol == psClass->psBase->pcSymbol)
			{
				pcFound = pcMember;
			}
		}
		sPlace.psType = psClass->psBase->psType;
		sPlace.iOffset = 0;
		papcArguments = &apcNone;
		if (pcFound && pcFound->mpcInitializer)
		{
			if (pcFound->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer)
			{
				papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcFound->mpcInitializer)->macpElements;
			}
			else if (pcFound->mpcInitializer->GetKind() == CPPS_InitializerList)
			{
				papcArguments = &((CCPPSyntaxInitializerList*)pcFound->mpcInitializer)->macpElements;
			}
		}
		bResult &= ConstructClass(&sPlace, psClass->psBase, papcArguments, pcFound ? (CCPPSyntax*)pcFound : (CCPPSyntax*)pcFunction);
	}

	bResult &= SetVtable(NULL, psClass);

	//The fields, in order.
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		pcFound = NULL;
		for (j = 0; pcFunction && (j < pcFunction->macpMemberInitializers.NumElements()); j++)
		{
			pcMember = (CCPPSyntaxMemberInitializer*)pcFunction->macpMemberInitializers.GetPtr(j);
			if (pcMember->mpcName->mpcSymbol == psField->pcSymbol)
			{
				pcFound = pcMember;
			}
		}

		sPlace.eKind = CPP65PK_This;
		sPlace.psType = psField->psType;
		sPlace.iOffset = psField->iOffset;
		pcInitialiser = pcFound ? pcFound->mpcInitializer : psField->pcInitialiser;
		if (pcInitialiser || psField->psType->IsClass() || psField->psType->IsArray())
		{
			bResult &= Construct(&sPlace, NULL, pcInitialiser, pcFound ? (CCPPSyntax*)pcFound : (CCPPSyntax*)pcFunction);
		}
	}

	apcNone.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The end of a destructor, after its body: the fields in reverse, then the base.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DestructorBody(SCPP65Function* psFunction)
{
	SCPP65Class*	psClass;
	SCPP65Field*	psField;
	SCPP65Place		sPlace;
	size			i;
	bool			bResult;

	psClass = psFunction->psClass;
	bResult = true;
	sPlace.eKind = CPP65PK_This;
	sPlace.lliName = -1;
	sPlace.iString = -1;
	sPlace.psClass = NULL;
	for (i = psClass->macFields.NumElements(); i > 0; i--)
	{
		psField = psClass->macFields.Get(i - 1);
		sPlace.psType = psField->psType;
		sPlace.iOffset = psField->iOffset;
		bResult &= Destroy(&sPlace);
	}
	if (psClass->psBase && psClass->psBase->psDestructor)
	{
		sPlace.psType = psClass->psBase->psType;
		sPlace.iOffset = 0;
		bResult &= Destroy(&sPlace);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Initialises the object at a place from a declaration's initialiser (or a member
// initialiser): nothing, (arguments), {arguments}, = value or = Class(arguments).
// Scalars are stored, arrays are initialised element by element, objects are
// constructed or copied.  papcArguments is used when pcInitialiser is NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Construct(SCPP65Place* psPlace, CArraySyntax* papcArguments, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	SCPP65Type*						psType;
	SCPP65Type*						psSource;
	SCPP65Place						sElement;
	CArraySyntax					apcArguments;
	CCPPSyntaxConstructExpression*	pcConstruct;
	SCPP65Class*					psClass;
	int								i;
	bool							bResult;

	psType = psPlace->psType;
	apcArguments.Init();
	if (!papcArguments)
	{
		papcArguments = &apcArguments;
	}

	if (pcInitialiser)
	{
		switch (pcInitialiser->GetKind())
		{
		case CPPS_ParenthesisedInitializer:
			papcArguments = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			break;
		case CPPS_InitializerList:
			papcArguments = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
			pcInitialiser = NULL;
			break;
		default:
			break;
		}
	}

	if (psType->IsArray())
	{
		bResult = true;
		if (pcInitialiser || (papcArguments->NumElements() > (size)psType->iCount))
		{
			apcArguments.Kill();
			return Error(pcNode, "This array initialiser is not supported by the 65C64 generator yet");
		}
		sElement = *psPlace;
		sElement.psType = psType->psBase;
		for (i = 0; i < psType->iCount; i++)
		{
			if ((size)i < papcArguments->NumElements())
			{
				bResult &= Construct(&sElement, NULL, papcArguments->GetPtr(i), pcNode);
			}
			else if (psType->psBase->IsClass())
			{
				bResult &= Construct(&sElement, NULL, NULL, pcNode);
			}
			else
			{
				Line("cpy A, 0")->AppendNewLine();
				Store(&sElement, "A", NULL);
			}
			sElement.iOffset += psType->psBase->iBytes;
		}
		apcArguments.Kill();
		return bResult;
	}

	if (!psType->IsClass())
	{
		if (!pcInitialiser)
		{
			if (papcArguments->NumElements() > 1)
			{
				apcArguments.Kill();
				return Error(pcNode, "Expected one initialiser");
			}
			pcInitialiser = (papcArguments->NumElements() == 1) ? papcArguments->GetPtr(0) : NULL;
		}
		if (pcInitialiser)
		{
			bResult = Convert(Expression(pcInitialiser), psType, pcInitialiser);
		}
		else
		{
			Line("cpy A, 0")->AppendNewLine();
			bResult = true;
		}
		if (bResult)
		{
			Store(psPlace, "A", NULL);
		}
		apcArguments.Kill();
		return bResult;
	}

	psClass = psType->psClass;
	if (pcInitialiser)
	{
		//= Class(arguments) constructs in place.
		if ((pcInitialiser->GetKind() == CPPS_ConstructExpression))
		{
			pcConstruct = (CCPPSyntaxConstructExpression*)pcInitialiser;
			if ((GetTypeIdType(pcConstruct->mpcType) == psType) && pcConstruct->mpcInitializer && (pcConstruct->mpcInitializer->GetKind() == CPPS_ParenthesisedInitializer))
			{
				bResult = ConstructClass(psPlace, psClass, &((CCPPSyntaxParenthesisedInitializer*)pcConstruct->mpcInitializer)->macpElements, pcNode);
				apcArguments.Kill();
				return bResult;
			}
		}

		//= object copies it, = value calls a constructor that takes it.
		psSource = TypeOf(pcInitialiser);
		if (psSource && psSource->IsClass() && IsDerived(psSource->psClass, psClass))
		{
			bResult = CopyObject(psPlace, pcInitialiser, pcNode);
			apcArguments.Kill();
			return bResult;
		}
		apcArguments.Add(pcInitialiser);
		papcArguments = &apcArguments;
	}

	bResult = ConstructClass(psPlace, psClass, papcArguments, pcNode);
	apcArguments.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls the constructor of psClass that matches the arguments for the object at a
// place.  A class without constructors takes its fields from {arguments}, in order.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConstructClass(SCPP65Place* psPlace, SCPP65Class* psClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	SCPP65Function*	psConstructor;
	SCPP65Field*	psField;
	SCPP65Place		sField;
	size			i;
	bool			bResult;

	if (psClass->bConstructors)
	{
		psConstructor = FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, pcNode);
		if (!psConstructor)
		{
			return Error(pcNode, "No constructor matches the arguments");
		}
		return CallMember(psConstructor, psPlace, papcArguments, false, pcNode);
	}

	if (papcArguments->NumElements() == 0)
	{
		psConstructor = FindFunction(psClass, psClass->lliName, CPP65FK_Constructor, papcArguments, false, pcNode);
		if (psConstructor)
		{
			return CallMember(psConstructor, psPlace, papcArguments, false, pcNode);
		}
		return true;
	}

	//An aggregate: the fields in order, the rest zero.
	if (psClass->psBase || (psClass->iVtableOffset != -1) || (papcArguments->NumElements() > psClass->macFields.NumElements()))
	{
		return Error(pcNode, "These initialisers do not match the class");
	}
	bResult = true;
	sField = *psPlace;
	for (i = 0; i < psClass->macFields.NumElements(); i++)
	{
		psField = psClass->macFields.Get(i);
		sField.psType = psField->psType;
		sField.iOffset = psPlace->iOffset + psField->iOffset;
		bResult &= Construct(&sField, NULL, (i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL, pcNode);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls the destructor of the object (or each object of an array, last first) at a
// place.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Destroy(SCPP65Place* psPlace)
{
	SCPP65Place		sElement;
	SCPP65Type*		psType;
	int				i;
	bool			bResult;

	psType = psPlace->psType;
	if (psType->IsArray())
	{
		if (!psType->psBase->IsClass() || !psType->psBase->psClass->psDestructor)
		{
			return true;
		}
		bResult = true;
		sElement = *psPlace;
		sElement.psType = psType->psBase;
		for (i = psType->iCount; i > 0; i--)
		{
			sElement.iOffset = psPlace->iOffset + (i - 1) * psType->psBase->iBytes;
			bResult &= Destroy(&sElement);
		}
		return bResult;
	}
	if (!psType->IsClass() || !psType->psClass->psDestructor)
	{
		return true;
	}
	return CallMember(psType->psClass->psDestructor, psPlace, NULL, false, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a member function: the arguments are pushed, X gets the object's address
// (from psObject, or this when it is NULL) and the function is called, through the
// vtable if bVirtual and the function is virtual.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CallMember(SCPP65Function* psFunction, SCPP65Place* psObject, CArraySyntax* papcArguments, bool bVirtual, CCPPSyntax* pcNode)
{
	int		iBytes;

	iBytes = PushArguments(psFunction, papcArguments, pcNode);
	if (iBytes < 0)
	{
		return false;
	}

	if (!psFunction->bStatic)
	{
		if (psObject)
		{
			LoadAddress(psObject);
			Line("cpy X, A")->AppendNewLine();
			ChangeX();
		}
		else
		{
			if (miThisOffset == -1)
			{
				return Error(pcNode, "A member function needs an object");
			}
			EnsureThis();
		}
	}

	Call(psFunction, bVirtual, iBytes);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a function whose arguments are pushed (and whose object is in X), then
// removes the arguments.  A virtual call goes through the object's vtable:
//
//		cpy A, [vtable + X]
//		jsr [4 * slot + A]
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Call(SCPP65Function* psFunction, bool bVirtual, int iBytes)
{
	if (bVirtual && psFunction->bVirtual)
	{
		Line("cpy A, [")->Append(psFunction->psClass->iVtableOffset)->Append(" + X]")->AppendNewLine();
		Line("jsr [")->Append(4 * psFunction->iSlot)->Append(" + A]")->AppendNewLine();
	}
	else
	{
		Line("jsr ");
		AppendFunctionLabel(mpszOut, psFunction);
		mpszOut->AppendNewLine();
	}
	ChangeX();
	if (iBytes > 0)
	{
		Line("add S, ")->Append(iBytes)->AppendNewLine();
		miDepth -= iBytes;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// object.f(), pointer->f(), f() and Class::f() in a member function.  Calls through a
// pointer or this are virtual; calls on an object variable and qualified calls are
// not.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::MemberCall(CCPPSyntaxCallExpression* pcCall, CCPPSyntax* pcCallee)
{
	CCPPSyntaxMemberExpression*	pcMember;
	CCPPSyntaxIdExpression*		pcId;
	CCPPSyntaxName*				pcName;
	CCPPSyntax*					pcObject;
	SCPP65Class*				psClass;
	SCPP65Function*				psFunction;
	SCPP65Type*					psType;
	SCPP65Place					sObject;
	int64						lliName;
	int							iBytes;
	size						uiErrors;
	bool						bVirtual;
	bool						bThis;

	psClass = NULL;
	pcObject = NULL;
	bVirtual = true;
	bThis = false;
	if (pcCallee->GetKind() == CPPS_MemberExpression)
	{
		pcMember = (CCPPSyntaxMemberExpression*)pcCallee;
		psType = TypeOf(pcMember->mpcObject);
		if (!psType)
		{
			Expression(pcMember->mpcObject);
			return NULL;
		}
		if (pcMember->mbArrow)
		{
			psType = psType->IsPointer() ? psType->psBase : NULL;
		}
		if (!psType || !psType->IsClass())
		{
			Error(pcCallee, "Member functions need an object of a class");
			return NULL;
		}
		psClass = psType->psClass;
		pcObject = pcMember->mpcObject;
		pcName = pcMember->mpcName;
		if (!pcMember->mbArrow && (SkipParentheses(pcObject)->GetKind() == CPPS_IdExpression))
		{
			bVirtual = false;
		}
	}
	else
	{
		pcId = (CCPPSyntaxIdExpression*)pcCallee;
		pcName = pcId->mpcName;
		if (pcName->macpComponents.NumElements() == 2)
		{
			psClass = FindClass(GetComponentName((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0)));
			bVirtual = false;
		}
		else if (mpsFunction)
		{
			psClass = mpsFunction->psClass;
		}
		bThis = true;
	}

	lliName = GetComponentName(pcName->GetLast());
	if (!psClass || (lliName == -1))
	{
		Error(pcCallee, "This call is not supported by the 65C64 generator yet");
		return NULL;
	}
	uiErrors = mcErrors.NumErrors();
	psFunction = FindFunction(psClass, lliName, CPP65FK_Function, &pcCall->macpArguments, true, pcCall);
	if (!psFunction)
	{
		if (mcErrors.NumErrors() == uiErrors)
		{
			Error(pcCallee, "The class has no member function of that name");
		}
		return NULL;
	}
	if (bThis && !psFunction->bStatic && (!mpsFunction || !mpsFunction->psClass || !IsDerived(mpsFunction->psClass, psFunction->psClass)))
	{
		Error(pcCallee, "A member function needs an object");
		return NULL;
	}

	//The arguments are pushed before the object's address is worked out, as that is
	//left in A.
	iBytes = PushArguments(psFunction, &pcCall->macpArguments, pcCall);
	if (iBytes < 0)
	{
		return NULL;
	}
	if (!psFunction->bStatic)
	{
		if (pcObject)
		{
			if (((CCPPSyntaxMemberExpression*)pcCallee)->mbArrow)
			{
				if (!Expression(pcObject))
				{
					return NULL;
				}
			}
			else
			{
				if (!Place(pcObject, &sObject))
				{
					return NULL;
				}
				LoadAddress(&sObject);
			}
			Line("cpy X, A")->AppendNewLine();
			ChangeX();
		}
		else
		{
			EnsureThis();
		}
	}

	Call(psFunction, bVirtual, iBytes);
	return psFunction->psReturn;
}


//////////////////////////////////////////////////////////////////////////
//
// Pushes the arguments last first, each converted to its parameter's type and at
// its size.  Returns the bytes pushed, or -1 after an error.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::PushArguments(SCPP65Function* psFunction, CArraySyntax* papcArguments, CCPPSyntax* pcNode)
{
	size			uiArguments;
	size			i;
	SCPP65Type*		psType;
	SCPP65Type*		psParameter;
	CCPPSyntax*		pcArgument;
	int				iBytes;

	uiArguments = papcArguments ? papcArguments->NumElements() : 0;
	if (uiArguments != psFunction->mapsParameters.NumElements())
	{
		Error(pcNode, "The number of arguments does not match the function");
		return -1;
	}

	iBytes = 0;
	for (i = uiArguments; i > 0; i--)
	{
		pcArgument = papcArguments->GetPtr(i - 1);
		psType = Expression(pcArgument);
		psParameter = psFunction->mapsParameters.GetPtr(i - 1);
		if (psType && psType->IsInteger() && (psParameter->eKind == CPP65TK_Integer) && (psParameter->iBytes < 4))
		{
			//Pushing a narrow value keeps only its low bytes, which is the conversion.
		}
		else if (!Convert(psType, psParameter, pcArgument))
		{
			return -1;
		}
		PushValue(psParameter);
		iBytes += psParameter->iBytes;
	}
	return iBytes;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies an object (of the destination's class or one derived from it) a word at a
// time through Y.  The destination keeps its own class's vtable.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CopyObject(SCPP65Place* psDestination, CCPPSyntax* pcSource, CCPPSyntax* pcNode)
{
	SCPP65Place		sSource;
	SCPP65Class*	psClass;
	int				iBytes;
	int				iOffset;
	int				iChunk;

	psClass = psDestination->psType->psClass;
	if (psDestination->eKind == CPP65PK_Address)
	{
		Push("A");
	}
	if (!Place(pcSource, &sSource))
	{
		return false;
	}
	if (!sSource.psType->IsClass() || !IsDerived(sSource.psType->psClass, psClass))
	{
		return Error(pcNode, "The object cannot be copied to that class");
	}
	LoadAddress(&sSource);
	Line("cpy B, A")->AppendNewLine();
	if (psDestination->eKind == CPP65PK_Address)
	{
		Pull("A");
	}
	LoadAddress(psDestination);

	iBytes = psClass->psType->iBytes;
	for (iOffset = 0; iOffset < iBytes; iOffset += iChunk)
	{
		iChunk = (iBytes - iOffset >= 4) ? 4 : ((iBytes - iOffset >= 2) ? 2 : 1);
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
	if (psClass->iVtableOffset != -1)
	{
		Line("cpy Y, ");
		AppendClassLabel(mpszOut, psClass, "2");
		mpszOut->AppendNewLine();
		Line("cpy [")->Append(psClass->iVtableOffset)->Append(" + A], Y")->AppendNewLine();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// object.field and pointer->field.  A static member is a global.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::MemberPlace(CCPPSyntaxMemberExpression* pcMember, SCPP65Place* psPlace)
{
	SCPP65Type*		psType;
	SCPP65Field*	psField;
	SCPP65Variable*	psVariable;
	SCPP65Class*	psClass;
	int64			lliName;

	if (pcMember->mbArrow)
	{
		psType = Expression(pcMember->mpcObject);
		if (!psType)
		{
			return false;
		}
		if (!psType->IsPointer() || !psType->psBase->IsClass())
		{
			return Error(pcMember, "-> needs a pointer to an object");
		}
		psPlace->eKind = CPP65PK_Address;
		psPlace->psType = psType->psBase;
		psPlace->iOffset = 0;
		psPlace->lliName = -1;
		psPlace->iString = -1;
		psPlace->psClass = NULL;
	}
	else
	{
		if (!Place(pcMember->mpcObject, psPlace))
		{
			return false;
		}
		if (!psPlace->psType->IsClass())
		{
			return Error(pcMember, ". needs an object");
		}
	}

	psClass = psPlace->psType->psClass;
	lliName = GetComponentName(pcMember->mpcName->GetLast());
	psField = FindField(psClass, lliName);
	if (psField)
	{
		psPlace->psType = psField->psType;
		psPlace->iOffset += psField->iOffset;
		return true;
	}
	psVariable = FindStatic(psClass, lliName);
	if (psVariable)
	{
		return VariablePlace(psVariable, psPlace);
	}
	return Error(pcMember, "The class has no member of that name");
}


//////////////////////////////////////////////////////////////////////////
//
// Points the vtable pointer of this (psPlace is NULL) at psClass's vtable, in its
// constructors and destructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::SetVtable(SCPP65Place* psPlace, SCPP65Class* psClass)
{
	if (psClass->iVtableOffset == -1)
	{
		return true;
	}
	Line("cpy A, ");
	AppendClassLabel(mpszOut, psClass, "2");
	mpszOut->AppendNewLine();
	EnsureThis();
	Line("cpy [")->Append(psClass->iVtableOffset)->Append(" + X], A")->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Loads this into X from the frame if something else has used X.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EnsureThis(void)
{
	if (!mbThisInX && (miThisOffset != -1))
	{
		Line("cpy X, [")->Append(1 + miThisOffset)->Append(" + V]")->AppendNewLine();
	}
	mbThisInX = true;
}


//////////////////////////////////////////////////////////////////////////
//
// X no longer holds this.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::ChangeX(void)
{
	mbThisInX = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The identifier of a name component, or -1.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetComponentName(CCPPSyntaxNameComponent* pcComponent)
{
	CCPPToken*	pcToken;

	if (!pcComponent || (pcComponent->miName < 0))
	{
		return -1;
	}
	pcToken = mpcTokeniser->Get(pcComponent->miName);
	if (!pcToken->IsIdentifier())
	{
		return -1;
	}
	return ((CCTIdentifier*)pcToken)->mlliIdentifier;
}
