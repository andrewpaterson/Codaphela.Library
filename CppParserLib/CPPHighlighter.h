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
#ifndef __C_P_P_HIGHLIGHTER_H__
#define __C_P_P_HIGHLIGHTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"


enum ECPPHighlight
{
	CPPH_Comment,
	CPPH_Inactive,			//Lines skipped by conditional compilation.
	CPPH_Directive,
	CPPH_Macro,				//The name of a macro where it is used.
	CPPH_Keyword,
	CPPH_Operator,
	CPPH_Number,
	CPPH_Character,
	CPPH_String,
	CPPH_Namespace,
	CPPH_Class,
	CPPH_Enum,
	CPPH_Typedef,
	CPPH_TypeParameter,
	CPPH_BuiltinType,
	CPPH_Function,
	CPPH_Variable,
	CPPH_Field,
	CPPH_Parameter,
	CPPH_Enumerator,
	CPPH_Label,
	CPPH_Member,			//A member of a type the parser doesn't know, after . or ->.
	CPPH_Unresolved,		//A name that was not found.
	CPPH_Identifier,		//Any other identifier.
};


//CCPPHighlight::uiFlags
#define CPPHF_DECLARATION	0x01


struct SCPPHighlight
{
	int32	iLine;			//0 based.
	int32	iColumn;		//0 based.
	int32	iLength;
	uint8	uiKind;
	uint8	uiFlags;
};


typedef CArrayTemplate<SCPPHighlight>	CArrayHighlights;


//Highlights one source file of a parsed translation unit: what each token is, and the comments, directives, macros and
//skipped lines.  Spans are sorted by line then column and don't cross lines.
class CCPPHighlighter
{
protected:
	CArrayHighlights	macHighlights;
	CSourceFile*		mpcFile;

public:
	void				Init(void);
	void				Kill(void);
	void				Highlight(CSourceFile* pcFile, CPreprocessedTokens* pcPreprocessed, CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree);

	size				NumHighlights(void);
	SCPPHighlight*		Get(size iIndex);
	char*				Print(CChars* psz);
	static char*		GetKindName(ECPPHighlight eKind);

protected:
	void				AddComments(void);
	void				AddInactiveLines(CPreprocessedTokens* pcPreprocessed);
	void				AddDirectives(CPreprocessedTokens* pcPreprocessed);
	void				AddMacros(CPreprocessedTokens* pcPreprocessed);
	void				AddTokens(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree);
	ECPPHighlight		GetIdentifierKind(CCPPTokeniser* pcTokeniser, CCPPSyntaxTree* pcTree, size iToken);
	void				AddSpan(int iOffset, int iLength, ECPPHighlight eKind, uint8 uiFlags);
	void				Add(int iLine, int iColumn, int iLength, ECPPHighlight eKind, uint8 uiFlags);
	void				Sort(void);
	int					SourceLength(int iOffset, int iLength);
	int					LineLength(int iLine);
};


#endif // __C_P_P_HIGHLIGHTER_H__

