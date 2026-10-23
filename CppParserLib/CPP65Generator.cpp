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
#include "SourceFile.h"
#include "CTIdentifier.h"
#include "CTString.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPP65Generator.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Init(CCPPSyntaxTree* pcTree)
{
	mpcTree = pcTree;
	mpcTokeniser = pcTree->GetTokeniser();
	mcTypes.Init();

	mszCode.Init();
	mszData.Init();
	mszThunks.Init();
	maszRuntime.Init();
	mszBody.Init();
	mpszOut = &mszCode;
	mcErrors.Init();

	mapsGlobals.Init();
	mapsFunctions.Init();
	mapsClasses.Init();
	mapsTemplates.Init();
	mapsAllBindings.Init();
	mapsQueue.Init();
	mapcFailed.Init();
	mpsBindings = NULL;
	mcTypeOfs.Init(true);
	mszUnit.Init();
	maszOwnEmitted.Init();
	mpaszEmitted = &maszOwnEmitted;
	miQuiet = 0;
	mbQuietFailed = false;
	maszStrings.Init();
	mszInitialise.Init();
	mszFinalise.Init();
	mbPureVirtual = false;

	mapsLocals.Init();
	mapsDestructors.Init();
	macJumps.Init();
	macGotoLabels.Init();
	macCases.Init();
	mpsFunction = NULL;
	miFrameBytes = 0;
	miStatics = 0;
	mbStaticLocal = false;
	miDepth = 0;
	miTemporaries = 0;
	mbFramePointer = false;
	miReturnLabel = -1;
	miThisOffset = -1;
	mbThisInX = false;
	miLabels = 0;
	mbSignedDivide = false;
	mbSignedModulus = false;
	mbSignedDivide64 = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Kill(void)
{
	size			i;
	size			j;
	SCPP65Function*	psFunction;
	SCPP65Class*	psClass;
	SCPP65Bindings*	psBindings;

	macCases.Kill();
	macGotoLabels.Kill();
	macJumps.Kill();
	mapsDestructors.Kill();
	FreeVariables(&mapsLocals);
	mapsLocals.Kill();

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		maszStrings.Get(i)->Kill();
	}
	maszStrings.Kill();
	mszFinalise.Kill();
	mszInitialise.Kill();
	for (i = 0; i < mapsClasses.NumElements(); i++)
	{
		psClass = mapsClasses.GetPtr(i);
		psClass->mszArguments.Kill();
		for (j = 0; j < psClass->macVtables.NumElements(); j++)
		{
			psClass->macVtables.Get(j)->maiAdjust.Kill();
			psClass->macVtables.Get(j)->mapsFunctions.Kill();
		}
		psClass->macVtables.Kill();
		psClass->mapsVirtuals.Kill();
		psClass->macFields.Kill();
		psClass->macBases.Kill();
		free(psClass);
	}
	mapsClasses.Kill();
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		psFunction->mszArguments.Kill();
		psFunction->mapcDefaults.Kill();
		psFunction->mapsParameters.Kill();
		free(psFunction);
	}
	mapsFunctions.Kill();
	for (i = 0; i < mapsTemplates.NumElements(); i++)
	{
		free(mapsTemplates.GetPtr(i));
	}
	mapsTemplates.Kill();
	for (i = 0; i < mapsAllBindings.NumElements(); i++)
	{
		psBindings = mapsAllBindings.GetPtr(i);
		psBindings->macBindings.Kill();
		free(psBindings);
	}
	mapsAllBindings.Kill();
	mapsQueue.Kill();
	mapcFailed.Kill();
	mcTypeOfs.Kill();
	maszOwnEmitted.Kill();
	mszUnit.Kill();
	FreeVariables(&mapsGlobals);
	mapsGlobals.Kill();

	mcErrors.Kill();
	mszBody.Kill();
	maszRuntime.Kill();
	mszThunks.Kill();
	mszData.Kill();
	mszCode.Kill();
	mcTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the assembly for the translation unit to pszDest: the functions, the
// routines that make and destroy global objects, the runtime routines they use, then
// the data.  Returns false if anything could not be generated; GetErrors says what.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Generate(CChars* pszDest)
{
	CCPPSyntaxTranslationUnit*	pcRoot;
	SCPP65Function*				psFunction;

	pcRoot = mpcTree->GetRoot();
	if (pcRoot)
	{
		Declarations(&pcRoot->macpDeclarations);
	}

	//Functions generated when they are first called, which may call others.
	while (mapsQueue.NumElements() > 0)
	{
		psFunction = mapsQueue.GetPtr(0);
		mapsQueue.RemoveAt(0);
		GenerateFunction(psFunction);
	}

	if (mszInitialise.Length() > 0)
	{
		mszCode.Append(&mszUnit);
		mszCode.Append("R_Initialise:");
		mszCode.AppendNewLine();
		mszCode.Append(mszInitialise);
		mszCode.Append("\t\tret");
		mszCode.AppendNewLine();
		mszCode.AppendNewLine();
	}
	if (mszFinalise.Length() > 0)
	{
		mszCode.Append(&mszUnit);
		mszCode.Append("R_Finalise:");
		mszCode.AppendNewLine();
		mszCode.Append(mszFinalise);
		mszCode.Append("\t\tret");
		mszCode.AppendNewLine();
		mszCode.AppendNewLine();
	}
	AppendRuntime();

	pszDest->Append(mszCode);
	pszDest->Append(mszThunks);
	pszDest->Append(mszData);
	AppendStrings(pszDest);
	return mcErrors.NumErrors() == 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A whole program from one translation unit: the start up (see AppendStartUp), then
// the translation unit.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateProgram(CChars* pszDest)
{
	CChars		szUnit;
	CArrayChars	aszInitialise;
	CArrayChars	aszFinalise;
	bool		bResult;

	szUnit.Init();
	bResult = Generate(&szUnit);

	aszInitialise.Init();
	aszFinalise.Init();
	GetStartUp(&aszInitialise, &aszFinalise);
	AppendStartUp(pszDest, &aszInitialise, &aszFinalise);
	aszFinalise.Kill();
	aszInitialise.Kill();
	pszDest->Append(szUnit);
	szUnit.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// For a program of several translation units: szUnit is put in front of this
// unit's own labels (L1, Str0, R_Initialise ...) so they don't clash with other
// units', and paszEmitted (shared by all the units) has the labels of the
// functions, vtables and runtime routines written so far.  A unit leaves out the
// inline, template and generated functions, vtables and runtime routines another
// unit has written, as a linker would.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::SetUnit(const char* szUnit, CArrayChars* paszEmitted)
{
	mszUnit.Clear();
	mszUnit.Append(szUnit);
	mpaszEmitted = paszEmitted ? paszEmitted : &maszOwnEmitted;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the labels of this unit's routines that make and destroy its global objects,
// if it has any.  Call after Generate.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetStartUp(CArrayChars* paszInitialise, CArrayChars* paszFinalise)
{
	CChars	szLabel;

	if (mszInitialise.Length() > 0)
	{
		szLabel.Init(mszUnit);
		szLabel.Append("R_Initialise");
		paszInitialise->Add(szLabel);
		szLabel.Kill();
	}
	if (mszFinalise.Length() > 0)
	{
		szLabel.Init(mszUnit);
		szLabel.Append("R_Finalise");
		paszFinalise->Add(szLabel);
		szLabel.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The start up at 0001:0000 sets the stack to 001F:FFFF (the first 4 bytes pushed
// are at 001F:FFFC), W to the pseudo registers at 0000:0000 and the registers to
// 32 bits, makes the global objects (the units in order), calls main, destroys the
// global objects (the units in reverse) and stops with main's result in A.  The
// reset vector is left to the loader.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendStartUp(CChars* pszDest, CArrayChars* paszInitialise, CArrayChars* paszFinalise)
{
	size	i;

	pszDest->Append("\t\t.org $")->Append((uint32)CPP65_PROGRAM_ADDRESS, 16)->AppendNewLine();
	pszDest->Append("Start:")->AppendNewLine();
	pszDest->Append("\t\tcpy S, $")->Append((uint32)CPP65_STACK_TOP, 16)->AppendNewLine();
	pszDest->Append("\t\tcpy W, 0")->AppendNewLine();
	pszDest->Append("\t\twtth (A, B), 4")->AppendNewLine();
	pszDest->Append("\t\twtth (X, Y), 4")->AppendNewLine();
	for (i = 0; i < paszInitialise->NumElements(); i++)
	{
		pszDest->Append("\t\tjsr ")->Append(paszInitialise->GetText(i))->AppendNewLine();
	}
	pszDest->Append("\t\tjsr _main")->AppendNewLine();
	if (paszFinalise->NumElements() > 0)
	{
		pszDest->Append("\t\tpsh A")->AppendNewLine();
		for (i = paszFinalise->NumElements(); i > 0; i--)
		{
			pszDest->Append("\t\tjsr ")->Append(paszFinalise->GetText(i - 1))->AppendNewLine();
		}
		pszDest->Append("\t\tpul A")->AppendNewLine();
	}
	pszDest->Append("\t\tstp")->AppendNewLine();
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPP65Generator::GetErrors(void)
{
	//Their tokens are C++ token indices.
	return &mcErrors;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::NumErrors(void)
{
	return (int)mcErrors.NumErrors();
}


//////////////////////////////////////////////////////////////////////////
//
// The labels of functions and global variables that were used but not defined in
// the translation unit.  They must come from somewhere else.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GetExternals(CArrayChars* paszLabels)
{
	SCPP65Function*	psFunction;
	SCPP65Variable*	psVariable;
	CChars			szLabel;
	size			i;

	for (i = 0; i < mapsGlobals.NumElements(); i++)
	{
		psVariable = mapsGlobals.GetPtr(i);
		if (psVariable->bUsed && !psVariable->bDefined)
		{
			szLabel.Init();
			AppendGlobalLabel(&szLabel, psVariable->psClass, psVariable->pcSymbol->mlliName);
			paszLabels->AddIfUnique(szLabel);
			szLabel.Kill();
		}
	}

	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psFunction = mapsFunctions.GetPtr(i);
		if (psFunction->bCalled && !psFunction->bGenerated)
		{
			szLabel.Init();
			AppendFunctionLabel(&szLabel, psFunction);
			paszLabels->AddIfUnique(szLabel);
			szLabel.Kill();
		}
	}

	//The soft float runtime (Libc65/softfloat.cpp).
	for (i = 0; i < maszRuntime.NumElements(); i++)
	{
		paszLabels->AddIfUnique(maszRuntime.GetText(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The macros the 65C64 target predefines, for the configuration code is
// preprocessed with.  __65C64__ selects the target in BaseLib's Define.h.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AddDefines(CConfig* pcConfig)
{
	pcConfig->AddDefine("__65C64__=1");
}


//////////////////////////////////////////////////////////////////////////
//
// Errors are "File (line, column): message [token]", like the other stages'.
// In quiet mode (declarations, which only fail when what they declare is used) they
// are only noted.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Error(CCPPSyntax* pcNode, const char* szMessage)
{
	if (miQuiet > 0)
	{
		mbQuietFailed = true;
		return false;
	}
	return ErrorAt(pcNode ? pcNode->miFirstToken : -1, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ErrorAt(int iToken, const char* szMessage)
{
	SPreprocessedToken*	psSource;
	CChars				szDetail;

	if ((iToken < 0) || ((size)iToken >= mpcTokeniser->NumTokens()))
	{
		mcErrors.Add(NULL, -1, -1, -1, szMessage);
		return false;
	}

	psSource = mpcTokeniser->GetSource(mpcTokeniser->Get(iToken));
	szDetail.Init();
	psSource->Append(&szDetail);
	mcErrors.Add(psSource->pcFile ? psSource->pcFile->ShortName() : NULL, psSource->iLine, psSource->iColumn, iToken, szMessage, szDetail.Text());
	szDetail.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Declarations of things the generator cannot do yet are only errors when they
// are used.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::BeginQuiet(void)
{
	if (miQuiet == 0)
	{
		mbQuietFailed = false;
	}
	miQuiet++;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false if an error happened since BeginQuiet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::EndQuiet(void)
{
	miQuiet--;
	return !mbQuietFailed;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts an instruction line.  The caller appends the rest and the new line.
//
//////////////////////////////////////////////////////////////////////////
CChars* CCPP65Generator::Line(const char* szText)
{
	mpszOut->Append("\t\t");
	return mpszOut->Append(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::Label(int iLabel)
{
	//Code can arrive here from anywhere, so X may not hold this.
	ChangeX();
	AppendLabel(mpszOut, iLabel);
	mpszOut->Append(':');
	mpszOut->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Generated labels are L and a number.  C++ names start with an underscore so they
// never clash with them or with the registers.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendLabel(CChars* psz, int iLabel)
{
	psz->Append(&mszUnit);
	psz->Append('L');
	psz->Append(iLabel);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendName(CChars* psz, int64 lliName)
{
	char*	szName;
	int		iLength;

	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	psz->Append('_');
	psz->Append(szName, iLength);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::NewLabel(void)
{
	miLabels++;
	return miLabels;
}


//////////////////////////////////////////////////////////////////////////
//
// The identifier a declarator declares, or -1 for an operator, a qualified name or
// no name.
//
//////////////////////////////////////////////////////////////////////////
int64 CCPP65Generator::GetName(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPToken*					pcToken;

	pcName = pcDeclarator ? pcDeclarator->GetName() : NULL;
	if (!pcName || !pcName->IsSimple())
	{
		return -1;
	}
	pcComponent = pcName->GetLast();
	if (!pcComponent || (pcComponent->meKind != CPPNC_Identifier) || (pcComponent->miName < 0))
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Declarations(CArraySyntax* papcDeclarations)
{
	size					i;
	CCPPSyntax*				pcDeclaration;
	CCPPSyntaxLinkageSpecification*	pcLinkage;
	bool					bResult;

	bResult = true;
	for (i = 0; i < papcDeclarations->NumElements(); i++)
	{
		pcDeclaration = papcDeclarations->GetPtr(i);
		switch (pcDeclaration->GetKind())
		{
		case CPPS_FunctionDefinition:
			bResult &= Function((CCPPSyntaxFunctionDefinition*)pcDeclaration, NULL);
			break;
		case CPPS_SimpleDeclaration:
			bResult &= GlobalDeclaration((CCPPSyntaxSimpleDeclaration*)pcDeclaration);
			break;
		case CPPS_LinkageSpecification:
			pcLinkage = (CCPPSyntaxLinkageSpecification*)pcDeclaration;
			bResult &= Declarations(&pcLinkage->macpDeclarations);
			break;
		case CPPS_TemplateDeclaration:
			bResult &= TemplateDeclaration((CCPPSyntaxTemplateDeclaration*)pcDeclaration);
			break;
		case CPPS_AliasDeclaration:
		case CPPS_UsingDeclaration:
		case CPPS_UsingDirective:
			break;
		case CPPS_EmptyDeclaration:
		case CPPS_StaticAssert:
			break;
		default:
			bResult &= Error(pcDeclaration, "This declaration is not supported by the 65C64 generator yet");
			break;
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Records a function's signature, a member of psClass if it is not NULL, or an
// instance of psTemplate.  A function may be declared any number of times but
// defined once.  Functions with the same name and different parameters are
// overloads.  Returns NULL after an error.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::DeclareFunction(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, bool bDefinition, SCPP65Class* psClass, bool bPure, SCPP65Template* psTemplate, CChars* pszArguments)
{
	int64						lliName;
	SCPP65Function*				psFunction;
	SCPP65Function*				psOther;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;
	SCPP65Type*					psType;
	SCPP65Type*					psReturn;
	ECPP65FunctionKind			eKind;
	CArrayCPP65Types			apsParameters;
	CArraySyntax				apcDefaults;
	size						i;
	size						j;
	uint32						uiFlags;

	pcName = pcDeclarator->GetName();
	pcComponent = pcName ? pcName->GetLast() : NULL;
	if (!pcComponent || (pcComponent->meKind == CPPNC_Conversion) || (!psClass && !psTemplate && (pcName->mbGlobal || (pcName->macpComponents.NumElements() != 1))))
	{
		Error(pcDeclarator, "Conversion functions and qualified names are not supported by the 65C64 generator yet");
		return NULL;
	}
	lliName = GetFunctionName(pcComponent);
	if (lliName == -1)
	{
		Error(pcDeclarator, "This function name is not supported by the 65C64 generator yet");
		return NULL;
	}
	eKind = CPP65FK_Function;
	if (pcComponent->meKind == CPPNC_Destructor)
	{
		eKind = CPP65FK_Destructor;
	}
	else if (psClass && (lliName == psClass->lliName))
	{
		eKind = CPP65FK_Constructor;
	}

	pcSuffix = pcDeclarator->GetFunctionSuffix();
	if ((pcDeclarator->macpSuffixes.NumElements() != 1) || pcDeclarator->mpcNested || pcSuffix->mpcTrailingReturn)
	{
		Error(pcDeclarator, "This function declarator is not supported by the 65C64 generator yet");
		return NULL;
	}

	psReturn = &mcTypes.msVoid;
	if (eKind == CPP65FK_Function)
	{
		psReturn = GetBaseType(pcSpecifiers, pcDeclarator);
		for (i = 0; psReturn && (i < pcDeclarator->macpPrefix.NumElements()); i++)
		{
			switch (((CCPPSyntaxPointerOperator*)pcDeclarator->macpPrefix.GetPtr(i))->meKind)
			{
			case CPPP_Pointer:
				psReturn = mcTypes.Pointer(psReturn);
				break;
			case CPPP_Reference:
			case CPPP_RValueReference:
				psReturn = mcTypes.Reference(psReturn);
				break;
			default:
				Error(pcDeclarator, "Pointers to members are not supported by the 65C64 generator yet");
				psReturn = NULL;
				break;
			}
		}
		if (!psReturn)
		{
			return NULL;
		}
	}

	apsParameters.Init();
	apcDefaults.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		if (!psType)
		{
			apcDefaults.Kill();
			apsParameters.Kill();
			return NULL;
		}
		if (psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
		{
			break;
		}
		if (psType->IsArray())
		{
			psType = mcTypes.Pointer(psType->psBase);
		}
		else if (psType->IsFunction())
		{
			psType = mcTypes.Pointer(psType);
		}
		apsParameters.Add(psType);
		apcDefaults.Add(pcParameter->mpcDefault);
	}

	//A declaration of a function declared before.
	uiFlags = pcSpecifiers ? pcSpecifiers->muiFlags : 0;
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psOther = mapsFunctions.GetPtr(i);
		if ((psOther->psClass != psClass) || (psOther->lliName != lliName) || (psOther->eKind != eKind) || (psOther->mapsParameters.NumElements() != apsParameters.NumElements()) || (psOther->psTemplate != psTemplate))
		{
			continue;
		}
		if (psTemplate && !psOther->mszArguments.Equals(pszArguments))
		{
			continue;
		}
		psFunction = psOther;
		for (j = 0; j < apsParameters.NumElements(); j++)
		{
			if (!psOther->mapsParameters.GetPtr(j)->Equals(apsParameters.GetPtr(j)))
			{
				psFunction = NULL;
				break;
			}
		}
		if (!psFunction)
		{
			continue;
		}
		for (j = 0; j < apcDefaults.NumElements(); j++)
		{
			if (apcDefaults.GetPtr(j))
			{
				psFunction->mapcDefaults.SetPtr(j, apcDefaults.GetPtr(j));
			}
		}
		apcDefaults.Kill();
		apsParameters.Kill();
		psFunction->bVirtual |= (uiFlags & CPPDS_VIRTUAL) != 0;
		psFunction->bPure |= bPure;
		return psFunction;
	}

	psFunction = AddFunction(psClass, lliName, eKind, psReturn);
	for (i = 0; i < apsParameters.NumElements(); i++)
	{
		psFunction->mapsParameters.Add(apsParameters.GetPtr(i));
		psFunction->mapcDefaults.Add(apcDefaults.GetPtr(i));
	}
	apcDefaults.Kill();
	apsParameters.Kill();
	psFunction->bVariadic = pcSuffix->mbVariadic;
	psFunction->bStatic = (uiFlags & CPPDS_STATIC) != 0;
	psFunction->bVirtual = (uiFlags & CPPDS_VIRTUAL) != 0;
	psFunction->bPure = bPure;
	psFunction->bInline = (uiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR)) != 0;
	psFunction->psTemplate = psTemplate;
	if (pszArguments)
	{
		psFunction->mszArguments.Append(pszArguments);
	}
	if (eKind == CPP65FK_Constructor)
	{
		psClass->bConstructors = true;
	}
	else if (eKind == CPP65FK_Destructor)
	{
		psClass->psDestructor = psFunction;
	}
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A new function.  Its overload number counts the functions with its name before it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Function* CCPP65Generator::AddFunction(SCPP65Class* psClass, int64 lliName, ECPP65FunctionKind eKind, SCPP65Type* psReturn)
{
	SCPP65Function*	psFunction;
	SCPP65Function*	psOther;
	size			i;

	psFunction = (SCPP65Function*)malloc(sizeof(SCPP65Function));
	psFunction->lliName = lliName;
	psFunction->psReturn = psReturn;
	psFunction->mapsParameters.Init();
	psFunction->bDefined = false;
	psFunction->psClass = psClass;
	psFunction->eKind = eKind;
	psFunction->bStatic = false;
	psFunction->bVirtual = false;
	psFunction->bPure = false;
	psFunction->iSlot = -1;
	psFunction->iOverload = 0;
	psFunction->bImplicit = false;
	psFunction->mapcDefaults.Init();
	psFunction->bVariadic = false;
	psFunction->pcDefinition = NULL;
	psFunction->psBindings = psClass ? psClass->psBindings : NULL;
	psFunction->bInline = false;
	psFunction->bQueued = false;
	psFunction->bGenerated = false;
	psFunction->bCalled = false;
	psFunction->psTemplate = NULL;
	psFunction->mszArguments.Init();
	for (i = 0; i < mapsFunctions.NumElements(); i++)
	{
		psOther = mapsFunctions.GetPtr(i);
		if ((psOther->psClass == psClass) && (psOther->lliName == lliName) && (psOther->eKind == eKind) && !psOther->psTemplate)
		{
			psFunction->iOverload++;
		}
	}
	mapsFunctions.Add(psFunction);
	return psFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A function is called: it is generated later if it is defined but not generated
// yet.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::RequireFunction(SCPP65Function* psFunction)
{
	psFunction->bCalled = true;
	if (!psFunction->bGenerated && !psFunction->bQueued && (psFunction->pcDefinition || psFunction->bImplicit))
	{
		psFunction->bQueued = true;
		mapsQueue.Add(psFunction);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Notes that a label is being written.  Returns false if a unit has written it
// already.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Emit(CChars* pszLabel)
{
	if (mpaszEmitted->GetIndex(pszLabel) != ARRAY_ELEMENT_NOT_FOUND)
	{
		return false;
	}
	mpaszEmitted->Add(pszLabel);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::SameParameters(SCPP65Function* psFunction, SCPP65Function* psOther)
{
	size	i;

	if (psFunction->mapsParameters.NumElements() != psOther->mapsParameters.NumElements())
	{
		return false;
	}
	for (i = 0; i < psFunction->mapsParameters.NumElements(); i++)
	{
		if (!psFunction->mapsParameters.GetPtr(i)->Equals(psOther->mapsParameters.GetPtr(i)))
		{
			return false;
		}
	}
	return true;
}




//////////////////////////////////////////////////////////////////////////
//
// Function declarations, classes, typedefs, enums and global variables.  Global
// variables are data with constant initialisers (zero without one).  Global objects
// are zero and made by R_Initialise before main, and destroyed (last first) by
// R_Finalise after it.  A qualified name defines a static data member.  Function
// and extern declarations the generator cannot do are only errors when used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GlobalDeclaration(CCPPSyntaxSimpleDeclaration* pcDeclaration)
{
	size						i;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	SCPP65Variable*				psVariable;
	SCPP65Function*				psFunction;
	SCPP65Type*					psType;
	SCPP65Type*					psElement;
	SCPP65Place					sPlace;
	CChars						szDestroy;
	CCPPSyntaxName*				pcName;
	SCPP65Class*				psClass;
	int64						lliName;
	bool						bResult;

	pcSpecifiers = pcDeclaration->mpcSpecifiers;
	if (pcSpecifiers && (pcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return true;
	}
	if (pcSpecifiers && pcSpecifiers->mpcType && (pcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
	{
		//The class is laid out when it is first needed whole.
		BeginQuiet();
		GetBaseType(pcSpecifiers, pcDeclaration);
		EndQuiet();
	}

	bResult = true;
	for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
		if (pcInit->mpcDeclarator->IsFunction())
		{
			BeginQuiet();
			psFunction = DeclareFunction(pcSpecifiers, pcInit->mpcDeclarator, false, NULL, false);
			EndQuiet();
			if (!psFunction)
			{
				mapcFailed.Add(pcInit->mpcDeclarator);
			}
			continue;
		}

		lliName = GetName(pcInit->mpcDeclarator);
		pcName = pcInit->mpcDeclarator->GetName();
		if ((lliName == -1) && pcName && (pcName->macpComponents.NumElements() == 2))
		{
			//int CClass::siStatic = 0; defines a static data member, so the class is laid out.
			psClass = GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcInit);
			if (psClass)
			{
				CompleteClass(psClass, pcInit);
			}
		}
		psVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
		if (!pcInit->mpcSymbol || ((lliName == -1) && !(psVariable && psVariable->psClass)))
		{
			bResult &= Error(pcInit, "Only variables with simple names are supported by the 65C64 generator yet");
			continue;
		}

		if (pcSpecifiers->muiFlags & CPPDS_EXTERN)
		{
			BeginQuiet();
			psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			EndQuiet();
			if (psType && !psVariable)
			{
				AddVariable(&mapsGlobals, pcInit->mpcSymbol, psType, CPP65S_Global, 0);
			}
			continue;
		}

		psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
		if (!psType)
		{
			bResult = false;
			continue;
		}
		psElement = psType->IsArray() ? psType->psBase : psType;
		if (psElement->IsClass() && !CompleteClass(psElement->psClass, pcInit))
		{
			bResult = false;
			continue;
		}

		if (!psVariable)
		{
			psVariable = AddVariable(&mapsGlobals, pcInit->mpcSymbol, psType, CPP65S_Global, 0);
		}
		psVariable->bInternal |= !psVariable->psClass && ((pcSpecifiers->muiFlags & CPPDS_STATIC) != 0);
		if (psVariable->bDefined)
		{
			bResult &= Error(pcInit, "The variable is defined twice");
			continue;
		}
		psVariable->bDefined = true;
		psVariable->psType = psType;

		if ((psType->iBytes >= 4) || psElement->IsClass())
		{
			mszData.Append("\t\t.align 4");
			mszData.AppendNewLine();
		}
		else if (psType->iBytes == 2)
		{
			mszData.Append("\t\t.align 2");
			mszData.AppendNewLine();
		}
		VariablePlace(psVariable, &sPlace);
		AppendPlaceLabel(&mszData, &sPlace);
		mszData.Append(':');
		mszData.AppendNewLine();
		VariablePlace(psVariable, &sPlace);
		if (!psElement->IsClass() && !psType->IsReference())
		{
			miThisOffset = -1;
			bResult &= GlobalInitialiser(psType, pcInit->mpcInitializer, pcInit, &sPlace);
			continue;
		}

		AppendZeros(psType->iBytes);
		mpszOut = &mszInitialise;
		miThisOffset = -1;
		bResult &= Construct(&sPlace, NULL, pcInit->mpcInitializer, pcInit);

		szDestroy.Init();
		mpszOut = &szDestroy;
		bResult &= Destroy(&sPlace);
		mszFinalise.Insert(0, &szDestroy);
		szDestroy.Kill();
		mpszOut = &mszCode;
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the data for a global of psType at psPlace: zero, a constant, a list of
// constants or a string.  Values that are not constants are zero data and worked
// out by R_Initialise.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GlobalInitialiser(SCPP65Type* psType, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode, SCPP65Place* psPlace)
{
	int64						lliValue;
	CCPPSyntaxInitializerList*	pcList;
	CArraySyntax*				papcElements;
	size						i;
	CArrayUint8					auiBytes;
	bool						bResult;
	SCPP65Place					sElement;
	CChars*						pszOut;
	double						dValue;

	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer))
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : pcInitialiser;
	}
	if (pcInitialiser && (pcInitialiser->GetKind() == CPPS_InitializerList) && psType->IsScalar())
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
	}

	if (psType->IsArray())
	{
		bResult = true;
		if (!pcInitialiser)
		{
			i = 0;
		}
		else if ((pcInitialiser->GetKind() == CPPS_StringExpression) && (psType->psBase->iBytes == 1))
		{
			auiBytes.Init();
			bResult = DecodeString(pcInitialiser, &auiBytes);
			for (i = 0; i < auiBytes.NumElements(); i++)
			{
				AppendData(psType->psBase, auiBytes.GetValue(i));
			}
			auiBytes.Kill();
			if (i > (size)psType->iCount)
			{
				return Error(pcInitialiser, "The string is longer than the array");
			}
		}
		else if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			pcList = (CCPPSyntaxInitializerList*)pcInitialiser;
			if (pcList->macpElements.NumElements() > (size)psType->iCount)
			{
				return Error(pcInitialiser, "Too many initialisers for the array");
			}
			sElement = *psPlace;
			sElement.psType = psType->psBase;
			for (i = 0; i < pcList->macpElements.NumElements(); i++)
			{
				bResult &= GlobalInitialiser(psType->psBase, pcList->macpElements.GetPtr(i), pcNode, &sElement);
				sElement.iOffset += psType->psBase->iBytes;
			}
		}
		else
		{
			return Error(pcInitialiser, "An array needs a list of initialisers");
		}

		for (; i < (size)psType->iCount; i++)
		{
			GlobalInitialiser(psType->psBase, NULL, pcNode, psPlace);
		}
		return bResult;
	}

	if (!psType->IsScalar())
	{
		return Error(pcNode, "This type of global variable is not supported by the 65C64 generator yet");
	}
	lliValue = 0;
	if (pcInitialiser && psType->IsFloat() && FloatConstant(pcInitialiser, &dValue))
	{
		AppendData(psType, FloatBits(psType, dValue));
		return true;
	}
	if (pcInitialiser && (psType->IsFloat() || !Constant(pcInitialiser, &lliValue)))
	{
		if (mbStaticLocal)
		{
			return Error(pcInitialiser, "A static local variable needs a constant initialiser in the 65C64 generator yet");
		}
		//Not a constant: R_Initialise works it out before main.  [basic.start.init]
		AppendData(psType, 0);
		pszOut = mpszOut;
		mpszOut = &mszInitialise;
		bResult = Convert(Expression(pcInitialiser), psType, pcInitialiser);
		if (bResult)
		{
			Store(psPlace, "A", NULL);
		}
		mpszOut = pszOut;
		return bResult;
	}
	AppendData(psType, lliValue);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendData(SCPP65Type* psType, int64 lliValue)
{
	switch (psType->iBytes)
	{
	case 1:
		mszData.Append("\t\t.byte ");
		mszData.Append((int64)(lliValue & 0xFF));
		break;
	case 2:
		mszData.Append("\t\t.word ");
		mszData.Append((int64)(lliValue & 0xFFFF));
		break;
	case 8:
		mszData.Append("\t\t.quad ");
		mszData.Append(lliValue);
		break;
	case 16:
		mszData.Append("\t\t.quad ");
		mszData.Append(lliValue);
		mszData.AppendNewLine();
		mszData.Append("\t\t.quad 0");
		break;
	default:
		mszData.Append("\t\t.long ");
		mszData.Append((int64)(lliValue & 0xFFFFFFFF));
		break;
	}
	mszData.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendZeros(int iBytes)
{
	int		i;

	for (i = 0; i + 4 <= iBytes; i += 4)
	{
		mszData.Append("\t\t.long 0");
		mszData.AppendNewLine();
	}
	for (; i < iBytes; i++)
	{
		mszData.Append("\t\t.byte 0");
		mszData.AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// _name, or _Class__name for a static data member.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendGlobalLabel(CChars* psz, SCPP65Class* psClass, int64 lliName)
{
	CChars	szName;
	char*	szIdentifier;
	int		iLength;

	if (!psClass)
	{
		AppendName(psz, lliName);
		return;
	}
	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	szName.Init(szIdentifier, 0, iLength);
	AppendClassLabel(psz, psClass, szName.Text());
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::AddVariable(CArrayCPP65Variables* papsVariables, CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPP65Storage eStorage, int iOffset)
{
	SCPP65Variable*	psVariable;

	psVariable = (SCPP65Variable*)malloc(sizeof(SCPP65Variable));
	psVariable->pcSymbol = pcSymbol;
	psVariable->psType = psType;
	psVariable->eStorage = eStorage;
	psVariable->iOffset = iOffset;
	psVariable->psClass = NULL;
	psVariable->bDefined = false;
	psVariable->bUsed = false;
	psVariable->bInternal = false;
	papsVariables->Add(psVariable);
	return psVariable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::FreeVariables(CArrayCPP65Variables* papsVariables)
{
	size	i;

	for (i = 0; i < papsVariables->NumElements(); i++)
	{
		free(papsVariables->GetPtr(i));
	}
	papsVariables->ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// The local, parameter or global a symbol names.  Locals are searched first.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Variable* CCPP65Generator::GetVariable(CCPPSymbol* pcSymbol)
{
	size			i;
	SCPP65Variable*	psVariable;

	for (i = 0; i < mapsLocals.NumElements(); i++)
	{
		psVariable = mapsLocals.GetPtr(i);
		if (psVariable->pcSymbol == pcSymbol)
		{
			return psVariable;
		}
	}
	for (i = 0; i < mapsGlobals.NumElements(); i++)
	{
		psVariable = mapsGlobals.GetPtr(i);
		if (psVariable->pcSymbol == pcSymbol)
		{
			return psVariable;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type a declarator declares.  pcDeclarator may be NULL for the base type.  An
// array without a size gets it from pcInitialiser.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser)
{
	SCPP65Type*	psType;

	psType = GetBaseType(pcSpecifiers, pcNode);
	if (!psType || !pcDeclarator)
	{
		return psType;
	}
	return GetDeclaratorType(psType, pcDeclarator, pcNode, pcInitialiser);
}


//////////////////////////////////////////////////////////////////////////
//
// Applies a declarator to a type: the pointer and reference operators first, then
// the array and function suffixes from the last, then a nested declarator, as in
// int (*pf)(int) (a pointer to a function).  [dcl.meaning]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetDeclaratorType(SCPP65Type* psType, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode, CCPPSyntax* pcInitialiser)
{
	CCPPSyntaxPointerOperator*	pcPointer;
	CCPPSyntax*				pcSuffix;
	CCPPSyntaxArraySuffix*	pcArray;
	size					i;
	int64					lliCount;

	for (i = 0; i < pcDeclarator->macpPrefix.NumElements(); i++)
	{
		pcPointer = (CCPPSyntaxPointerOperator*)pcDeclarator->macpPrefix.GetPtr(i);
		switch (pcPointer->meKind)
		{
		case CPPP_Pointer:
			psType = mcTypes.Pointer(psType);
			break;
		case CPPP_Reference:
		case CPPP_RValueReference:
			psType = mcTypes.Reference(psType);
			break;
		default:
			Error(pcNode, "Pointers to members are not supported by the 65C64 generator yet");
			return NULL;
		}
	}

	for (i = pcDeclarator->macpSuffixes.NumElements(); i > 0; i--)
	{
		pcSuffix = pcDeclarator->macpSuffixes.GetPtr(i - 1);
		if (pcSuffix->GetKind() == CPPS_FunctionSuffix)
		{
			psType = GetFunctionType(psType, (CCPPSyntaxFunctionSuffix*)pcSuffix, pcNode);
			if (!psType)
			{
				return NULL;
			}
			continue;
		}

		pcArray = (CCPPSyntaxArraySuffix*)pcSuffix;
		if (!pcArray->mpcSize)
		{
			lliCount = ((i == 1) && !pcDeclarator->mpcNested) ? GetInitialiserCount(pcInitialiser) : 0;
			if (lliCount <= 0)
			{
				Error(pcNode, "An array needs a size or an initialiser");
				return NULL;
			}
		}
		else if (!Constant(pcArray->mpcSize, &lliCount) || (lliCount <= 0))
		{
			Error(pcArray->mpcSize, "An array size must be a positive constant");
			return NULL;
		}
		if (psType->IsClass() && !CompleteClass(psType->psClass, pcNode))
		{
			return NULL;
		}
		psType = mcTypes.Array(psType, (int)lliCount);
	}

	if (pcDeclarator->mpcNested)
	{
		return GetDeclaratorType(psType, pcDeclarator->mpcNested, pcNode, pcInitialiser);
	}
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// A function type: what it returns and its parameters' types.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetFunctionType(SCPP65Type* psReturn, CCPPSyntaxFunctionSuffix* pcSuffix, CCPPSyntax* pcNode)
{
	CCPPSyntaxParameter*	pcParameter;
	CArrayCPP65Types		apsParameters;
	SCPP65Type*				psType;
	size					i;

	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		if (!psType)
		{
			apsParameters.Kill();
			return NULL;
		}
		if (psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
		{
			break;
		}
		apsParameters.Add(psType->IsArray() ? mcTypes.Pointer(psType->psBase) : psType);
	}
	psType = mcTypes.Function(psReturn, &apsParameters, pcSuffix->mbVariadic);
	apsParameters.Kill();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The number of elements an initialiser gives an array without a size: the
// elements of a list, or a string's characters and its zero.  0 if it gives none.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::GetInitialiserCount(CCPPSyntax* pcInitialiser)
{
	CArrayUint8		auiBytes;
	int				iCount;

	if (!pcInitialiser)
	{
		return 0;
	}
	if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		return (int)((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements.NumElements();
	}
	if (pcInitialiser->GetKind() == CPPS_StringExpression)
	{
		auiBytes.Init();
		iCount = DecodeString(pcInitialiser, &auiBytes) ? (int)auiBytes.NumElements() + 1 : 0;
		auiBytes.Kill();
		return iCount;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The type the specifiers name.  long is 4 bytes, as on Windows.  char is signed.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetBaseType(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntax* pcNode)
{
	uint32								uiKeywords;
	bool								bSigned;
	CCPPSyntaxTypeSpecifier*			pcType;
	CCPPSyntaxName*						pcName;
	CCPPSyntaxClassSpecifier*			pcClass;
	CCPPSyntaxElaboratedTypeSpecifier*	pcElaborated;
	SCPP65Class*						psClass;
	SCPP65Binding*						psBinding;

	if (!pcSpecifiers)
	{
		return &mcTypes.msInt;
	}

	pcType = pcSpecifiers->mpcType;
	if (pcType)
	{
		if (pcType->GetKind() == CPPS_EnumSpecifier)
		{
			return &mcTypes.msInt;
		}
		if (pcType->GetKind() == CPPS_ClassSpecifier)
		{
			//The class is laid out when it is first needed whole.
			pcClass = (CCPPSyntaxClassSpecifier*)pcType;
			psClass = pcClass->mpcSymbol ? GetClass(pcClass->mpcSymbol, pcNode) : GetUnnamedClass(pcClass);
			if (!psClass->pcSpecifier)
			{
				psClass->pcSpecifier = pcClass;
			}
			return psClass->psType;
		}
		if (pcType->GetKind() == CPPS_ElaboratedTypeSpecifier)
		{
			pcElaborated = (CCPPSyntaxElaboratedTypeSpecifier*)pcType;
			if (pcElaborated->meKey == CT_enum)
			{
				return &mcTypes.msInt;
			}
			psClass = GetNameClass(pcElaborated->mpcName, pcNode);
			if (psClass)
			{
				return psClass->psType;
			}
		}
		else if (pcType->GetKind() == CPPS_NamedTypeSpecifier)
		{
			pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcType)->mpcName;
			if (pcName->mpcSymbol)
			{
				switch (pcName->mpcSymbol->meKind)
				{
				case CPPSK_Enum:
					return &mcTypes.msInt;
				case CPPSK_Typedef:
					return GetSymbolType(pcName->mpcSymbol, pcNode);
				case CPPSK_TypeParameter:
					psBinding = FindBinding(pcName->mpcSymbol);
					if (!psBinding || !psBinding->psType)
					{
						Error(pcNode, "A template parameter used outside an instantiation");
						return NULL;
					}
					return psBinding->psType;
				case CPPSK_Class:
				case CPPSK_ClassTemplate:
					psClass = GetNameClass(pcName, pcNode);
					return psClass ? psClass->psType : NULL;
				default:
					break;
				}
			}
		}
		Error(pcNode, "This type is not supported by the 65C64 generator yet");
		return NULL;
	}

	//long double is double, as MSVC.
	uiKeywords = pcSpecifiers->muiTypeKeywords;
	bSigned = (uiKeywords & CPPTK_UNSIGNED) == 0;
	if (uiKeywords & CPPTK_FLOAT)
	{
		return mcTypes.Float(4);
	}
	if (uiKeywords & CPPTK_DOUBLE)
	{
		return (pcSpecifiers->miLongs >= 1) ? mcTypes.LongDouble() : mcTypes.Float(8);
	}
	if ((pcSpecifiers->miLongs >= 2) || (uiKeywords & CPPTK_INT64))
	{
		return mcTypes.Integer(8, bSigned);
	}
	if (uiKeywords & CPPTK_AUTO)
	{
		Error(pcSpecifiers, "auto is not supported by the 65C64 generator yet");
		return NULL;
	}
	if (uiKeywords & CPPTK_VOID)
	{
		return &mcTypes.msVoid;
	}
	if (uiKeywords & CPPTK_BOOL)
	{
		return &mcTypes.msBool;
	}
	if (uiKeywords & (CPPTK_CHAR | CPPTK_INT8))
	{
		return mcTypes.Integer(1, bSigned);
	}
	if (uiKeywords & (CPPTK_WCHAR_T | CPPTK_CHAR16_T))
	{
		return mcTypes.Integer(2, false);
	}
	if (uiKeywords & CPPTK_CHAR32_T)
	{
		return mcTypes.Integer(4, false);
	}
	if (uiKeywords & (CPPTK_SHORT | CPPTK_INT16))
	{
		return mcTypes.Integer(2, bSigned);
	}
	if (uiKeywords & (CPPTK_INT | CPPTK_INT32 | CPPTK_SIGNED | CPPTK_UNSIGNED) || (pcSpecifiers->miLongs == 1))
	{
		return mcTypes.Integer(4, bSigned);
	}
	Error(pcNode, "This type is not supported by the 65C64 generator yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetTypeIdType(CCPPSyntaxTypeId* pcTypeId)
{
	return GetType(pcTypeId->mpcSpecifiers, pcTypeId->mpcDeclarator, pcTypeId);
}


//////////////////////////////////////////////////////////////////////////
//
// The type a typedef, variable or parameter symbol was declared with.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPP65Generator::GetSymbolType(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode)
{
	CCPPSyntax*					pcDeclaration;
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxParameter*		pcParameter;

	pcDeclaration = pcSymbol->mpcDeclaration;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration;
		return GetType(((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers, pcInit->mpcDeclarator, pcNode, pcInit->mpcInitializer);
	}
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_Parameter))
	{
		pcParameter = (CCPPSyntaxParameter*)pcDeclaration;
		return GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcNode);
	}
	Error(pcNode, "This declaration is not supported by the 65C64 generator yet");
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A function definition outside a class, a member of psClass if it is not NULL (or
// of the class its qualified name names).  Inline and constexpr functions are
// generated when they are called; others now.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Function(CCPPSyntaxFunctionDefinition* pcFunction, SCPP65Class* psClass)
{
	CCPPSyntaxName*		pcName;
	SCPP65Function*		psFunction;
	size				uiErrors;

	pcName = pcFunction->mpcDeclarator->GetName();
	if (!psClass && pcName && (pcName->macpComponents.NumElements() == 2))
	{
		psClass = GetComponentClass((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0), NULL, pcFunction);
		if (!psClass)
		{
			return Error(pcFunction, "The function's class has not been defined");
		}
		if (!CompleteClass(psClass, pcFunction))
		{
			return false;
		}
	}
	else if (pcName && (pcName->macpComponents.NumElements() > 2))
	{
		return Error(pcFunction, "Namespaces and nested classes are not supported by the 65C64 generator yet");
	}

	if (pcFunction->meBody == CPPFB_Skipped)
	{
		//A body skipped in a system header: only a declaration.
		BeginQuiet();
		DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, false, psClass, false);
		EndQuiet();
		return true;
	}
	psFunction = DeclareFunction(pcFunction->mpcSpecifiers, pcFunction->mpcDeclarator, true, psClass, false);
	if (!psFunction)
	{
		return false;
	}
	if (psFunction->pcDefinition || psFunction->bGenerated)
	{
		return Error(pcFunction, "The function is defined twice");
	}
	SetDefinition(psFunction, pcFunction);
	if (!psFunction->pcDefinition && !psFunction->bImplicit)
	{
		//A body skipped in a system header.
		return true;
	}
	psFunction->bInline = pcFunction->mpcSpecifiers && (pcFunction->mpcSpecifiers->muiFlags & (CPPDS_INLINE | CPPDS_CONSTEXPR));
	if (psFunction->bInline)
	{
		return true;
	}

	uiErrors = mcErrors.NumErrors();
	GenerateFunction(psFunction);
	return mcErrors.NumErrors() == uiErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// Generates a function from its definition, or a constructor or destructor the
// generator writes, with the template arguments it was made with.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::GenerateFunction(SCPP65Function* psFunction)
{
	SCPP65Bindings*	psSaved;
	CChars			szLabel;
	bool			bEmit;

	if (psFunction->bGenerated)
	{
		return;
	}
	psFunction->bGenerated = true;
	psFunction->bQueued = false;

	//Inline, template and generated functions are written by the first unit that needs them.
	if (psFunction->bInline || psFunction->bImplicit || psFunction->psTemplate)
	{
		szLabel.Init();
		AppendFunctionLabel(&szLabel, psFunction);
		bEmit = Emit(&szLabel);
		szLabel.Kill();
		if (!bEmit)
		{
			return;
		}
	}
	psSaved = mpsBindings;
	mpsBindings = psFunction->psBindings;

	if (psFunction->bImplicit)
	{
		BeginFunction(psFunction);
		if (psFunction->eKind == CPP65FK_Constructor)
		{
			ConstructorBody(psFunction, NULL);
		}
		else
		{
			SetVtable(NULL, psFunction->psClass);
		}
		EndFunction(psFunction);
	}
	else
	{
		GenerateDefinition(psFunction);
	}
	mpsBindings = psSaved;
}


//////////////////////////////////////////////////////////////////////////
//
// The code for a function's definition: its parameters and locals get their places,
// a constructor makes its bases and fields first, a destructor sets its vtable
// pointer first (EndFunction destroys its fields and bases), then the body.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GenerateDefinition(SCPP65Function* psFunction)
{
	CCPPSyntaxFunctionDefinition*	pcFunction;
	CCPPSyntaxFunctionSuffix*		pcSuffix;
	CCPPSyntaxParameter*			pcParameter;
	SCPP65Type*						psType;
	size							i;
	int								iOffset;
	bool							bResult;

	pcFunction = psFunction->pcDefinition;
	if ((pcFunction->macpMemberInitializers.NumElements() != 0) && (psFunction->eKind != CPP65FK_Constructor))
	{
		return Error(pcFunction, "Only constructors have member initialisers");
	}

	BeginFunction(psFunction);

	//Arguments take their own size on the stack, the first lowest.
	pcSuffix = pcFunction->mpcDeclarator->GetFunctionSuffix();
	iOffset = 0;
	for (i = 0; i < psFunction->mapsParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = psFunction->mapsParameters.GetPtr(i);
		if (pcParameter->mpcSymbol)
		{
			AddVariable(&mapsLocals, pcParameter->mpcSymbol, psType, CPP65S_Parameter, iOffset);
		}
		iOffset += psType->iBytes;
	}
	if (!AllocateLocals(pcFunction->mpcBody))
	{
		EndFunction(psFunction);
		return false;
	}
	miFrameBytes = (miFrameBytes + 3) & ~3;

	bResult = true;
	if (psFunction->eKind == CPP65FK_Constructor)
	{
		bResult &= ConstructorBody(psFunction, pcFunction);
	}
	else if (psFunction->eKind == CPP65FK_Destructor)
	{
		bResult &= SetVtable(NULL, psFunction->psClass);
	}
	bResult &= Statement(pcFunction->mpcBody);
	EndFunction(psFunction);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Starts generating a function's body.  A member function keeps this (from X) at
// the bottom of its frame.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::BeginFunction(SCPP65Function* psFunction)
{
	mpsFunction = psFunction;
	FreeVariables(&mapsLocals);
	mapsDestructors.ReInit();
	macJumps.ReInit();
	macGotoLabels.ReInit();
	macCases.ReInit();

	miFrameBytes = 0;
	miThisOffset = -1;
	if (psFunction->psClass && !psFunction->bStatic)
	{
		miThisOffset = 0;
		miFrameBytes = 4;
	}
	mbThisInX = true;
	miDepth = 0;
	miTemporaries = 0;
	mszBody.Clear();
	mpszOut = &mszBody;
	miReturnLabel = NewLabel();
}


//////////////////////////////////////////////////////////////////////////
//
// A function is a label, the frame, the body and the return.  V points at the frame
// (the caller's V is kept on the stack) when the function has locals or parameters:
//
//	_name:
//			psh V
//			sub S, frame
//			cpy V, S
//			cpy [1 + V], X			; this, in a member function
//			...
//	Lreturn:
//			add S, frame
//			pul V
//			ret
//
// A destructor destroys the fields and the base after the return label.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::EndFunction(SCPP65Function* psFunction)
{
	CChars	szReturn;

	mpszOut = &mszCode;

	//A return at the end of the body does not need to branch to the return.
	szReturn.Init("\t\tbra ");
	AppendLabel(&szReturn, miReturnLabel);
	szReturn.AppendNewLine();
	if (mszBody.EndsWith(szReturn.Text()))
	{
		mszBody.RemoveFromEnd(szReturn.Length());
	}
	szReturn.Kill();

	mbFramePointer = (miFrameBytes > 0) || (mapsLocals.NumElements() > 0);
	AppendFunctionLabel(&mszCode, psFunction);
	mszCode.Append(':');
	mszCode.AppendNewLine();
	if (mbFramePointer)
	{
		Line("psh V")->AppendNewLine();
		if (miFrameBytes > 0)
		{
			Line("sub S, ")->Append(miFrameBytes)->AppendNewLine();
		}
		Line("cpy V, S")->AppendNewLine();
		if (miThisOffset != -1)
		{
			Line("cpy [")->Append(1 + miThisOffset)->Append(" + V], X")->AppendNewLine();
		}
	}
	mszCode.Append(mszBody);
	Label(miReturnLabel);
	if (psFunction->eKind == CPP65FK_Destructor)
	{
		DestructorBody(psFunction);
	}
	if (mbFramePointer)
	{
		if (miFrameBytes > 0)
		{
			Line("add S, ")->Append(miFrameBytes)->AppendNewLine();
		}
		Line("pul V")->AppendNewLine();
	}
	Line("ret")->AppendNewLine();
	mszCode.AppendNewLine();

	mpsFunction = NULL;
	miThisOffset = -1;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives every local variable in the function its own place in the frame before the
// body is generated, so the frame size is known.  Places are not shared between
// blocks yet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::AllocateLocals(CCPPSyntax* pcNode)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Type*					psType;
	size						i;
	int							iAlign;
	bool						bResult;

	if (!pcNode)
	{
		return true;
	}

	if ((pcNode->GetKind() == CPPS_InitDeclarator) && pcNode->mpcParent && (pcNode->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcNode;
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcNode->mpcParent;
		if (pcInit->mpcSymbol && (pcInit->mpcSymbol->meKind == CPPSK_Variable))
		{
			if ((pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL)) == CPPDS_STATIC)
			{
				return StaticLocal(pcInit, pcDeclaration->mpcSpecifiers);
			}
			if (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL))
			{
				return Error(pcNode, "Extern and thread local variables are not supported by the 65C64 generator yet");
			}
			psType = GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
			if (!psType)
			{
				return false;
			}
			if (!psType->IsScalar() && !psType->IsArray() && !psType->IsClass() && !psType->IsReference())
			{
				return Error(pcNode, "This type of local variable is not supported by the 65C64 generator yet");
			}
			if ((psType->IsArray() ? psType->psBase : psType)->IsClass() && !CompleteClass((psType->IsArray() ? psType->psBase : psType)->psClass, pcNode))
			{
				return false;
			}

			iAlign = (psType->iBytes >= 4) ? 4 : psType->iBytes;
			iAlign = (psType->IsArray() || psType->IsClass()) ? 4 : iAlign;
			miFrameBytes = (miFrameBytes + iAlign - 1) & ~(iAlign - 1);
			AddVariable(&mapsLocals, pcInit->mpcSymbol, psType, CPP65S_Local, miFrameBytes);
			miFrameBytes += psType->iBytes;
		}
	}

	bResult = true;
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		bResult &= AllocateLocals(pcNode->GetChild(i));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A static local variable is data with its own label, initialised in the data.  Its
// initialiser must be constant; one that is not (it would be worked out the first
// time the declaration is reached) is not supported yet, nor are objects.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::StaticLocal(CCPPSyntaxInitDeclarator* pcInit, CCPPSyntaxDeclSpecifiers* pcSpecifiers)
{
	SCPP65Variable*		psVariable;
	SCPP65Type*			psType;
	SCPP65Place			sPlace;
	bool				bResult;
	int					iThisOffset;

	psType = GetType(pcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	if (!psType)
	{
		return false;
	}
	if ((psType->IsArray() ? psType->psBase : psType)->IsClass() || psType->IsReference())
	{
		return Error(pcInit, "Static local objects and references are not supported by the 65C64 generator yet");
	}

	psVariable = AddVariable(&mapsLocals, pcInit->mpcSymbol, psType, CPP65S_Static, miStatics);
	miStatics++;
	psVariable->bDefined = true;
	if (psType->iBytes >= 4)
	{
		mszData.Append("\t\t.align 4");
		mszData.AppendNewLine();
	}
	else if (psType->iBytes == 2)
	{
		mszData.Append("\t\t.align 2");
		mszData.AppendNewLine();
	}
	VariablePlace(psVariable, &sPlace);
	AppendPlaceLabel(&mszData, &sPlace);
	mszData.Append(':');
	mszData.AppendNewLine();

	iThisOffset = miThisOffset;
	miThisOffset = -1;
	mbStaticLocal = true;
	bResult = GlobalInitialiser(psType, pcInit->mpcInitializer, pcInit, &sPlace);
	mbStaticLocal = false;
	miThisOffset = iThisOffset;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The label of a Global place: a global, a static data member or a static local
// (unit, S, its number, _ and its name).  A static global has the unit in front.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendPlaceLabel(CChars* psz, SCPP65Place* psPlace)
{
	char*	szName;
	int		iLength;

	if ((psPlace->lliName != -1) && (psPlace->iString != -1))
	{
		szName = mpcTokeniser->GetIdentifier(psPlace->lliName, &iLength);
		psz->Append(&mszUnit);
		psz->Append('S');
		psz->Append(psPlace->iString);
		psz->Append('_');
		psz->Append(szName, iLength);
		return;
	}
	if (psPlace->bInternal)
	{
		psz->Append(&mszUnit);
	}
	AppendGlobalLabel(psz, psPlace->psClass, psPlace->lliName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::Statement(CCPPSyntax* pcStatement)
{
	CCPPSyntaxExpressionStatement*	pcExpression;

	if (!pcStatement)
	{
		return true;
	}

	switch (pcStatement->GetKind())
	{
	case CPPS_CompoundStatement:
		return CompoundStatement((CCPPSyntaxCompoundStatement*)pcStatement);
	case CPPS_DeclarationStatement:
		return DeclarationStatement((CCPPSyntaxDeclarationStatement*)pcStatement);
	case CPPS_ExpressionStatement:
		pcExpression = (CCPPSyntaxExpressionStatement*)pcStatement;
		if (pcExpression->mpcExpression)
		{
			return Expression(pcExpression->mpcExpression) != NULL;
		}
		return true;
	case CPPS_IfStatement:
		return IfStatement((CCPPSyntaxIfStatement*)pcStatement);
	case CPPS_WhileStatement:
		return WhileStatement((CCPPSyntaxWhileStatement*)pcStatement);
	case CPPS_DoStatement:
		return DoStatement((CCPPSyntaxDoStatement*)pcStatement);
	case CPPS_ForStatement:
		return ForStatement((CCPPSyntaxForStatement*)pcStatement);
	case CPPS_SwitchStatement:
		return SwitchStatement((CCPPSyntaxSwitchStatement*)pcStatement);
	case CPPS_CaseStatement:
		return CaseStatement((CCPPSyntaxCaseStatement*)pcStatement);
	case CPPS_ReturnStatement:
		return ReturnStatement((CCPPSyntaxReturnStatement*)pcStatement);
	case CPPS_BreakStatement:
		return BreakStatement(pcStatement, false);
	case CPPS_ContinueStatement:
		return BreakStatement(pcStatement, true);
	case CPPS_GotoStatement:
		return GotoStatement((CCPPSyntaxGotoStatement*)pcStatement);
	case CPPS_LabelStatement:
		return LabelStatement((CCPPSyntaxLabelStatement*)pcStatement);
	default:
		return Error(pcStatement, "This statement is not supported by the 65C64 generator yet");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// goto name: a branch to the label's assembly label.  Jumping past objects that
// need destroying is not supported yet.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::GotoStatement(CCPPSyntaxGotoStatement* pcGoto)
{
	int		iLabel;

	if (mapsDestructors.NumElements() > 0)
	{
		return Error(pcGoto, "goto where objects need destroying is not supported by the 65C64 generator yet");
	}
	iLabel = GotoLabel(pcGoto->miLabel);
	if (iLabel == -1)
	{
		return Error(pcGoto, "goto needs a label");
	}
	Line("bra ");
	AppendLabel(mpszOut, iLabel);
	mpszOut->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// name: statement.  X is reloaded after a label, as after any other.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LabelStatement(CCPPSyntaxLabelStatement* pcLabel)
{
	int		iLabel;

	if (mapsDestructors.NumElements() > 0)
	{
		return Error(pcLabel, "Labels where objects need destroying are not supported by the 65C64 generator yet");
	}
	iLabel = GotoLabel(pcLabel->miName);
	if (iLabel == -1)
	{
		return Error(pcLabel, "A label needs a name");
	}
	Label(iLabel);
	return Statement(pcLabel->mpcStatement);
}


//////////////////////////////////////////////////////////////////////////
//
// The assembly label for the goto label named by the token, given the first time
// it is seen (by a goto or by the label).  -1 if the token is not a name.
//
//////////////////////////////////////////////////////////////////////////
int CCPP65Generator::GotoLabel(int iToken)
{
	CCPPToken*			pcToken;
	SCPP65GotoLabel*	psLabel;
	int64				lliName;
	size				i;

	pcToken = mpcTokeniser->Get(iToken);
	if (!pcToken || !pcToken->IsIdentifier())
	{
		return -1;
	}
	lliName = ((CCTIdentifier*)pcToken)->mlliIdentifier;
	for (i = 0; i < macGotoLabels.NumElements(); i++)
	{
		if (macGotoLabels.Get(i)->lliName == lliName)
		{
			return macGotoLabels.Get(i)->iLabel;
		}
	}
	psLabel = macGotoLabels.Add();
	psLabel->lliName = lliName;
	psLabel->iLabel = NewLabel();
	return psLabel->iLabel;
}


//////////////////////////////////////////////////////////////////////////
//
// The objects made in a block are destroyed at its end, last first.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CompoundStatement(CCPPSyntaxCompoundStatement* pcCompound)
{
	size	i;
	int		iFirst;
	bool	bResult;

	iFirst = (int)mapsDestructors.NumElements();
	bResult = true;
	for (i = 0; i < pcCompound->macpStatements.NumElements(); i++)
	{
		bResult &= Statement(pcCompound->macpStatements.GetPtr(i));
	}
	DestroyLocals(iFirst);
	while ((int)mapsDestructors.NumElements() > iFirst)
	{
		mapsDestructors.Pop();
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Destroys the local objects from the last made back to iFirst.  They stay in the
// list: break, continue and return destroy them on their way out of a block.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::DestroyLocals(int iFirst)
{
	SCPP65Place	sPlace;
	int			i;

	for (i = (int)mapsDestructors.NumElements(); i > iFirst; i--)
	{
		VariablePlace(mapsDestructors.GetPtr(i - 1), &sPlace);
		Destroy(&sPlace);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65Jump* CCPP65Generator::PushJump(int iBreak, int iContinue)
{
	SCPP65Jump*	psJump;


	psJump = macJumps.Add();
	psJump->iBreak = iBreak;
	psJump->iContinue = iContinue;
	psJump->iDestructors = (int)mapsDestructors.NumElements();
	return psJump;
}


//////////////////////////////////////////////////////////////////////////
//
// Local variables already have their places; this runs their initialisers.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DeclarationStatement(CCPPSyntaxDeclarationStatement* pcStatement)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	SCPP65Variable*					psVariable;
	SCPP65Type*						psElement;
	SCPP65Place						sPlace;
	size							i;
	bool							bResult;

	if (!pcStatement->mpcDeclaration || (pcStatement->mpcDeclaration->GetKind() != CPPS_SimpleDeclaration))
	{
		return Error(pcStatement, "This declaration is not supported by the 65C64 generator yet");
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcStatement->mpcDeclaration;
	if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_TYPEDEF))
	{
		return true;
	}
	if (pcDeclaration->mpcSpecifiers && pcDeclaration->mpcSpecifiers->mpcType && (pcDeclaration->mpcSpecifiers->mpcType->GetKind() == CPPS_ClassSpecifier))
	{
		return Error(pcStatement, "Local classes are not supported by the 65C64 generator yet");
	}

	bResult = true;
	for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
		if (pcInit->mpcDeclarator->IsFunction())
		{
			bResult &= DeclareFunction(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, false, NULL, false) != NULL;
			continue;
		}
		psVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
		if (!psVariable)
		{
			bResult &= Error(pcInit, "This declaration is not supported by the 65C64 generator yet");
			continue;
		}
		if (psVariable->eStorage == CPP65S_Static)
		{
			//Initialised in its data.
			continue;
		}
		psElement = psVariable->psType->IsArray() ? psVariable->psType->psBase : psVariable->psType;
		if (psElement->IsClass() || psElement->IsReference())
		{
			VariablePlace(psVariable, &sPlace);
			bResult &= Construct(&sPlace, NULL, pcInit->mpcInitializer, pcInit);
			if (psElement->IsClass() && psElement->psClass->psDestructor)
			{
				mapsDestructors.Add(psVariable);
			}
		}
		else if (pcInit->mpcInitializer)
		{
			bResult &= LocalInitialiser(psVariable, pcInit->mpcInitializer, pcInit);
		}
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Stores the initial value of a local.  An array's elements without an initialiser
// are zero.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::LocalInitialiser(SCPP65Variable* psVariable, CCPPSyntax* pcInitialiser, CCPPSyntax* pcNode)
{
	SCPP65Place		sPlace;
	SCPP65Type*		psType;
	SCPP65Type*		psElement;
	CArraySyntax*	papcElements;
	CArrayUint8		auiBytes;
	int				iCount;

	VariablePlace(psVariable, &sPlace);
	psType = psVariable->psType;
	if (pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer)
	{
		papcElements = &((CCPPSyntaxParenthesisedInitializer*)pcInitialiser)->macpElements;
		if (papcElements->NumElements() != 1)
		{
			return Error(pcInitialiser, "Expected one initialiser");
		}
		pcInitialiser = papcElements->GetPtr(0);
	}

	if (psType->IsScalar())
	{
		if (pcInitialiser->GetKind() == CPPS_InitializerList)
		{
			papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
			if (papcElements->NumElements() == 0)
			{
				Line("cpy A, 0")->AppendNewLine();
				Store(&sPlace, "A", NULL);
				return true;
			}
			if (papcElements->NumElements() != 1)
			{
				return Error(pcInitialiser, "Expected one initialiser");
			}
			pcInitialiser = papcElements->GetPtr(0);
		}
		if (!Convert(Expression(pcInitialiser), psType, pcInitialiser))
		{
			return false;
		}
		Store(&sPlace, "A", NULL);
		return true;
	}

	psElement = psType->psBase;
	if (!psElement->IsScalar())
	{
		return Error(pcNode, "Arrays of arrays cannot be initialised by the 65C64 generator yet");
	}
	sPlace.psType = psElement;
	iCount = 0;
	if ((pcInitialiser->GetKind() == CPPS_StringExpression) && (psElement->iBytes == 1))
	{
		auiBytes.Init();
		if (!DecodeString(pcInitialiser, &auiBytes))
		{
			auiBytes.Kill();
			return false;
		}
		if (auiBytes.NumElements() > (size)psType->iCount)
		{
			auiBytes.Kill();
			return Error(pcInitialiser, "The string is longer than the array");
		}
		for (iCount = 0; iCount < (int)auiBytes.NumElements(); iCount++)
		{
			Line("cpy A, ")->Append((int)auiBytes.GetValue(iCount))->AppendNewLine();
			Store(&sPlace, "A", NULL);
			sPlace.iOffset++;
		}
		auiBytes.Kill();
	}
	else if (pcInitialiser->GetKind() == CPPS_InitializerList)
	{
		papcElements = &((CCPPSyntaxInitializerList*)pcInitialiser)->macpElements;
		if (papcElements->NumElements() > (size)psType->iCount)
		{
			return Error(pcInitialiser, "Too many initialisers for the array");
		}
		for (iCount = 0; iCount < (int)papcElements->NumElements(); iCount++)
		{
			if (!Convert(Expression(papcElements->GetPtr(iCount)), psElement, papcElements->GetPtr(iCount)))
			{
				return false;
			}
			Store(&sPlace, "A", NULL);
			sPlace.iOffset += psElement->iBytes;
		}
	}
	else
	{
		return Error(pcInitialiser, "An array needs a list of initialisers");
	}

	if (iCount < psType->iCount)
	{
		Line("cpy A, 0")->AppendNewLine();
		for (; iCount < psType->iCount; iCount++)
		{
			Store(&sPlace, "A", NULL);
			sPlace.iOffset += psElement->iBytes;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a condition's declaration (if (int x = f())), leaving the variable's value in
// A.  Returns false if pcCondition is not a declaration.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ConditionDeclaration(CCPPSyntax* pcCondition)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	SCPP65Variable*					psVariable;

	if (!pcCondition || (pcCondition->GetKind() != CPPS_SimpleDeclaration))
	{
		return false;
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcCondition;
	pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(0);
	psVariable = pcInit->mpcSymbol ? GetVariable(pcInit->mpcSymbol) : NULL;
	return psVariable && pcInit->mpcInitializer && LocalInitialiser(psVariable, pcInit->mpcInitializer, pcInit);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IfStatement(CCPPSyntaxIfStatement* pcIf)
{
	int		iElse;
	int		iEnd;
	bool	bResult;

	iElse = NewLabel();
	bResult = Condition(pcIf->mpcCondition, false, iElse);
	bResult &= Statement(pcIf->mpcThen);
	if (pcIf->mpcElse)
	{
		iEnd = NewLabel();
		Line("bra ");
		AppendLabel(mpszOut, iEnd);
		mpszOut->AppendNewLine();
		Label(iElse);
		bResult &= Statement(pcIf->mpcElse);
		Label(iEnd);
	}
	else
	{
		Label(iElse);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The condition is tested at the bottom so a loop is one branch a time round.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::WhileStatement(CCPPSyntaxWhileStatement* pcWhile)
{
	int			iBody;
	int			iTest;
	int			iEnd;
	bool		bResult;

	iBody = NewLabel();
	iTest = NewLabel();
	iEnd = NewLabel();
	Line("bra ");
	AppendLabel(mpszOut, iTest);
	mpszOut->AppendNewLine();
	Label(iBody);

	PushJump(iEnd, iTest);
	bResult = Statement(pcWhile->mpcBody);
	macJumps.Pop();

	Label(iTest);
	bResult &= Condition(pcWhile->mpcCondition, true, iBody);
	Label(iEnd);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::DoStatement(CCPPSyntaxDoStatement* pcDo)
{
	int			iBody;
	int			iTest;
	int			iEnd;
	bool		bResult;

	iBody = NewLabel();
	iTest = NewLabel();
	iEnd = NewLabel();
	Label(iBody);

	PushJump(iEnd, iTest);
	bResult = Statement(pcDo->mpcBody);
	macJumps.Pop();

	Label(iTest);
	bResult &= Condition(pcDo->mpcCondition, true, iBody);
	Label(iEnd);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ForStatement(CCPPSyntaxForStatement* pcFor)
{
	int			iBody;
	int			iNext;
	int			iTest;
	int			iEnd;
	bool		bResult;

	iBody = NewLabel();
	iNext = NewLabel();
	iTest = NewLabel();
	iEnd = NewLabel();
	bResult = Statement(pcFor->mpcInitialiser);
	Line("bra ");
	AppendLabel(mpszOut, iTest);
	mpszOut->AppendNewLine();
	Label(iBody);

	PushJump(iEnd, iNext);
	bResult &= Statement(pcFor->mpcBody);
	macJumps.Pop();

	Label(iNext);
	if (pcFor->mpcIncrement)
	{
		bResult &= Expression(pcFor->mpcIncrement) != NULL;
	}
	Label(iTest);
	if (pcFor->mpcCondition)
	{
		bResult &= Condition(pcFor->mpcCondition, true, iBody);
	}
	else
	{
		Line("bra ");
		AppendLabel(mpszOut, iBody);
		mpszOut->AppendNewLine();
	}
	Label(iEnd);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A switch compares the value with each case in turn.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::SwitchStatement(CCPPSyntaxSwitchStatement* pcSwitch)
{
	SCPP65Case*		psCase;
	SCPP65Type*		psType;
	size			uiFirst;
	size			i;
	int				iDefault;
	int				iEnd;
	int64			lliValue;
	bool			bResult;

	if (ConditionDeclaration(pcSwitch->mpcCondition))
	{
		psType = &mcTypes.msInt;
	}
	else
	{
		psType = Expression(pcSwitch->mpcCondition);
	}
	if (!psType)
	{
		return false;
	}
	if (!psType->IsInteger())
	{
		return Error(pcSwitch->mpcCondition, "A switch needs an integer");
	}

	iEnd = NewLabel();
	iDefault = -1;
	uiFirst = macCases.NumElements();
	bResult = FindCases(pcSwitch->mpcBody, &iDefault);
	for (i = uiFirst; i < macCases.NumElements(); i++)
	{
		psCase = macCases.Get(i);
		if (!psCase->pcCase->mpcValue)
		{
			continue;
		}
		if (!Constant(psCase->pcCase->mpcValue, &lliValue))
		{
			bResult &= Error(psCase->pcCase->mpcValue, "A case needs a constant");
			continue;
		}
		Line("cmp A, ")->Append((int64)(int32)lliValue)->AppendNewLine();
		Line("bras z, ");
		AppendLabel(mpszOut, psCase->iLabel);
		mpszOut->AppendNewLine();
	}
	Line("bra ");
	AppendLabel(mpszOut, (iDefault != -1) ? iDefault : iEnd);
	mpszOut->AppendNewLine();

	PushJump(iEnd, -1);
	bResult &= Statement(pcSwitch->mpcBody);
	macJumps.Pop();
	Label(iEnd);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives the case and default labels of a switch (not of switches inside it) their
// assembly labels.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::FindCases(CCPPSyntax* pcNode, int* piDefault)
{
	SCPP65Case*		psCase;
	size			i;
	bool			bResult;

	if (!pcNode || (pcNode->GetKind() == CPPS_SwitchStatement))
	{
		return true;
	}

	bResult = true;
	if (pcNode->GetKind() == CPPS_CaseStatement)
	{
		psCase = macCases.Add();
		psCase->pcCase = (CCPPSyntaxCaseStatement*)pcNode;
		psCase->iLabel = NewLabel();
		if (!psCase->pcCase->mpcValue)
		{
			if (*piDefault != -1)
			{
				bResult = Error(pcNode, "A switch can only have one default");
			}
			*piDefault = psCase->iLabel;
		}
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		bResult &= FindCases(pcNode->GetChild(i), piDefault);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::CaseStatement(CCPPSyntaxCaseStatement* pcCase)
{
	size	i;

	for (i = 0; i < macCases.NumElements(); i++)
	{
		if (macCases.Get(i)->pcCase == pcCase)
		{
			Label(macCases.Get(i)->iLabel);
			return Statement(pcCase->mpcStatement);
		}
	}
	return Error(pcCase, "A case outside a switch");
}


//////////////////////////////////////////////////////////////////////////
//
// The value is returned in A.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::ReturnStatement(CCPPSyntaxReturnStatement* pcReturn)
{
	SCPP65Type*	psType;
	SCPP65Place	sPlace;

	if (pcReturn->mpcValue && mpsFunction->psReturn->IsReference())
	{
		//A reference returns the object's address.
		if (!Place(pcReturn->mpcValue, &sPlace))
		{
			return false;
		}
		LoadAddress(&sPlace);
	}
	else if (pcReturn->mpcValue)
	{
		if (mpsFunction->psReturn->IsVoid())
		{
			psType = Expression(pcReturn->mpcValue);
			if (!psType)
			{
				return false;
			}
		}
		else if (!Convert(Expression(pcReturn->mpcValue), mpsFunction->psReturn, pcReturn->mpcValue))
		{
			return false;
		}
	}
	else if (!mpsFunction->psReturn->IsVoid())
	{
		return Error(pcReturn, "The function must return a value");
	}

	//The local objects are destroyed after the value is worked out.
	if (mapsDestructors.NumElements() > 0)
	{
		PushWide("A", mpsFunction->psReturn->Is64());
		DestroyLocals(0);
		PullWide("A", mpsFunction->psReturn->Is64());
	}
	Line("bra ");
	AppendLabel(mpszOut, miReturnLabel);
	mpszOut->AppendNewLine();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::BreakStatement(CCPPSyntax* pcStatement, bool bContinue)
{
	size	i;
	int		iLabel;

	for (i = macJumps.NumElements(); i > 0; i--)
	{
		iLabel = bContinue ? macJumps.Get(i - 1)->iContinue : macJumps.Get(i - 1)->iBreak;
		if (iLabel != -1)
		{
			DestroyLocals(macJumps.Get(i - 1)->iDestructors);
			Line("bra ");
			AppendLabel(mpszOut, iLabel);
			mpszOut->AppendNewLine();
			return true;
		}
	}
	return Error(pcStatement, bContinue ? "continue outside a loop" : "break outside a loop or switch");
}


//////////////////////////////////////////////////////////////////////////
//
// String literals are Str and a number, zero terminated.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendStrings(CChars* pszDest)
{
	size	i;

	for (i = 0; i < maszStrings.NumElements(); i++)
	{
		pszDest->Append(&mszUnit);
		pszDest->Append("Str");
		pszDest->Append((int)i);
		pszDest->Append(':');
		pszDest->AppendNewLine();
		pszDest->Append("\t\t.byte ");
		pszDest->Append(maszStrings.Get(i));
		pszDest->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Signed divide: A / B.  The quotient is in A and the remainder in B.  The quotient
// is negative when the signs differ, the remainder has the dividend's sign.  The
// 64 bit routine is called with A and B 64 bits wide and leaves them so; it widens
// X and Y while it works.  Changes A, B, X and Y.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendDivideSigned(const char* szName, int iBytes)
{
	mszCode.Append(szName);
	mszCode.Append(':');
	mszCode.AppendNewLine();
	if (iBytes == 8)
	{
		Line("wtth (X, Y), 8")->AppendNewLine();
	}
	Line("cpy X, A")->AppendNewLine();
	Line("xor X, B")->AppendNewLine();
	Line("psh X")->AppendNewLine();
	Line("psh A")->AppendNewLine();
	Line("abs Y, A")->AppendNewLine();
	Line("abs A, B")->AppendNewLine();
	Line("cpy B, 0")->AppendNewLine();
	Line("psh W")->AppendNewLine();
	Line("cpy W, 0")->AppendNewLine();
	Line("div")->AppendNewLine();
	Line("pul W")->AppendNewLine();
	Line("pul X")->AppendNewLine();
	Line("cmp X, 0")->AppendNewLine();
	Line("brac n, ")->Append(szName)->Append('1')->AppendNewLine();
	Line("neg B, B")->AppendNewLine();
	mszCode.Append(szName);
	mszCode.Append("1:");
	mszCode.AppendNewLine();
	Line("pul X")->AppendNewLine();
	Line("cmp X, 0")->AppendNewLine();
	Line("brac n, ")->Append(szName)->Append('2')->AppendNewLine();
	Line("neg Y, Y")->AppendNewLine();
	mszCode.Append(szName);
	mszCode.Append("2:");
	mszCode.AppendNewLine();
	Line("cpy A, Y")->AppendNewLine();
	if (iBytes == 8)
	{
		Line("wtth (X, Y), 4")->AppendNewLine();
	}
	Line("ret")->AppendNewLine();
	mszCode.AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// Runtime routines the generated code calls.  Only the ones used are added.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Generator::AppendRuntime(void)
{
	CChars	szLabel;
	bool	bPure;
	bool	bDivide;

	mpszOut = &mszCode;
	szLabel.Init("R_Pure");
	bPure = mbPureVirtual && Emit(&szLabel);
	szLabel.Kill();
	szLabel.Init("R_DivideSigned");
	bDivide = (mbSignedDivide || mbSignedModulus) && Emit(&szLabel);
	szLabel.Kill();
	if (bPure)
	{
		//A pure virtual function was called.
		mszCode.Append("R_Pure:");
		mszCode.AppendNewLine();
		Line("stp")->AppendNewLine();
		mszCode.AppendNewLine();
	}
	if (bDivide)
	{
		AppendDivideSigned("R_DivideSigned", 4);
	}
	szLabel.Init("R_DivideSigned64");
	bDivide = mbSignedDivide64 && Emit(&szLabel);
	szLabel.Kill();
	if (bDivide)
	{
		AppendDivideSigned("R_DivideSigned64", 8);
	}
}
