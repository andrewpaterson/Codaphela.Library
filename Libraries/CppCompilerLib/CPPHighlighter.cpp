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
#include <stdlib.h>
#include "SourceFile.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CPPSyntaxTranslationUnit.h"
#include "CPPHighlighter.h"


//The name of each kind of highlight, indexed by ECPPHighlight.
char* gaszHighlightKindNames[] =
{
	"comment",
	"inactive",
	"directive",
	"macro",
	"keyword",
	"operator",
	"number",
	"character",
	"string",
	"namespace",
	"class",
	"enum",
	"typedef",
	"type parameter",
	"builtin type",
	"function",
	"variable",
	"field",
	"parameter",
	"enumerator",
	"label",
	"member",
	"unresolved",
	"identifier",
};
static_assert(sizeof(gaszHighlightKindNames) / sizeof(char*) == CPPH_Identifier + 1, "gaszHighlightKindNames must have a name for every ECPPHighlight.");


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::Init(void)
{
	macHighlights.Init();
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::Kill(void)
{
	macHighlights.Kill();
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::Highlight(CSourceFile* pcFile, CPreprocessedTokens* pcPreprocessed, CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree)
{
	macHighlights.Kill();
	macHighlights.Init();
	mpcFile = pcFile;

	AddComments();
	AddInactiveLines(pcPreprocessed);
	AddDirectives(pcPreprocessed);
	AddMacros(pcPreprocessed);
	AddTokens(pcTokeniser, pcTree);
	Sort();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::AddComments(void)
{
	CArrayPPComments*	pacComments;
	SPPComment*			psComment;
	size				i;

	pacComments = mpcFile->GetComments();
	for (i = 0; i < pacComments->NumElements(); i++)
	{
		psComment = pacComments->Get(i);
		AddSpan(psComment->iStart, psComment->iLength, CPPH_Comment, 0);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The lines conditional compilation skipped.  A file included more than
// once in the unit has the lines each inclusion skipped: a header's
// include guard skips all of it every time but the first, so lines
// another inclusion compiled aren't shown as skipped.
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::AddInactiveLines(CPreprocessedTokens* pcPreprocessed)
{
	SInactiveRegion*	psRegion;
	size				i;
	int					iLine;

	for (i = 0; i < pcPreprocessed->NumInactiveRegions(); i++)
	{
		psRegion = pcPreprocessed->GetInactiveRegion(i);
		if ((psRegion->pcFile == mpcFile) && !IsCompiledElsewhere(pcPreprocessed, psRegion))
		{
			for (iLine = psRegion->iStartLine; iLine <= psRegion->iEndLine; iLine++)
			{
				Add(iLine, 0, LineLength(iLine), CPPH_Inactive, 0);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether any of a skipped region's lines was compiled by another
// inclusion of its file: a token, or a directive that was obeyed, is on
// one of them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPHighlighter::IsCompiledElsewhere(CPreprocessedTokens* pcPreprocessed, SInactiveRegion* psRegion)
{
	SPreprocessedToken*	psToken;
	SDirectiveLine*		psDirective;
	size				i;

	for (i = 0; i < pcPreprocessed->NumDirectiveLines(); i++)
	{
		psDirective = pcPreprocessed->GetDirectiveLine(i);
		if (psDirective->bActive && (psDirective->pcFile == psRegion->pcFile) && (psDirective->iLine >= psRegion->iStartLine) && (psDirective->iLine <= psRegion->iEndLine))
		{
			return true;
		}
	}
	for (i = 0; i < pcPreprocessed->NumTokens(); i++)
	{
		psToken = pcPreprocessed->Get(i);
		if ((psToken->pcFile == psRegion->pcFile) && (psToken->iLine >= psRegion->iStartLine) && (psToken->iLine <= psRegion->iEndLine))
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
void CCPPHighlighter::AddDirectives(CPreprocessedTokens* pcPreprocessed)
{
	SDirectiveLine*	psLine;
	size			i;

	for (i = 0; i < pcPreprocessed->NumDirectiveLines(); i++)
	{
		psLine = pcPreprocessed->GetDirectiveLine(i);
		if ((psLine->pcFile == mpcFile) && psLine->bActive)
		{
			Add(psLine->iLine, 0, LineLength(psLine->iLine), CPPH_Directive, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::AddMacros(CPreprocessedTokens* pcPreprocessed)
{
	SMacroExpansion*	psExpansion;
	size				i;

	//Macros whose names are written in this file.  Names in other macros' bodies are spelled in the #define.
	for (i = 0; i < pcPreprocessed->NumExpansions(); i++)
	{
		psExpansion = pcPreprocessed->GetExpansion(i);
		if ((psExpansion->pcFile == mpcFile) && (psExpansion->iParent == -1))
		{
			Add(psExpansion->iLine, psExpansion->iColumn, (int)psExpansion->uiNameLength, CPPH_Macro, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::AddTokens(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree)
{
	size					i;
	CCPPToken*				pcToken;
	SPreprocessedToken*		psSource;
	ECPPHighlight			eKind;
	ECppTokens				eToken;
	SCPPTokenSymbol*		psSymbol;
	uint8					uiFlags;
	int						iOffset;

	for (i = 0; i < pcTokeniser->NumTokens(); i++)
	{
		pcToken = pcTokeniser->Get(i);
		psSource = pcTokeniser->GetSource(pcToken);

		//Only tokens written in this file.  Tokens from macro bodies are highlighted as the macro's name.
		if ((psSource->pcFile != mpcFile) || (psSource->iExpansion != -1))
		{
			continue;
		}

		uiFlags = 0;
		if (pcToken->IsIdentifier())
		{
			eKind = GetIdentifierKind(pcTokeniser, pcTree, i);
			psSymbol = pcTree->GetTokenSymbol((int)i);
			if (psSymbol && (psSymbol->uiRole == CPPTR_Declaration))
			{
				uiFlags = CPPHF_DECLARATION;
			}
		}
		else if (pcToken->IsReservedWord())
		{
			eToken = ((CCTReservedWord*)pcToken)->GetToken();
			eKind = (eToken >= CT_alignas) ? CPPH_Keyword : CPPH_Operator;
		}
		else if (pcToken->IsBool())
		{
			eKind = CPPH_Keyword;
		}
		else if (pcToken->IsString())
		{
			eKind = CPPH_String;
		}
		else if (pcToken->IsChar() || (psSource->uiKind == PTK_Character))
		{
			eKind = CPPH_Character;
		}
		else
		{
			eKind = CPPH_Number;
		}
		//A token can be split over lines by backslash-newlines.
		iOffset = (int)mpcFile->GetOffset(psSource->iLine, psSource->iColumn);
		AddSpan(iOffset, SourceLength(iOffset, (int)psSource->uiLength), eKind, uiFlags);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
ECPPHighlight CCPPHighlighter::GetIdentifierKind(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, size iToken)
{
	SCPPTokenSymbol*	psSymbol;
	CCPPToken*			pcPrevious;
	CCPPToken*			pcNext;
	int64				lliName;

	psSymbol = pcTree->GetTokenSymbol((int)iToken);
	if (!psSymbol || !psSymbol->pcSymbol)
	{
		pcPrevious = (iToken > 0) ? pcTokeniser->Get(iToken - 1) : NULL;
		if (pcPrevious && pcPrevious->Is(CT_template) && (iToken > 1))
		{
			pcPrevious = pcTokeniser->Get(iToken - 2);
		}
		if (pcPrevious && (pcPrevious->Is(CT_Member) || pcPrevious->Is(CT_MemberByPointer)))
		{
			return CPPH_Member;
		}

		lliName = ((CCTIdentifier*)pcTokeniser->Get(iToken))->miIdentifier;
		if ((lliName == pcTokeniser->mlliOverride) || (lliName == pcTokeniser->mlliFinal))
		{
			return CPPH_Keyword;
		}

		pcNext = pcTokeniser->Get(iToken + 1);
		if (pcPrevious && pcPrevious->Is(CT_goto))
		{
			return CPPH_Label;
		}
		if (pcNext && pcNext->Is(CT_Colon) && (!pcPrevious || pcPrevious->Is(CT_SemiColon) || pcPrevious->Is(CT_OpenCurlyBracket) || pcPrevious->Is(CT_CloseCurlyBracket)))
		{
			return CPPH_Label;
		}

		if (psSymbol && (psSymbol->uiRole == CPPTR_Unresolved))
		{
			return CPPH_Unresolved;
		}
		return CPPH_Identifier;
	}

	switch (psSymbol->pcSymbol->meKind)
	{
	case CPPSK_Namespace:			return CPPH_Namespace;
	case CPPSK_Class:
	case CPPSK_ClassTemplate:		return CPPH_Class;
	case CPPSK_Enum:				return CPPH_Enum;
	case CPPSK_Typedef:
	case CPPSK_AliasTemplate:		return CPPH_Typedef;
	case CPPSK_TypeParameter:		return CPPH_TypeParameter;
	case CPPSK_BuiltinType:			return CPPH_BuiltinType;
	case CPPSK_Function:
	case CPPSK_FunctionTemplate:	return CPPH_Function;
	case CPPSK_Variable:			return CPPH_Variable;
	case CPPSK_Field:				return CPPH_Field;
	case CPPSK_Parameter:
	case CPPSK_ValueParameter:		return CPPH_Parameter;
	case CPPSK_Enumerator:			return CPPH_Enumerator;
	case CPPSK_Label:				return CPPH_Label;
	}
	return CPPH_Identifier;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::AddSpan(int iOffset, int iLength, ECPPHighlight eKind, uint8 uiFlags)
{
	int		iLine;
	int		iColumn;
	int		iEndLine;
	int		iEndColumn;
	int		i;

	//A span over many lines, like a /* */ comment, is a span for each line.
	if (!mpcFile->GetLineAndColumn(iOffset, &iLine, &iColumn) || !mpcFile->GetLineAndColumn(iOffset + iLength, &iEndLine, &iEndColumn))
	{
		return;
	}

	if (iLine == iEndLine)
	{
		Add(iLine, iColumn, iLength, eKind, uiFlags);
		return;
	}

	Add(iLine, iColumn, LineLength(iLine) - iColumn, eKind, uiFlags);
	for (i = iLine + 1; i < iEndLine; i++)
	{
		Add(i, 0, LineLength(i), eKind, uiFlags);
	}
	if (iEndColumn > 0)
	{
		Add(iEndLine, 0, iEndColumn, eKind, uiFlags);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::Add(int iLine, int iColumn, int iLength, ECPPHighlight eKind, uint8 uiFlags)
{
	SCPPHighlight*	psHighlight;

	if (iLength <= 0)
	{
		return;
	}
	psHighlight = macHighlights.Add();
	psHighlight->iLine = iLine;
	psHighlight->iColumn = iColumn;
	psHighlight->iLength = iLength;
	psHighlight->uiKind = (uint8)eKind;
	psHighlight->uiFlags = uiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareHighlights(const void* pv1, const void* pv2)
{
	SCPPHighlight*	ps1;
	SCPPHighlight*	ps2;

	ps1 = (SCPPHighlight*)pv1;
	ps2 = (SCPPHighlight*)pv2;
	if (ps1->iLine != ps2->iLine)
	{
		return (ps1->iLine < ps2->iLine) ? -1 : 1;
	}
	if (ps1->iColumn != ps2->iColumn)
	{
		return (ps1->iColumn < ps2->iColumn) ? -1 : 1;
	}

	//Whole line spans, inactive and directive lines, come before the tokens on them.
	if (ps1->iLength != ps2->iLength)
	{
		return (ps1->iLength > ps2->iLength) ? -1 : 1;
	}
	return (ps1->uiKind < ps2->uiKind) ? -1 : ((ps1->uiKind > ps2->uiKind) ? 1 : 0);
}


//Gives the highlighter each node under a node, for its syntax ranges.
class CCPPHighlighterRangeVisitor : public CCPPSyntaxChildVisitor
{
public:
	CCPPHighlighter*		mpcHighlighter;
	CCPPTokeniser*			mpcTokeniser;
	CArrayCPPSyntaxRanges*	mpasDest;

	void	Visit(CCPPSyntax* pcChild) override
	{
		mpcHighlighter->AddSyntaxRange(pcChild, mpcTokeniser, mpasDest);
		pcChild->VisitFields(this);
	}
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareSyntaxRanges(const void* pv1, const void* pv2)
{
	SCPPSyntaxRange*	ps1;
	SCPPSyntaxRange*	ps2;

	ps1 = (SCPPSyntaxRange*)pv1;
	ps2 = (SCPPSyntaxRange*)pv2;
	if (ps1->iLine != ps2->iLine)
	{
		return (ps1->iLine < ps2->iLine) ? -1 : 1;
	}
	if (ps1->iColumn != ps2->iColumn)
	{
		return (ps1->iColumn < ps2->iColumn) ? -1 : 1;
	}
	if (ps1->iEndLine != ps2->iEndLine)
	{
		return (ps1->iEndLine < ps2->iEndLine) ? -1 : 1;
	}
	if (ps1->iEndColumn != ps2->iEndColumn)
	{
		return (ps1->iEndColumn < ps2->iEndColumn) ? -1 : 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// What each node of the tree was parsed from in pcFile, for an editor's
// Extend Selection to grow through: a node whose first and last tokens
// are written in pcFile (not made by a macro, not from another file) has
// a range from the first's start to the last's end.  They are sorted by
// start then end, each once.
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::FindSyntaxRanges(CSourceFile* pcFile, CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, CArrayCPPSyntaxRanges* pasDest)
{
	CCPPHighlighterRangeVisitor	cVisitor;
	size						i;
	size						iKept;

	mpcFile = pcFile;
	if (pcTree->GetRoot() == NULL)
	{
		return;
	}
	cVisitor.mpcHighlighter = this;
	cVisitor.mpcTokeniser = pcTokeniser;
	cVisitor.mpasDest = pasDest;
	cVisitor.Visit(pcTree->GetRoot());

	if (pasDest->NumElements() <= 1)
	{
		return;
	}
	pasDest->QuickSort(&CompareSyntaxRanges);
	iKept = 0;
	for (i = 1; i < pasDest->NumElements(); i++)
	{
		if (CompareSyntaxRanges(pasDest->Get(i), pasDest->Get(iKept)) != 0)
		{
			iKept++;
			*pasDest->Get(iKept) = *pasDest->Get(i);
		}
	}
	pasDest->SetUsedElements(iKept + 1);
}


//////////////////////////////////////////////////////////////////////////
//
// pcNode's range in the file, if its first and last tokens are written in
// it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::AddSyntaxRange(CCPPSyntax* pcNode, CCPPTokeniser* pcTokeniser, CArrayCPPSyntaxRanges* pasDest)
{
	SPreprocessedToken*	psFirst;
	SPreprocessedToken*	psLast;
	SCPPSyntaxRange		sRange;
	int					iOffset;

	if ((pcNode->miFirstToken < 0) || (pcNode->miLastToken < pcNode->miFirstToken) || ((size)pcNode->miLastToken >= pcTokeniser->NumTokens()))
	{
		return;
	}
	psFirst = pcTokeniser->GetSource(pcTokeniser->Get(pcNode->miFirstToken));
	psLast = pcTokeniser->GetSource(pcTokeniser->Get(pcNode->miLastToken));
	if ((psFirst->pcFile != mpcFile) || (psFirst->iExpansion != -1) || (psLast->pcFile != mpcFile) || (psLast->iExpansion != -1))
	{
		return;
	}

	iOffset = (int)mpcFile->GetOffset(psLast->iLine, psLast->iColumn);
	iOffset += SourceLength(iOffset, (int)psLast->uiLength);
	if (!mpcFile->GetLineAndColumn(iOffset, &sRange.iEndLine, &sRange.iEndColumn))
	{
		return;
	}
	sRange.iLine = psFirst->iLine;
	sRange.iColumn = psFirst->iColumn;
	*pasDest->Add() = sRange;
}


//////////////////////////////////////////////////////////////////////////
//
// The ranges as "line:column-line:column" from 1, one to a line.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPHighlighter::PrintSyntaxRanges(CArrayCPPSyntaxRanges* pasRanges, CChars* psz)
{
	SCPPSyntaxRange*	psRange;
	size				i;

	for (i = 0; i < pasRanges->NumElements(); i++)
	{
		psRange = pasRanges->Get(i);
		psz->Append(psRange->iLine + 1);
		psz->Append(':');
		psz->Append(psRange->iColumn + 1);
		psz->Append('-');
		psz->Append(psRange->iEndLine + 1);
		psz->Append(':');
		psz->Append(psRange->iEndColumn + 1);
		psz->Append('\n');
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPHighlighter::Sort(void)
{
	size			i;
	size			iKept;
	SCPPHighlight*	psHighlight;
	SCPPHighlight*	psKept;

	if (macHighlights.NumElements() <= 1)
	{
		return;
	}
	macHighlights.QuickSort(&CompareHighlights);

	//A macro argument used more than once in the body gives the same token more than once.  The more specific kind sorts first and is kept.
	//A line of a skipped group that is all comment is inactive: the group is skipped whatever is in it.
	iKept = 0;
	for (i = 1; i < macHighlights.NumElements(); i++)
	{
		psHighlight = macHighlights.Get(i);
		psKept = macHighlights.Get(iKept);
		if ((psHighlight->iLine != psKept->iLine) || (psHighlight->iColumn != psKept->iColumn) || (psHighlight->iLength != psKept->iLength))
		{
			iKept++;
			if (iKept != i)
			{
				*macHighlights.Get(iKept) = *psHighlight;
			}
		}
		else if (psHighlight->uiKind == CPPH_Inactive)
		{
			*psKept = *psHighlight;
		}
	}
	macHighlights.SetUsedElements(iKept + 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPHighlighter::SourceLength(int iOffset, int iLength)
{
	char*	sz;
	int		iEnd;
	int		iContentsLength;

	//The length of a token in the file, where its length doesn't count the backslash-newlines it was spliced over.
	sz = mpcFile->GetContents();
	iContentsLength = (int)mpcFile->GetContentsLength();
	iEnd = iOffset;
	while (iLength > 0)
	{
		while ((iEnd < iContentsLength) && (sz[iEnd] == '\\'))
		{
			if ((iEnd + 1 < iContentsLength) && (sz[iEnd + 1] == '\n'))
			{
				iEnd += 2;
			}
			else if ((iEnd + 2 < iContentsLength) && (sz[iEnd + 1] == '\r') && (sz[iEnd + 2] == '\n'))
			{
				iEnd += 3;
			}
			else
			{
				break;
			}
		}
		if (iEnd >= iContentsLength)
		{
			break;
		}
		iEnd++;
		iLength--;
	}
	return iEnd - iOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CCPPHighlighter::LineLength(int iLine)
{
	size	iStart;
	size	iEnd;
	char*	sz;

	iStart = mpcFile->GetOffset(iLine, 0);
	if (iStart == ARRAY_ELEMENT_NOT_FOUND)
	{
		return 0;
	}
	sz = mpcFile->GetContents();
	for (iEnd = iStart; (iEnd < mpcFile->GetContentsLength()) && (sz[iEnd] != '\n') && (sz[iEnd] != '\r'); iEnd++)
	{
	}
	return (int)(iEnd - iStart);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPHighlight* CCPPHighlighter::Get(size iIndex)
{
	return macHighlights.SafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPHighlighter::NumHighlights(void)
{
	return macHighlights.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPHighlighter::Print(CChars* psz)
{
	size			i;
	SCPPHighlight*	psHighlight;

	for (i = 0; i < macHighlights.NumElements(); i++)
	{
		psHighlight = macHighlights.Get(i);
		psz->Append(psHighlight->iLine + 1);
		psz->Append(':');
		psz->Append(psHighlight->iColumn + 1);
		psz->Append(' ');
		psz->Append(psHighlight->iLength);
		psz->Append(' ');
		psz->Append(GetKindName((ECPPHighlight)psHighlight->uiKind));
		if (psHighlight->uiFlags & CPPHF_DECLARATION)
		{
			psz->Append(" declaration");
		}
		psz->AppendNewLine();
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPHighlighter::GetKindName(ECPPHighlight eKind)
{
	if ((eKind >= CPPH_Comment) && (eKind <= CPPH_Identifier))
	{
		return gaszHighlightKindNames[eKind];
	}
	return "unknown";
}

