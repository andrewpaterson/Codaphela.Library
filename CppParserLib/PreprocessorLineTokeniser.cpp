/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include "BaseLib/EscapeCodes.h"
#include "PreprocessorLineTokeniser.h"
#include "GeneralToken.h"
#include "PPText.h"
#include "PPTextWithSource.h"
#include "PPWhiteSpace.h"
#include "PPHashes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::Preprocess(CPPTokenHolder* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokens* pcTokens, BOOL bAllowEscapes)
{
	CPreprocessorLineTokensier	cLinePreprocessor;

	cLinePreprocessor.Init(&pcLinesTokens->mcArray, pcParser, pcTokens, bAllowEscapes);
	cLinePreprocessor.Preprocess();
	pcTokens->Dump();
	cLinePreprocessor.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::Init(CArrayPPTokenPtrs* pcTokenPtrs, CPreprocessorParser* pcParser, CPPTokens* pcTokens, BOOL bAllowEscapes)
{
	mpcParser = pcParser;
	mpcTokenPtrs = pcTokenPtrs;
	mpcTokens = pcTokens;
	mbOnlyWhiteSpace = TRUE;
	mbAllowEscapes = bAllowEscapes;
	NullAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::Kill(void)
{

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::Preprocess(void)
{
	//Remember that at this point the hash and directive have already been strippeed if they existed.
	//Any existing hashes are... something else.
	while (!mpcParser->mbEndOfFile)
	{
		if (mpcParser->IsNewLine())
		{
			if (mszSingleQuoteStart)
			{
				//Newline in constant... not allowed.
				AddSingleQuotedToken();
				NullAll();
			}
			else if (mszDoubleQuoteStart)
			{
				//Newline in constant... not allowed.
				AddDoubleQuotedToken();
				NullAll();
			}
			else
			{
				AddRelevantToken();
				NullAll();
			}
			mpcParser->SkipNewLine();
			break;
		}
		else if ((*mpcParser->mszPos) == '\\')
		{
			mpcParser->StepRight();
			if (mpcParser->IsNewLine())
			{
				//Was a line continuer.
				mpcParser->SkipNewLine();
				mbContainsLineContinuers = TRUE;
			}
			else
			{
				//Was an escape code or something that looked like one.
				if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
				{
					AddRelevantToken();  //It's illegal to have it outside of a string but add the previous token anyway.
					NullAll();
				}
				mpcParser->StepRight();
			}
		}
		else if ((*mpcParser->mszPos) == '"')
		{
			if (!mszSingleQuoteStart)
			{
				if (!mszDoubleQuoteStart)
				{
					AddRelevantToken();
					NullAll();
					mszDoubleQuoteStart = mpcParser->mszPos;
					mpcParser->StepRight();
				}
				else
				{
					mpcParser->StepRight();
					AddDoubleQuotedToken();
					NullAll();
				}
			}
			else
			{
				mpcParser->StepRight();
			}
		}
		else if ((*mpcParser->mszPos) == '\'')
		{
			if (!mszDoubleQuoteStart)
			{
				if (!mszSingleQuoteStart)
				{
					AddRelevantToken();
					NullAll();
					mszSingleQuoteStart = mpcParser->mszPos;
					mpcParser->StepRight();
				}
				else
				{
					mpcParser->StepRight();
					AddSingleQuotedToken();
					NullAll();
				}
			}
			else
			{
				mpcParser->StepRight();
			}
		}
		else if (mpcParser->IsWhiteSpace())
		{
			//If we're in a quote then white space is parsed as normal text.
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				if (!mszWhiteSpaceStart)
				{
					AddRelevantToken();
					NullAll();
					mszWhiteSpaceStart = mpcParser->mszPos;
				}
			}
			mpcParser->StepRight();
		}
		else if ((*mpcParser->mszPos) == '/')
		{
			if (PossibleComment())
			{
				break;
			}
		}
		else if ((*mpcParser->mszPos) == '#')
		{
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				if (!mszHashStart)
				{
					AddRelevantToken();
					NullAll();
					mszHashStart = mpcParser->mszPos;
				}
			}
			mpcParser->StepRight();
		}
		else if (mpcParser->IsFirstIdentifier())
		{
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				if (!mszIdentifierStart)
				{
					AddRelevantToken();
					NullAll();
					mszIdentifierStart = mpcParser->mszPos;
				}
			}
			mpcParser->StepRight();
		}
		else if (mpcParser->IsDigit())
		{
			if ((!mszIdentifierStart) && (!(mszSingleQuoteStart || mszDoubleQuoteStart)))
			{
				if (!mszNumberStart)
				{
					AddRelevantToken();
					NullAll();
					mszNumberStart = mpcParser->mszPos;
				}
			}
			mpcParser->StepRight();
		}
		else
		{
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->mszPos;
			}
			mpcParser->StepRight();
		}
	}

	//This handles the end of file without an newline case.
	AddRelevantToken();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessorLineTokensier::PossibleComment(void)
{
	//If we're in a quote then comments are ignored.
	if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
	{
		//If we haven't already begun white space then the comment is the beginning.
		if (!mszWhiteSpaceStart)
		{
			mpcParser->StepRight();
			if ((*mpcParser->mszPos) == '/')
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszWhiteSpaceStart = mpcParser->mszPos;
				mpcParser->StepRight(2);

				if ((*mpcParser->mszPos) == '@')
				{
					AddAnnotationToken();
					mpcParser->SkipNewLine();
					return TRUE;
				}
				else
				{
					mpcParser->SkipCPPStyleComment();
					mpcParser->SkipNewLine();
					return TRUE;
				}
			}
			else if ((*mpcParser->mszPos) == '*')
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszWhiteSpaceStart = mpcParser->mszPos;
				mpcParser->StepRight(2);
				mpcParser->SkipCStyleComment();
				return FALSE;
			}
			else
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->mszPos;
				mpcParser->StepRight();
				return FALSE;
			}
		}
		else
		{
			mpcParser->StepRight();
			if ((*mpcParser->mszPos) == '/')
			{
				mpcParser->StepRight();

				if ((*mpcParser->mszPos) == '@')
				{
					AddAnnotationToken();
					mpcParser->SkipNewLine();
					return TRUE;
				}
				else
				{
					mpcParser->SkipCPPStyleComment();
					mpcParser->SkipNewLine();
					return TRUE;
				}
			}
			else if ((*mpcParser->mszPos) == '*')
			{
				mpcParser->StepRight();
				mpcParser->SkipCStyleComment();
				return FALSE;
			}
			else
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->mszPos;
				mpcParser->StepRight();
				return FALSE;
			}
		}
	}
	else
	{
		mpcParser->StepRight();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::AddRelevantToken(void)
{
	CPPWhiteSpace*	pcWhiteSpace;
	CPPHashes*		pcHashes;
	CPPText*		pcText;

	if ((mszDecorationStart) && (mszDecorationStart <= mpcParser->mszEnd))
	{
		//The length of a decorator must be 1!
		AddText(PPT_Decorator, mszDecorationStart, mpcParser->mszPos);
	}
	else if ((mszIdentifierStart) && (mszIdentifierStart <= mpcParser->mszEnd))
	{
		pcText = AddText(PPT_Identifier, mszIdentifierStart, mpcParser->mszPos);
	}
	else if ((mszNumberStart) && (mszNumberStart <= mpcParser->mszEnd))
	{
		AddText(PPT_Number, mszNumberStart, mpcParser->mszPos);
	}
	else if ((mszWhiteSpaceStart) && (mszWhiteSpaceStart <= mpcParser->mszEnd))
	{
		if (!mbOnlyWhiteSpace)
		{
			pcWhiteSpace = mpcTokens->AddWhiteSpace();
			AddToken(pcWhiteSpace, mpcTokenPtrs);
			pcWhiteSpace->Init(mpcParser->miLine, mpcParser->miColumn);
		}
	}
	else if ((mszHashStart) && (mszHashStart <= mpcParser->mszEnd))
	{
		pcHashes = mpcTokens->AddHashes();
		AddToken(pcHashes, mpcTokenPtrs);
		pcHashes->Init((int)(mpcParser->mszPos - mszHashStart), mpcParser->miLine, mpcParser->miColumn);
		mbOnlyWhiteSpace = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPText* CPreprocessorLineTokensier::AddText(EPreprocessorText eType, char* szStart, char* szEndExclusive)
{
	CPPText*			pcText;
	CChars				sz;
	CPPTextWithSource*	pcTextWithSource;

	mbOnlyWhiteSpace = FALSE;
	if (!mbContainsLineContinuers)
	{
		pcText = mpcTokens->AddText();
		AddToken(pcText, mpcTokenPtrs);
		pcText->Init(eType, mpcParser->miLine, mpcParser->miColumn, szStart, szEndExclusive);
		return pcText;
	}
	else
	{
		sz.Init();
		ReplaceLineContinuers(&sz, szStart, szEndExclusive);
		pcTextWithSource = mpcTokens->AddTextWithSource();
		AddToken(pcTextWithSource, mpcTokenPtrs);
		pcTextWithSource->Init(eType, mpcParser->miLine, mpcParser->miColumn, sz.Text(), sz.Length());
		sz.Kill();
		return pcTextWithSource;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::AddDoubleQuotedToken(void)
{
	CPPText*			pcText;
	CPPTextWithSource*	pcTextWithSource;
	CChars				sz;

	if (mszDoubleQuoteStart)
	{
		if (!mbAllowEscapes)
		{
			pcText = mpcTokens->AddText();
			AddToken(pcText, mpcTokenPtrs);
			pcText->Init(PPT_DoubleQuoted, mpcParser->miLine, mpcParser->miColumn, mszDoubleQuoteStart, mpcParser->mszPos);
		}
		else
		{
			sz.Init();
			ReplaceEscapeCodes(&sz, mszDoubleQuoteStart+1, mpcParser->mszPos-1, '"');
			pcTextWithSource = mpcTokens->AddTextWithSource();
			AddToken(pcTextWithSource, mpcTokenPtrs);
			pcTextWithSource->Init(PPT_DoubleQuoted, mpcParser->miLine, mpcParser->miColumn, sz.Text(), sz.Length());
			sz.Kill();
		}
		mbOnlyWhiteSpace = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::AddSingleQuotedToken(void)
{
	CPPText*			pcText;
	CPPTextWithSource*	pcTextWithSource;
	CChars				sz;

	if (mszSingleQuoteStart)
	{
		if (!mbAllowEscapes)
		{
			pcText = mpcTokens->AddText();
			AddToken(pcText, mpcTokenPtrs);
			pcText->Init(PPT_SingleQuoted, mpcParser->miLine, mpcParser->miColumn, mszSingleQuoteStart, mpcParser->mszPos);
		}
		else
		{
			sz.Init();
			ReplaceEscapeCodes(&sz, mszSingleQuoteStart+1, mpcParser->mszPos-1, '\'');
			pcTextWithSource = mpcTokens->AddTextWithSource();
			AddToken(pcTextWithSource, mpcTokenPtrs);
			pcTextWithSource->Init(PPT_SingleQuoted, mpcParser->miLine, mpcParser->miColumn, sz.Text(), sz.Length());
			sz.Kill();
		}
		mbOnlyWhiteSpace = FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::AddAnnotationToken(void)
{
	//Oi!  Annotations please.
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessorLineTokensier::AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pcTokens)
{
	CPPToken**	ppcToken;

	ppcToken = pcTokens->Add();
	*ppcToken = pcToken;
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::NullAll(void)
{
	mszWhiteSpaceStart = NULL;
	mszIdentifierStart = NULL;
	mszDecorationStart = NULL;
	mszDoubleQuoteStart = NULL;
	mszSingleQuoteStart = NULL;
	mszHashStart = NULL;
	mszNumberStart = NULL;
	mbContainsLineContinuers = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::ReplaceEscapeCodes(CChars* psz, char* szStart, char* szEnd, char cQuotes)
{
	char*	pc;
	char	cReturn;
	BOOL	bEscape;

	psz->Append(cQuotes);
	bEscape = FALSE;
	for (pc = szStart; pc < szEnd; pc++)
	{
		if ((*pc == '\\') && (!bEscape))
		{
			bEscape = TRUE;
		}
		else
		{
			if (!bEscape)
			{
				psz->Append(*pc);
			}
			else
			{
				cReturn = GetEscapeCode(*pc);
				if (cReturn != '@')
				{
					psz->Append(cReturn);
				}
				else
				{
					psz->Append("Bad escape code [\\");
					psz->Append(*pc);
					psz->Append("]");
				}
			}
		}
	}
	psz->Append(cQuotes);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokensier::ReplaceLineContinuers(CChars* psz, char* szStart, char* szEnd)
{
	char*	pc;
	BOOL	bEscape;
	char	cLast;
	BOOL	bSkipped;

	bEscape = FALSE;
	bSkipped = FALSE;
	cLast = '\0';
	for (pc = szStart; pc < szEnd; pc++)
	{
		if (*pc == '\\')
		{
			bEscape = TRUE;
			bSkipped = FALSE;
		}
		else
		{
			if (bEscape)
			{
				if ((*pc == '\n') || (*pc == '\r'))
				{
					if (*pc == cLast)
					{
						bEscape = FALSE;
						bSkipped = FALSE;
						psz->Append(*pc);
					}
					else
					{
						bSkipped = TRUE;
					}
				}
				else
				{
					bEscape = FALSE;
					if (!bSkipped)
					{
						psz->Append('\\');
					}
					psz->Append(*pc);
					bSkipped = FALSE;
				}
			}
			else
			{
				psz->Append(*pc);
			}
		}

		cLast = *pc;
	}
}

