/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
void CPreprocessorLineTokeniser::Preprocess(CPPTokenList* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokenMemory* pcTokens)
{
	CPreprocessorLineTokeniser	cLinePreprocessor;

	cLinePreprocessor.Init(pcLinesTokens, pcParser, pcTokens);
	cLinePreprocessor.Preprocess();
	cLinePreprocessor.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::Init(CPPTokenList* pcLinesTokens, CPreprocessorParser* pcParser, CPPTokenMemory* pcTokens)
{
	mpcParser = pcParser;
	mpcTokenHolder = pcLinesTokens;
	mpcTokens = pcTokens;
	mbOnlyWhitespace = true;
	miStartLine = pcParser->miLine;
	miStartColumn = pcParser->miColumn;
	NullAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::Preprocess(void)
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
		else if ((*mpcParser->GetPos()) == '\\')
		{
			mpcParser->StepRight();
			if (mpcParser->IsNewLine())
			{
				//Was a line continuer.
				mpcParser->SkipNewLine();
				mbContainsLineContinuers = true;
			}
			else if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				//A backslash outside a literal is a token of its own, as in the stringified str(: @\n).
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->GetPos();
				MarkStart();
				mpcParser->StepRight();
			}
			else
			{
				//An escape sequence.  The escaped character can't end the literal.
				mpcParser->StepRight();
			}
		}
		else if ((*mpcParser->GetPos()) == '"')
		{
			if (!mszSingleQuoteStart)
			{
				if (!mszDoubleQuoteStart)
				{
					AddRelevantToken();
					NullAll();
					mszDoubleQuoteStart = mpcParser->GetPos();
					MarkStart();
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
		else if (((*mpcParser->GetPos()) == '\'') && IsDigitSeparator())
		{
			//C++14 digit separator as in 0x10'0000.
			mpcParser->StepRight();
		}
		else if ((*mpcParser->GetPos()) == '\'')
		{
			if (!mszDoubleQuoteStart)
			{
				if (!mszSingleQuoteStart)
				{
					AddRelevantToken();
					NullAll();
					mszSingleQuoteStart = mpcParser->GetPos();
					MarkStart();
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
		else if (mpcParser->IsWhitespace())
		{
			//If we're in a quote then white space is parsed as normal text.
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				if (!mszWhitespaceStart)
				{
					AddRelevantToken();
					NullAll();
					mszWhitespaceStart = mpcParser->GetPos();
					MarkStart();
				}
			}
			mpcParser->StepRight();
		}
		else if ((*mpcParser->GetPos()) == '/')
		{
			if (PossibleComment())
			{
				break;
			}
		}
		else if ((*mpcParser->GetPos()) == '#')
		{
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				if (!mszHashStart)
				{
					AddRelevantToken();
					NullAll();
					mszHashStart = mpcParser->GetPos();
					MarkStart();
				}
			}
			mpcParser->StepRight();
		}
		else if (mpcParser->IsFirstIdentifier())
		{
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				//Letters after a digit are part of the pp-number, as in 0x7FFF or 201402L.
				if ((!mszIdentifierStart) && (!mszNumberStart))
				{
					AddRelevantToken();
					NullAll();
					mszIdentifierStart = mpcParser->GetPos();
					MarkStart();
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
					mszNumberStart = mpcParser->GetPos();
					MarkStart();
				}
			}
			mpcParser->StepRight();
		}
		else if (IsPPNumberContinuation())
		{
			mpcParser->StepRight();
		}
		else if (IsPPNumberStart())
		{
			AddRelevantToken();
			NullAll();
			mszNumberStart = mpcParser->GetPos();
			MarkStart();
			mpcParser->StepRight();
		}
		else
		{
			if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
			{
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->GetPos();
				MarkStart();
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
bool CPreprocessorLineTokeniser::IsDigitSeparator(void)
{
	char	c;

	if ((!mszNumberStart) || mszSingleQuoteStart || mszDoubleQuoteStart)
	{
		return false;
	}

	if (mpcParser->GetPos() >= mpcParser->GetEnd())
	{
		return false;
	}

	c = mpcParser->GetPos()[1];
	return ((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorLineTokeniser::IsPPNumberContinuation(void)
{
	char	c;
	char	cPrev;

	if ((!mszNumberStart) || mszSingleQuoteStart || mszDoubleQuoteStart)
	{
		return false;
	}

	//A pp-number continues with '.' and with a sign after an exponent, as in 1.5e+10 or 0x1p-3.
	c = *mpcParser->GetPos();
	if (c == '.')
	{
		return true;
	}

	if ((c == '+') || (c == '-'))
	{
		cPrev = mpcParser->GetPos()[-1];
		return (cPrev == 'e') || (cPrev == 'E') || (cPrev == 'p') || (cPrev == 'P');
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorLineTokeniser::IsPPNumberStart(void)
{
	char	c;

	if (mszSingleQuoteStart || mszDoubleQuoteStart || mszIdentifierStart)
	{
		return false;
	}

	//A pp-number can start with a '.', as in .5f.
	if ((*mpcParser->GetPos()) != '.')
	{
		return false;
	}

	if (mpcParser->GetPos() >= mpcParser->GetEnd())
	{
		return false;
	}

	c = mpcParser->GetPos()[1];
	return (c >= '0') && (c <= '9');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessorLineTokeniser::PossibleComment(void)
{
	//If we're in a quote then comments are ignored.
	if (!(mszSingleQuoteStart || mszDoubleQuoteStart))
	{
		//If we haven't already begun white space then the comment is the beginning.
		if (!mszWhitespaceStart)
		{
			mpcParser->StepRight();
			if ((*mpcParser->GetPos()) == '/')
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszWhitespaceStart = mpcParser->GetPos();
				MarkStart();
				mpcParser->StepRight(2);

				mpcParser->SkipCPPStyleComment();
				mpcParser->SkipNewLine();
				return true;
			}
			else if ((*mpcParser->GetPos()) == '*')
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszWhitespaceStart = mpcParser->GetPos();
				MarkStart();
				mpcParser->StepRight(2);
				mpcParser->SkipCStyleComment();
				return false;
			}
			else
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->GetPos();
				MarkStart();
				mpcParser->StepRight();
				return false;
			}
		}
		else
		{
			mpcParser->StepRight();
			if ((*mpcParser->GetPos()) == '/')
			{
				mpcParser->StepRight();

				mpcParser->SkipCPPStyleComment();
				mpcParser->SkipNewLine();
				return true;
			}
			else if ((*mpcParser->GetPos()) == '*')
			{
				mpcParser->StepRight();
				mpcParser->SkipCStyleComment();
				return false;
			}
			else
			{
				mpcParser->StepLeft();
				AddRelevantToken();
				NullAll();
				mszDecorationStart = mpcParser->GetPos();
				MarkStart();
				mpcParser->StepRight();
				return false;
			}
		}
	}
	else
	{
		mpcParser->StepRight();
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::AddRelevantToken(void)
{
	CPPWhitespace*	pcWhitespace;
	CPPHashes*		pcHashes;

	if ((mszDecorationStart) && (mszDecorationStart <= mpcParser->GetEnd()))
	{
		//The length of a decorator must be 1!
		AddText(PPT_Decorator, mszDecorationStart, mpcParser->GetPos());
	}
	else if ((mszIdentifierStart) && (mszIdentifierStart <= mpcParser->GetEnd()))
	{
		AddText(PPT_Identifier, mszIdentifierStart, mpcParser->GetPos());
	}
	else if ((mszNumberStart) && (mszNumberStart <= mpcParser->GetEnd()))
	{
		AddText(PPT_Number, mszNumberStart, mpcParser->GetPos());
	}
	else if ((mszWhitespaceStart) && (mszWhitespaceStart <= mpcParser->GetEnd()))
	{
		if (!mbOnlyWhitespace)
		{
			pcWhitespace = mpcTokens->AddToken<CPPWhitespace>();
			pcWhitespace->Init(miStartLine, miStartColumn, mpcParser->mszFileName);

			mpcTokenHolder->Add(pcWhitespace);
		}
	}
	else if ((mszHashStart) && (mszHashStart <= mpcParser->GetEnd()))
	{
		pcHashes = mpcTokens->AddToken<CPPHashes>();
		pcHashes->Init((int)(mpcParser->GetPos() - mszHashStart), miStartLine, miStartColumn, mpcParser->mszFileName);

		mpcTokenHolder->Add(pcHashes);
		mbOnlyWhitespace = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPText* CPreprocessorLineTokeniser::AddText(EPreprocessorText eType, char* szStart, char* szEndExclusive)
{
	CPPText*			pcText;
	CChars				sz;
	CPPTextWithSource*	pcTextWithSource;

	mbOnlyWhitespace = false;
	if (!mbContainsLineContinuers)
	{
		pcText = mpcTokens->AddToken<CPPText>();
		pcText->Init(eType, miStartLine, miStartColumn, mpcParser->mszFileName, szStart, szEndExclusive);

		mpcTokenHolder->Add(pcText);
		return pcText;
	}
	else
	{
		sz.Init();
		ReplaceLineContinuers(&sz, szStart, szEndExclusive);

		pcTextWithSource = mpcTokens->AddToken<CPPTextWithSource>();
		pcTextWithSource->Init(eType, miStartLine, miStartColumn, mpcParser->mszFileName, sz.Text(), (int)sz.Length());

		mpcTokenHolder->Add(pcTextWithSource);
		sz.Kill();
		return pcTextWithSource;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::AddDoubleQuotedToken(void)
{
	//String literals keep their spelling.  Escape sequences are the compiler's business.
	if (mszDoubleQuoteStart)
	{
		AddText(PPT_DoubleQuoted, mszDoubleQuoteStart, mpcParser->GetPos());
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::AddSingleQuotedToken(void)
{
	if (mszSingleQuoteStart)
	{
		AddText(PPT_SingleQuoted, mszSingleQuoteStart, mpcParser->GetPos());
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::MarkStart(void)
{
	//Tokens are positioned by where they start.
	miStartLine = mpcParser->miLine;
	miStartColumn = mpcParser->miColumn;
}


//																		//
//																		//
void CPreprocessorLineTokeniser::NullAll(void)
{
	mszWhitespaceStart = NULL;
	mszIdentifierStart = NULL;
	mszDecorationStart = NULL;
	mszDoubleQuoteStart = NULL;
	mszSingleQuoteStart = NULL;
	mszHashStart = NULL;
	mszNumberStart = NULL;
	mbContainsLineContinuers = false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::ReplaceLineContinuers(CChars* psz, char* szStart, char* szEnd)
{
	char*	pc;

	for (pc = szStart; pc < szEnd; pc++)
	{
		if ((*pc == '\\') && (pc + 1 < szEnd) && ((pc[1] == '\n') || (pc[1] == '\r')))
		{
			//A line splice: the backslash and the new line are removed.  [lex.phases] 2
			pc++;
			if ((pc + 1 < szEnd) && ((pc[1] == '\n') || (pc[1] == '\r')) && (pc[1] != pc[0]))
			{
				pc++;
			}
		}
		else
		{
			psz->Append(*pc);
		}
	}
}

