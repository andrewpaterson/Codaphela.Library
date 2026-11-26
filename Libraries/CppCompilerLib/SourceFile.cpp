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
#include "BaseLib/TextFile.h"
#include "BaseLib/Numbers.h"
#include "BaseLib/FileUtil.h"
#include "Library.h"
#include "SourceFile.h"
#include "PPTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Init(char* szName)
{
	mszContents.Init();
	mszFullName.Init(szName);
	mbLoaded = false;
	mbContentsSet = false;
	mcTokenMemory.Init();
	macBlockSets.Init();
	mbBlockSetsKilled = false;
	macComments.Init();
	maiLineStarts.Init();
	mcMutex.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::KillBlockSets(void)
{
	macBlockSets.Kill();
	mbBlockSetsKilled = true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Kill(void)
{
	if (!mbBlockSetsKilled)
	{
		macBlockSets.Kill();
	}
	maiLineStarts.Kill();
	macComments.Kill();
	mcTokenMemory.Kill();
	mszFullName.Kill();
	mszContents.Kill();
	mbLoaded = false;
	mcMutex.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Units preprocessed at the same time include the same headers: the first
// to include one loads and tokenises it while it is locked, and the others
// wait (CCPPPreprocessor::TokeniseFile).
//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Lock(void)
{
	mcMutex.Lock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Unlock(void)
{
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//																		//
// Frees the file's tokens (and its contents, if they were read from the	//
// file): it is read and tokenised again when next preprocessed.  Nothing	//
// may still point into them (a translation unit's preprocessed tokens).	//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Unload(void)
{
	if (!mbBlockSetsKilled)
	{
		macBlockSets.Kill();
	}
	macBlockSets.Init();
	mbBlockSetsKilled = false;
	maiLineStarts.Kill();
	maiLineStarts.Init();
	macComments.Kill();
	macComments.Init();
	mcTokenMemory.Kill();
	mcTokenMemory.Init();
	if (!mbContentsSet)
	{
		mszContents.Kill();
		mszContents.Init();
		mbLoaded = false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Reads the file's contents (once; contents given by SetContents are kept).
// False, with no contents, if the file can't be read; it is tried again
// next time.
//
//////////////////////////////////////////////////////////////////////////
bool CSourceFile::Load(void)
{
	CTextFile	cFile;
	bool		bRead;

	if (mbLoaded)
	{
		return true;
	}

	cFile.Init();
	bRead = cFile.Read(mszFullName.Text());
	mszContents.Kill();
	mszContents.Init(bRead ? cFile.Text() : "");
	cFile.Kill();
	mbLoaded = bRead;
	return bRead;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSourceFile::IsLoaded(void) const
{
	return mbLoaded;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSourceFile::IsNamed(char* szName)
{
	return (strcmp(ShortName(), szName) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Contents given with SetContents are let go: the file is read again
// from its file when it is next loaded.
//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::ClearContents(void)
{
	mszContents.Kill();
	mszContents.Init();
	mbContentsSet = false;
	mbLoaded = false;
	maiLineStarts.Kill();
	maiLineStarts.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::SetContents(const char* sz)
{
	mszContents.Set(sz);
	mbLoaded = true;
	mbContentsSet = true;
	maiLineStarts.Kill();
	maiLineStarts.Init();
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CSourceFile::ShortName(void)
{
	size		iIndex;
	size		iOther;

	//Either separator ends the directory: a header found by its path ("Sub/Deep.h") has both on Windows.
	iIndex = mszFullName.FindFromEnd(FILE_SEPARATOR);
	iOther = mszFullName.FindFromEnd(OTHER_OS_FILE_SEPARATOR);
	if ((iOther != ARRAY_ELEMENT_NOT_FOUND) && ((iIndex == ARRAY_ELEMENT_NOT_FOUND) || (iOther > iIndex)))
	{
		iIndex = iOther;
	}
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return mszFullName.Text();
	}
	else
	{
		return mszFullName.Text(iIndex + 1);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CSourceFile::FullName(void)
{
	return mszFullName.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::DumpBlockSets(void)
{
	macBlockSets.DumpBlockSets();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Path(CChars* pszPath)
{
	CFileUtil	cFileUtil;

	//A name given with the other separator ("Lib/Sub/User.cpp" on Windows) is in Lib\Sub, not in the directory before it.
	pszPath->Init(mszFullName);
	cFileUtil.FixSeparators(pszPath);
	cFileUtil.RemoveLastFromPath(pszPath);
}


//////////////////////////////////////////////////////////////////////////
//
// The character a trigraph (??= and the others) stands for, or 0 if sz
// doesn't start one.  [lex.trigraph] 1
//
//////////////////////////////////////////////////////////////////////////
static char TrigraphCharacter(const char* sz, const char* szEnd)
{
	if ((szEnd - sz < 3) || (sz[0] != '?') || (sz[1] != '?'))
	{
		return 0;
	}
	switch (sz[2])
	{
	case '=':	return '#';
	case '/':	return '\\';
	case '\'':	return '^';
	case '(':	return '[';
	case ')':	return ']';
	case '!':	return '|';
	case '<':	return '{';
	case '>':	return '}';
	case '-':	return '~';
	default:	return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static bool IsTrigraphNameCharacter(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c == '_') || ((uint8)c >= 0x80);
}


//////////////////////////////////////////////////////////////////////////
//
// Translation phase 1: each trigraph is the character it stands for, but
// in a raw string, where the replacement is undone.  The contents are only
// changed if they have one; columns after one on its line are two less
// than the source's.  Comments, strings, characters (and digit
// separators, which aren't) are followed only to know where a raw string
// starts.  [lex.phases] 1.1, [lex.trigraph], [lex.pptoken] 3.1
//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::ReplaceTrigraphs(void)
{
	enum ETrigraphState
	{
		TS_Code,
		TS_LineComment,
		TS_BlockComment,
		TS_String,
		TS_Character,
	};
	const char*		sz;
	const char*		szEnd;
	const char*		szClose;
	CChars			szDest;
	CChars			szDelimiter;
	ETrigraphState	eState;
	size			uiPrefix;
	size			uiLength;
	char			c;
	char			cPrevious;
	bool			bInNumber;
	bool			bEscaped;
	bool			bChanged;

	sz = mszContents.Text();
	szEnd = sz + mszContents.Length();
	if (!strstr(sz, "??"))
	{
		return;
	}

	szDest.Init();
	eState = TS_Code;
	bInNumber = false;
	bEscaped = false;
	bChanged = false;
	cPrevious = '\n';
	while (sz < szEnd)
	{
		c = TrigraphCharacter(sz, szEnd);
		if (c != 0)
		{
			sz += 3;
			bChanged = true;
		}
		else
		{
			c = *sz;
			sz++;
		}

		switch (eState)
		{
		case TS_Code:
			if (c == '"')
			{
				//R", u8R", uR", UR" and LR" start a raw string, whose characters are copied as they are.
				uiLength = szDest.Length();
				uiPrefix = 0;
				if ((uiLength >= 1) && (szDest.GetChar(uiLength - 1) == 'R'))
				{
					uiPrefix = 1;
					if ((uiLength >= 3) && (szDest.GetChar(uiLength - 3) == 'u') && (szDest.GetChar(uiLength - 2) == '8'))
					{
						uiPrefix = 3;
					}
					else if ((uiLength >= 2) && ((szDest.GetChar(uiLength - 2) == 'u') || (szDest.GetChar(uiLength - 2) == 'U') || (szDest.GetChar(uiLength - 2) == 'L')))
					{
						uiPrefix = 2;
					}
					if ((uiLength > uiPrefix) && IsTrigraphNameCharacter(szDest.GetChar(uiLength - uiPrefix - 1)))
					{
						uiPrefix = 0;
					}
				}
				if (uiPrefix != 0)
				{
					szDest.Append(c);
					szDelimiter.Init(")");
					while ((sz < szEnd) && (*sz != '(') && (*sz != '\n'))
					{
						szDelimiter.Append(*sz);
						szDest.Append(*sz);
						sz++;
					}
					szDelimiter.Append('"');
					szClose = strstr(sz, szDelimiter.Text());
					szClose = szClose ? szClose + szDelimiter.Length() : szEnd;
					szDest.Append(sz, (int)(szClose - sz));
					sz = szClose;
					szDelimiter.Kill();
					cPrevious = '"';
					continue;
				}
				eState = TS_String;
				bEscaped = false;
			}
			else if (c == '\'')
			{
				szDest.Append(c);
				if (!(bInNumber && (sz < szEnd) && IsTrigraphNameCharacter(*sz)))
				{
					eState = TS_Character;
					bEscaped = false;
				}
				cPrevious = c;
				continue;
			}
			else if ((c == '/') && (sz < szEnd) && (*sz == '/'))
			{
				eState = TS_LineComment;
			}
			else if ((c == '/') && (sz < szEnd) && (*sz == '*'))
			{
				szDest.Append(c);
				szDest.Append(*sz);
				sz++;
				eState = TS_BlockComment;
				cPrevious = ' ';
				continue;
			}
			else if (IsTrigraphNameCharacter(c) || (c == '.'))
			{
				if (!IsTrigraphNameCharacter(cPrevious) && (cPrevious != '.') && (cPrevious != '\''))
				{
					bInNumber = ((c >= '0') && (c <= '9')) || ((c == '.') && (sz < szEnd) && (*sz >= '0') && (*sz <= '9'));
				}
			}
			else if (!(((c == '+') || (c == '-')) && bInNumber && ((cPrevious == 'e') || (cPrevious == 'E') || (cPrevious == 'p') || (cPrevious == 'P'))))
			{
				bInNumber = false;
			}
			break;

		case TS_LineComment:
			if ((c == '\n') && (cPrevious != '\\') && !((cPrevious == '\r') && (szDest.Length() >= 2) && (szDest.GetChar(szDest.Length() - 2) == '\\')))
			{
				eState = TS_Code;
			}
			break;

		case TS_BlockComment:
			if ((c == '/') && (cPrevious == '*'))
			{
				eState = TS_Code;
				c = '/';
				szDest.Append(c);
				cPrevious = ' ';
				continue;
			}
			break;

		case TS_String:
		case TS_Character:
			if (bEscaped)
			{
				bEscaped = false;
			}
			else if (c == '\\')
			{
				bEscaped = true;
			}
			else if (((c == '"') && (eState == TS_String)) || ((c == '\'') && (eState == TS_Character)) || (c == '\n'))
			{
				eState = TS_Code;
				bInNumber = false;
			}
			break;
		}
		szDest.Append(c);
		cPrevious = c;
	}

	if (bChanged)
	{
		mszContents.Kill();
		mszContents.Init(szDest.Text());
		maiLineStarts.Kill();
		maiLineStarts.Init();
	}
	szDest.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
const char* CSourceFile::GetContents(void) const
{
	return mszContents.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSourceFile::GetContentsLength(void) const
{
	return mszContents.Length();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size_t CSourceFile::GetTokenMemoryUsed(void)
{
	return mcTokenMemory.Size();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPTokenMemory* CSourceFile::GetTokenMemory(void)
{
	return &mcTokenMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPBlockSetArray* CSourceFile::GetBlockSets(void)
{
	return &macBlockSets;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CArrayPPComments* CSourceFile::GetComments(void)
{
	return &macComments;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::CalculateLineStarts(void)
{
	char*	sz;
	size	i;
	size	iLength;
	char	c;

	//Line breaks are counted as the preprocessor parser counts them: \n, \r, \r\n and \n\r.
	maiLineStarts.Kill();
	maiLineStarts.Init();
	maiLineStarts.Add(0);

	sz = mszContents.Text();
	iLength = mszContents.Length();
	for (i = 0; i < iLength; i++)
	{
		c = sz[i];
		if ((c == '\n') || (c == '\r'))
		{
			if ((i + 1 < iLength) && ((sz[i + 1] == '\n') || (sz[i + 1] == '\r')) && (sz[i + 1] != c))
			{
				i++;
			}
			maiLineStarts.Add((int)(i + 1));
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSourceFile::NumLines(void)
{
	if (maiLineStarts.NumElements() == 0)
	{
		CalculateLineStarts();
	}
	return maiLineStarts.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSourceFile::GetLineAndColumn(size iOffset, int* piLine, int* piColumn)
{
	size	iLow;
	size	iHigh;
	size	iMiddle;

	if (maiLineStarts.NumElements() == 0)
	{
		CalculateLineStarts();
	}

	if (iOffset > mszContents.Length())
	{
		return false;
	}

	//The last line that starts at or before iOffset.
	iLow = 0;
	iHigh = maiLineStarts.NumElements();
	while (iHigh - iLow > 1)
	{
		iMiddle = (iLow + iHigh) / 2;
		if ((size)maiLineStarts.GetValue(iMiddle) <= iOffset)
		{
			iLow = iMiddle;
		}
		else
		{
			iHigh = iMiddle;
		}
	}

	*piLine = (int)iLow;
	*piColumn = (int)(iOffset - maiLineStarts.GetValue(iLow));
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSourceFile::GetOffset(int iLine, int iColumn)
{
	if (maiLineStarts.NumElements() == 0)
	{
		CalculateLineStarts();
	}

	if ((iLine < 0) || ((size)iLine >= maiLineStarts.NumElements()))
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
	return maiLineStarts.GetValue(iLine) + iColumn;
}

