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
	mcTokenMemory.Init();
	macBlockSets.Init();
	mbBlockSetsKilled = false;
	macComments.Init();
	maiLineStarts.Init();
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
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::Load(void)
{
	CTextFile	cFile;

	if (!mbLoaded)
	{
		mbLoaded = true;

		cFile.Init();
		cFile.Read(mszFullName.Text());

		mszContents.Kill();
		mszContents.Init(cFile.Text());

		cFile.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CSourceFile::IsLoaded(void)
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
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CSourceFile::SetContents(const char* sz)
{
	mszContents.Set(sz);
	mbLoaded = true;
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

	iIndex = mszFullName.FindFromEnd(FILE_SEPARATOR);
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return mszFullName.Text();
	}
	else
	{
		return mszFullName.Text(iIndex+1);
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

	pszPath->Init(mszFullName);
	cFileUtil.RemoveLastFromPath(pszPath);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CSourceFile::GetContents(void)
{
	return mszContents.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CSourceFile::GetContentsLength(void)
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

