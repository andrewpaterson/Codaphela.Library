/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "PreprocessorLineTokeniser.h"
#include "GeneralToken.h"
#include "PPText.h"
#include "PPTextWithSource.h"
#include "PPWhiteSpace.h"
#include "PPHashes.h"
#include "CPPNameCharacters.h"
#include "LiteralHelper.h"


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
	size	uiLength;

	//Remember that at this point the hash and directive have already been strippeed if they existed.
	//Any existing hashes are... something else.
	while (!mpcParser->mbEndOfFile)
	{
		if (mpcParser->IsNewLine())
		{
			//A string or character that hasn't ended is ended by the line: the C++ tokeniser reports it.
			AddRelevantToken();
			NullAll();
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
				mpcParser->StepLeft();
				uiLength = UniversalCharacterNameLength(mpcParser->GetPos(), mpcParser->GetEnd() + 1);
				if (uiLength != 0)
				{
					//A character of an identifier, spelled \u00E9: the token's text has the character itself.
					IdentifierCharacter();
					mbContainsUniversalNames = true;
					mpcParser->StepRight((int)uiLength - 1);
				}
				else
				{
					//A backslash outside a literal is a token of its own, as in the stringified str(: @\n).
					AddRelevantToken();
					NullAll();
					mszDecorationStart = mpcParser->GetPos();
					MarkStart();
					mpcParser->StepRight();
				}
			}
			else
			{
				//An escape sequence.  The escaped character can't end the literal.
				mpcParser->StepRight();
			}
		}
		else if ((*mpcParser->GetPos()) == '"')
		{
			Quote(&mszDoubleQuoteStart, mszSingleQuoteStart != NULL);
		}
		else if (((*mpcParser->GetPos()) == '\'') && IsDigitSeparator())
		{
			//C++14 digit separator as in 0x10'0000.
			mpcParser->StepRight();
		}
		else if ((*mpcParser->GetPos()) == '\'')
		{
			Quote(&mszSingleQuoteStart, mszDoubleQuoteStart != NULL);
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
				IdentifierCharacter();
			}
			else
			{
				mpcParser->StepRight();
			}
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
// A character an identifier can start with, outside a literal: it starts
// one, or is the next of the one being read.  Letters after a digit are
// part of the pp-number, as in 0x7FFF or 201402L.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::IdentifierCharacter(void)
{
	if ((!mszIdentifierStart) && (!mszNumberStart))
	{
		AddRelevantToken();
		NullAll();
		mszIdentifierStart = mpcParser->GetPos();
		MarkStart();
	}
	mpcParser->StepRight();
}


//////////////////////////////////////////////////////////////////////////
//
// A quote: the start of a string or character literal, its end, or a
// character of a literal of the other kind (bInOther).  *pszStart is where
// a literal of the quote's kind started, NULL when not in one.
//
// The literal starts at its encoding prefix, as the L of L"x" and the u8
// of u8'x', and ends after its user-defined suffix, as the _s of "x"_s:
// they are part of its token.  A raw string (an R ends its prefix) is
// read to its end at its opening quote.  [lex.ccon], [lex.string],
// [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::Quote(char** pszStart, bool bInOther)
{
	bool	bString;
	char*	szPrefix;
	char*	szRawEnd;
	size	uiRaw;

	if (bInOther)
	{
		mpcParser->StepRight();
		return;
	}

	bString = (*mpcParser->GetPos()) == '"';
	if (!*pszStart)
	{
		szPrefix = mszIdentifierStart;
		if (szPrefix && IsLiteralPrefix(szPrefix, mpcParser->GetPos() - szPrefix, bString))
		{
			//The identifier so far is the literal's start.
			mszIdentifierStart = NULL;
			*pszStart = szPrefix;
			if (mpcParser->GetPos()[-1] == 'R')
			{
				uiRaw = RawStringLength(mpcParser->GetPos(), mpcParser->GetEnd() + 1);
				if (uiRaw != 0)
				{
					//All of a raw string is read here: its lines are not lines of the source, and nothing in it is a
					//splice, a comment or a directive.  One that doesn't end is left to end with the line, as a string
					//does, so that it can't take the rest of the file with it.
					szRawEnd = mpcParser->GetPos() + uiRaw - 1;
					while (mpcParser->GetPos() < szRawEnd)
					{
						mpcParser->StepRight();
					}
					EndQuoted(pszStart, true);
					return;
				}
			}
		}
		else
		{
			AddRelevantToken();
			NullAll();
			*pszStart = mpcParser->GetPos();
			MarkStart();
		}
		mpcParser->StepRight();
	}
	else
	{
		EndQuoted(pszStart, bString);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The closing quote of the string or character literal that started at
// *pszStart: its token ends after its user-defined suffix, if it has one.
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::EndQuoted(char** pszStart, bool bString)
{
	mpcParser->StepRight();
	while (!mpcParser->mbEndOfFile && IsCPPNameCharacter(*mpcParser->GetPos()))
	{
		mpcParser->StepRight();
	}
	AddText(bString ? PPT_DoubleQuoted : PPT_SingleQuoted, *pszStart, mpcParser->GetPos());
	NullAll();
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
	return IsCPPNameCharacter(c);
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
	else if (mszDoubleQuoteStart)
	{
		AddText(PPT_DoubleQuoted, mszDoubleQuoteStart, mpcParser->GetPos());
	}
	else if (mszSingleQuoteStart)
	{
		AddText(PPT_SingleQuoted, mszSingleQuoteStart, mpcParser->GetPos());
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
	if (!mbContainsLineContinuers && !mbContainsUniversalNames)
	{
		pcText = mpcTokens->AddToken<CPPText>();
		pcText->Init(eType, miStartLine, miStartColumn, mpcParser->mszFileName, szStart, szEndExclusive);

		mpcTokenHolder->Add(pcText);
		return pcText;
	}
	else
	{
		sz.Init();
		Respell(&sz, szStart, szEndExclusive);

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
	mbContainsUniversalNames = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A token's text when it isn't what the source has from szStart to szEnd:
// line splices are taken out and, in an identifier or a number, universal
// character names are the characters they name, in UTF-8, so that a name
// is the same however it is spelled.  [lex.phases] 1 and 2
//
//////////////////////////////////////////////////////////////////////////
void CPreprocessorLineTokeniser::Respell(CChars* psz, char* szStart, char* szEnd)
{
	char*		pc;
	const char*	szName;
	size		uiLength;
	uint32		uiCharacter;
	char		szUTF8[UTF8_MAX_BYTES];

	for (pc = szStart; pc < szEnd; pc++)
	{
		uiLength = mbContainsUniversalNames ? UniversalCharacterNameLength(pc, szEnd) : 0;
		if ((*pc == '\\') && (pc + 1 < szEnd) && ((pc[1] == '\n') || (pc[1] == '\r')))
		{
			//A line splice: the backslash and the new line are removed.
			pc++;
			if ((pc + 1 < szEnd) && ((pc[1] == '\n') || (pc[1] == '\r')) && (pc[1] != pc[0]))
			{
				pc++;
			}
		}
		else if (uiLength != 0)
		{
			szName = pc + 1;
			DecodeEscape(&szName, pc + uiLength, &uiCharacter);
			psz->Append(szUTF8, EncodeUTF8(uiCharacter, szUTF8));
			pc += uiLength - 1;
		}
		else
		{
			psz->Append(*pc);
		}
	}
}

