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
#include <string.h>
#include "SourceFile.h"
#include "CppTokeniser.h"
#include "CPPUnit.h"
#include "CPPTypeWriter.h"
#include "IRBackend.h"
#include "CPP65DebugWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	masTypes.Init();
	mcIndices.Init();
	mcKeys.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::Kill(void)
{
	size				ui;
	SCPP65DebugType*	psType;

	for (ui = 0; ui < masTypes.NumElements(); ui++)
	{
		psType = masTypes.Get(ui);
		psType->szKey.Kill();
		psType->szName.Kill();
		psType->szMembers.Kill();
	}
	mcKeys.Kill();
	mcIndices.Kill();
	masTypes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's index of psType (without its const and volatile), described
// (with what it needs) the first time it is asked for; -1 for none.
//
//////////////////////////////////////////////////////////////////////////
int32 CCPP65DebugWriter::TypeIndex(SCPP65Type* psType)
{
	int32*				piIndex;
	int32				iIndex;
	SCPP65DebugType*	psDebug;
	CChars				szKey;

	if (!psType)
	{
		return -1;
	}
	psType = psType->Unqualified();
	piIndex = mcIndices.Get((int64)(size)psType);
	if (piIndex)
	{
		return *piIndex;
	}
	szKey.Init();
	AppendKey(psType, &szKey);
	piIndex = mcKeys.Get(szKey.Text());
	if (piIndex)
	{
		iIndex = *piIndex;
		mcIndices.Put((int64)(size)psType, &iIndex);
		szKey.Kill();
		return iIndex;
	}

	//Its index before what it needs is described: a class pointing at itself names it.
	iIndex = (int32)masTypes.NumElements();
	psDebug = masTypes.Add();
	psDebug->szKey.Init();
	psDebug->szName.Init();
	psDebug->szMembers.Init();
	mcIndices.Put((int64)(size)psType, &iIndex);
	mcKeys.Put(szKey.Text(), &iIndex);
	Describe(iIndex, psType, &szKey);
	szKey.Kill();
	return iIndex;
}


//////////////////////////////////////////////////////////////////////////
//
// The record of type iIndex: its key, name, kind, size and what it is of
// or has.  masTypes may grow while its target and members are described,
// so the record is found again after.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::Describe(int32 iIndex, SCPP65Type* psType, CChars* pszKey)
{
	CCPPTypeWriter		cWriter;
	CChars				szName;
	CChars				szMembers;
	const char*			szKind;
	uint8				uiFlags;
	int32				iTarget;
	int					iCount;
	SCPP65DebugType*	psDebug;

	szName.Init();
	cWriter.Init(mpcUnit, CPPTW_Shown);
	cWriter.AppendType(psType, &szName);
	szMembers.Init();
	uiFlags = 0;
	iTarget = -1;
	iCount = 0;
	switch (psType->eKind)
	{
	case CPP65TK_Bool:
		szKind = "bool";
		break;
	case CPP65TK_Integer:
		if (psType->pcEnum)
		{
			szKind = "enum";
			iTarget = TypeIndex(mpcUnit->GetTypes()->IntegerOfKind(psType->eInteger));
			DescribeEnum(psType->pcEnum, &szMembers);
			break;
		}
		szKind = "integer";
		uiFlags = (psType->bSigned ? 0x01 : 0) | (((psType->eInteger >= CPP65IK_Char) && (psType->eInteger <= CPP65IK_Char32)) ? 0x02 : 0);
		break;
	case CPP65TK_Float:
		szKind = "float";
		break;
	case CPP65TK_Pointer:
		szKind = "pointer";
		iTarget = TypeIndex(psType->psBase);
		break;
	case CPP65TK_Reference:
		szKind = "reference";
		iTarget = TypeIndex(psType->psBase);
		break;
	case CPP65TK_Array:
		szKind = "array";
		iTarget = TypeIndex(psType->psBase);
		iCount = psType->iCount;
		break;
	case CPP65TK_Class:
		szKind = "class";
		if (!psType->pcClass->IsComplete())
		{
			uiFlags = 0x08;
		}
		else
		{
			uiFlags = psType->pcClass->mbUnion ? 0x04 : 0;
			DescribeClass(iIndex, psType->pcClass, &szMembers);
		}
		break;
	case CPP65TK_Function:
		szKind = "function";
		break;
	case CPP65TK_MemberPointer:
	case CPP65TK_NullPointer:
		szKind = "pointer";
		break;
	default:
		szKind = "void";
		break;
	}

	psDebug = masTypes.Get(iIndex);
	psDebug->szKey.Set(pszKey);
	psDebug->szName.Set(szName);
	psDebug->szKind = szKind;
	psDebug->uiFlags = uiFlags;
	psDebug->iBytes = ((psType->eKind == CPP65TK_Function) || (uiFlags & 0x08)) ? 0 : psType->iBytes;
	psDebug->iTarget = iTarget;
	psDebug->iCount = iCount;
	psDebug->szMembers.Set(szMembers);
	szMembers.Kill();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A complete class's bases (virtual ones where a complete object of it
// has them), its vtable pointer and its named fields.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::DescribeClass(int32 iIndex, CCPPClass* pcClass, CChars* pszMembers)
{
	CCPPTypeWriter	cWriter;
	CChars			szName;
	SCPPField*		psField;
	SCPPBase*		psBase;
	size			ui;

	cWriter.Init(mpcUnit, CPPTW_Shown);
	for (ui = 0; ui < pcClass->macBases.NumElements(); ui++)
	{
		psBase = pcClass->macBases.Get(ui);
		if (!psBase->bVirtual)
		{
			szName.Init();
			cWriter.AppendClass(psBase->pcClass, &szName);
			AppendMember(pszMembers, "base", szName.Text(), TypeIndex(psBase->pcClass->mpsType), psBase->iOffset, 0, 0);
			szName.Kill();
		}
	}
	for (ui = 0; ui < pcClass->macVirtualBases.NumElements(); ui++)
	{
		szName.Init();
		cWriter.AppendClass(pcClass->macVirtualBases.Get(ui)->pcClass, &szName);
		AppendMember(pszMembers, "base", szName.Text(), TypeIndex(pcClass->macVirtualBases.Get(ui)->pcClass->mpsType), pcClass->macVirtualBases.Get(ui)->iOffset, 0, 0);
		szName.Kill();
	}
	if (pcClass->mcVtable.HasPointer())
	{
		AppendMember(pszMembers, "vtable", "", -1, pcClass->mcVtable.miPointerOffset, 0, 0);
	}
	for (ui = 0; ui < pcClass->macFields.NumElements(); ui++)
	{
		psField = pcClass->macFields.Get(ui);
		if (psField->IsUnnamedBitField())
		{
			continue;
		}
		szName.Init();
		cWriter.AppendIdentifier(psField->lliName, &szName);
		AppendMember(pszMembers, "field", szName.Text(), TypeIndex(psField->psType), psField->iOffset, psField->IsBitField() ? psField->iBitOffset : 0, psField->IsBitField() ? psField->iBitWidth : 0);
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// An enum's enumerators, each with its value.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::DescribeEnum(CCPPSymbol* pcEnum, CChars* pszMembers)
{
	CCPPTypeWriter	cWriter;
	CChars			szName;
	SMapIterator	sIter;
	CCPPSymbol**	ppcMember;
	bool			bMore;
	int64			lliValue;

	if (!pcEnum->mpcMembers)
	{
		return;
	}
	cWriter.Init(mpcUnit, CPPTW_Shown);
	for (bMore = pcEnum->mpcMembers->mcSymbols.StartIteration(&sIter, NULL, NULL, (void**)&ppcMember, NULL); bMore; bMore = pcEnum->mpcMembers->mcSymbols.Iterate(&sIter, NULL, NULL, (void**)&ppcMember, NULL))
	{
		if (!ppcMember || !*ppcMember || ((*ppcMember)->meKind != CPPSK_Enumerator) || !mpcUnit->EnumeratorValue(*ppcMember, &lliValue))
		{
			continue;
		}
		szName.Init();
		cWriter.AppendIdentifier((*ppcMember)->mlliName, &szName);
		AppendMember(pszMembers, "enumerator", szName.Text(), -1, lliValue, 0, 0);
		szName.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What names psType the same in every unit: a class or an enum by its
// name and where it is declared (two local classes may share a name), a
// pointer, reference or array by what it is of, the rest by its name.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendKey(SCPP65Type* psType, CChars* pszDest)
{
	CCPPTypeWriter	cWriter;

	cWriter.Init(mpcUnit, CPPTW_Shown);
	switch (psType->eKind)
	{
	case CPP65TK_Pointer:
		pszDest->Append('*');
		AppendKey(psType->psBase->Unqualified(), pszDest);
		return;
	case CPP65TK_Reference:
		pszDest->Append(psType->bRvalue ? "&&" : "&");
		AppendKey(psType->psBase->Unqualified(), pszDest);
		return;
	case CPP65TK_Array:
		pszDest->Append('[')->Append(psType->iCount)->Append(']');
		AppendKey(psType->psBase->Unqualified(), pszDest);
		return;
	default:
		cWriter.AppendType(psType, pszDest);
		if (psType->pcEnum && psType->pcEnum->mpcDeclaration)
		{
			AppendPlace(psType->pcEnum->mpcDeclaration->miFirstToken, pszDest);
		}
		else if ((psType->eKind == CPP65TK_Class) && psType->pcClass->mpcSymbol && psType->pcClass->mpcSymbol->mpcDeclaration)
		{
			AppendPlace(psType->pcClass->mpcSymbol->mpcDeclaration->miFirstToken, pszDest);
		}
		return;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// @File.h(12): where a token is.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendPlace(int iToken, CChars* pszDest)
{
	SPreprocessedToken*	psSource;

	psSource = mpcUnit->GetTokeniser()->GetSource(mpcUnit->GetTokeniser()->Get(iToken));
	if (psSource && psSource->pcFile)
	{
		pszDest->Append('@')->Append(psSource->pcFile->ShortName())->Append('(')->Append(psSource->iLine + 1)->Append(')');
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendMember(CChars* pszDest, const char* szKind, const char* szName, int32 iType, int64 iValue, int iBitOffset, int iBits)
{
	pszDest->Append("\t\t.dmember ")->Append(szKind)->Append(", ");
	AppendString(pszDest, szName);
	pszDest->Append(", ")->Append(iType)->Append(", ")->Append(iValue)->Append(", ")->Append(iBitOffset)->Append(", ")->Append(iBits)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// .dvariable for a variable of the function whose code pszDest is, in
// scope from line iFirstLine to iLastLine, where szWhere ("none", "frame"
// or "indirect") at D + iOffset.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendVariable(CChars* pszDest, const char* szName, SCPP65Type* psType, bool bParameter, const char* szWhere, int32 iOffset, int iFirstLine, int iLastLine)
{
	pszDest->Append("\t\t.dvariable ")->Append(bParameter ? "parameter" : "local")->Append(", ");
	AppendString(pszDest, szName);
	pszDest->Append(", ")->Append(TypeIndex(psType))->Append(", ")->Append(szWhere)->Append(", ")->Append(iOffset)->Append(", ")->Append(iFirstLine)->Append(", ")->Append(iLastLine)->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
// .dglobal for a global (or a class's static data member) at szLabel;
// .dstatic for a static local of the function whose code pszDest is.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendGlobal(CChars* pszDest, CCPPVariable* pcVariable, const char* szLabel, bool bStatic)
{
	CCPPTypeWriter	cWriter;
	CChars			szName;

	szName.Init();
	cWriter.Init(mpcUnit, CPPTW_Shown);
	if (bStatic)
	{
		cWriter.AppendIdentifier(pcVariable->mpcSymbol->mlliName, &szName);
	}
	else
	{
		cWriter.AppendVariable(pcVariable, &szName);
	}
	pszDest->Append(bStatic ? "\t\t.dstatic " : "\t\t.dglobal ");
	AppendString(pszDest, szName.Text());
	pszDest->Append(", ")->Append(TypeIndex(pcVariable->mpsType))->Append(", ")->Append(szLabel)->AppendNewLine();
	szName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The unit's types, in index order, each with its members.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendTypes(CChars* pszDest)
{
	size				ui;
	SCPP65DebugType*	psType;

	for (ui = 0; ui < masTypes.NumElements(); ui++)
	{
		psType = masTypes.Get(ui);
		pszDest->Append("\t\t.dtype ")->Append((int)ui)->Append(", ");
		AppendString(pszDest, psType->szKey.Text());
		pszDest->Append(", ");
		AppendString(pszDest, psType->szName.Text());
		pszDest->Append(", ")->Append(psType->szKind)->Append(", ")->Append((int)psType->uiFlags)->Append(", ")->Append(psType->iBytes)->Append(", ")->Append(psType->iTarget)->Append(", ")->Append(psType->iCount)->AppendNewLine();
		pszDest->Append(psType->szMembers);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// szText as an assembler string: in quotes, with a backslash or a quote
// in it escaped ("C:\\Path\\File.cpp").
//
//////////////////////////////////////////////////////////////////////////
void CCPP65DebugWriter::AppendString(CChars* pszDest, const char* szText)
{
	pszDest->Append('"');
	for (; *szText != '\0'; szText++)
	{
		if ((*szText == '\\') || (*szText == '"'))
		{
			pszDest->Append('\\');
		}
		pszDest->Append(*szText);
	}
	pszDest->Append('"');
}


//////////////////////////////////////////////////////////////////////////
//
// .dfunction "szName" after the function's label in its code: it starts
// there, and returns the unit's type iResult (-1 for void, left out).  So
// too after each label the back end says is an entry of its own
// (pcWritten's, a register function's body).  False if the code has no
// such label.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65DebugWriter::DescribeFunction(CChars* pszCode, const char* szLabel, const char* szName, int32 iResult, CIRWrittenFunction* pcWritten)
{
	CChars	szDirective;
	size	uiAfter;
	size	ui;
	bool	bResult;

	szDirective.Init("\t\t.dfunction ");
	AppendString(&szDirective, szName);
	if (iResult >= 0)
	{
		szDirective.Append(", long, ")->Append(iResult);
	}
	szDirective.AppendNewLine();
	uiAfter = FindAfterLabel(pszCode->Text(), szLabel);
	bResult = uiAfter != MAX_SIZE;
	if (bResult)
	{
		pszCode->Insert(uiAfter, &szDirective);
	}
	for (ui = 0; pcWritten && (ui < pcWritten->NumEntries()); ui++)
	{
		uiAfter = FindAfterLabel(pszCode->Text(), pcWritten->GetEntry(ui));
		if (uiAfter != MAX_SIZE)
		{
			pszCode->Insert(uiAfter, &szDirective);
		}
	}
	szDirective.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the line after szLabel's in szCode starts (the label at the start
// of a line, then a colon), MAX_SIZE if it has none.
//
//////////////////////////////////////////////////////////////////////////
size CCPP65DebugWriter::FindAfterLabel(const char* szCode, const char* szLabel)
{
	const char*	szAt;
	size		uiLength;

	uiLength = strlen(szLabel);
	for (szAt = strstr(szCode, szLabel); szAt; szAt = strstr(szAt + 1, szLabel))
	{
		if (((szAt == szCode) || (szAt[-1] == '\n')) && (szAt[uiLength] == ':') && (szAt[uiLength + 1] == '\n'))
		{
			return (size)(szAt - szCode) + uiLength + 2;
		}
	}
	return MAX_SIZE;
}
