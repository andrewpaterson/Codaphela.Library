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
#include <string.h>
#include <stdlib.h>
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "CPP65Assembly.h"


#define CPP65_CODE_KEY			'C'
#define CPP65_CODE_KEY_SIZE		7


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Assembly::Init(CSpillStore* pcStore)
{
	mszText.Init();
	mpcStore = pcStore;
	mauiPieces.Init();
	mszPiece.Init();
	mszLine.Init();
	Rewind();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Assembly::Kill(void)
{
	lsize	ui;

	for (ui = 0; ui < mauiPieces.NumElements(); ui++)
	{
		RemoveCode(mpcStore, ui, 0, *mauiPieces.Get(ui));
	}
	mszLine.Kill();
	mszPiece.Kill();
	mauiPieces.Kill();
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The text the functions' markers are in, for the generator to write.
//
//////////////////////////////////////////////////////////////////////////
CLargeChars* CCPP65Assembly::GetText(void)
{
	return &mszText;
}


//////////////////////////////////////////////////////////////////////////
//
// How many pieces each function's code (in the store) is in.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Assembly::SetPieces(CArrayLargeTemplate<uint16>* pauiPieces)
{
	lsize	ui;

	mauiPieces.Kill();
	mauiPieces.Init();
	for (ui = 0; ui < pauiPieces->NumElements(); ui++)
	{
		mauiPieces.Add(pauiPieces->Get(ui));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// No text and no functions (a unit all left out by #if).
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Assembly::IsEmpty(void)
{
	return mszText.Empty() && (mauiPieces.NumElements() == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Assembly::Rewind(void)
{
	muiPos = 0;
	muiFunction = MAX_LSIZE;
	muiPiece = 0;
	muiPiecePos = 0;
	mszPiece.Clear();
	mbDone = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The next line of the text with each marker replaced by its function's code.
// Like CAsm65TextSource the last line is what follows the last new line.
//
//////////////////////////////////////////////////////////////////////////
const char* CCPP65Assembly::NextLine(void)
{
	const char*	szText;
	const char*	szStop;
	const char*	szEnd;
	char*		szIndex;

	if (mbDone)
	{
		return NULL;
	}
	mszLine.Clear();
	for (;;)
	{
		if (muiFunction != MAX_LSIZE)
		{
			//In a function's code: to the end of the line or of the piece.
			szText = mszPiece.Text(muiPiecePos);
			szEnd = mszPiece.Text() + mszPiece.Length();
			szStop = (const char*)memchr(szText, '\n', (size_t)(szEnd - szText));
			if (szStop)
			{
				mszLine.AppendSubString(szText, szStop + 1);
				muiPiecePos += (size)(szStop + 1 - szText);
				return mszLine.Text();
			}
			mszLine.AppendSubString(szText, szEnd);
			if (!LoadPiece())
			{
				muiFunction = MAX_LSIZE;
			}
			continue;
		}

		//In the text: to the end of the line, a marker or the end.
		szText = mszText.Text(muiPos);
		szEnd = mszText.Text() + mszText.Length();
		for (szStop = szText; (szStop < szEnd) && (*szStop != '\n') && (*szStop != CPP65_ASSEMBLY_MARKER); szStop++)
		{
		}
		if (szStop == szEnd)
		{
			mszLine.AppendSubString(szText, szEnd);
			muiPos = mszText.Length();
			mbDone = true;
			return mszLine.Text();
		}
		if (*szStop == '\n')
		{
			mszLine.AppendSubString(szText, szStop + 1);
			muiPos += (lsize)(szStop + 1 - szText);
			return mszLine.Text();
		}
		mszLine.AppendSubString(szText, szStop);
		muiFunction = (lsize)strtoul(szStop + 1, &szIndex, 10);
		muiPos = (lsize)(szIndex + 1 - mszText.Text());
		muiPiece = 0;
		if (!LoadPiece())
		{
			muiFunction = MAX_LSIZE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The function's next piece of code.  False after its last.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Assembly::LoadPiece(void)
{
	uint8	auiKey[CPP65_CODE_KEY_SIZE];
	size	uiKeySize;
	size	uiMax;
	size	uiLength;

	if ((muiFunction >= mauiPieces.NumElements()) || (muiPiece >= *mauiPieces.Get(muiFunction)))
	{
		return false;
	}
	uiKeySize = Key(auiKey, muiFunction, muiPiece);
	uiMax = mpcStore->MaxDataSize();
	mszPiece.Clear();
	mszPiece.Append(' ', uiMax);
	uiLength = 0;
	mpcStore->Get(auiKey, uiKeySize, mszPiece.Text(), uiMax, &uiLength);
	mszPiece.SetLength(uiLength);
	muiPiece++;
	muiPiecePos = 0;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole assembly (for tests and the static Generate).
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Assembly::Text(CChars* pszDest)
{
	const char*	szLine;

	Rewind();
	while ((szLine = NextLine()) != NULL)
	{
		pszDest->Append(szLine);
	}
	Rewind();
}


//////////////////////////////////////////////////////////////////////////
//
// Writes the assembly (a listing) a line at a time.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Assembly::Write(const char* szFilename)
{
	CFileBasic	cFile;
	const char*	szLine;
	lsize		uiLength;
	lsize		uiTotal;
	bool		bResult;

	cFile.Init(DiskFile(szFilename));
	if (!cFile.Open(EFM_Write_Create))
	{
		cFile.Kill();
		return false;
	}
	bResult = true;
	uiTotal = 0;
	Rewind();
	while ((szLine = NextLine()) != NULL)
	{
		uiLength = strlen(szLine);
		if (uiLength > 0)
		{
			bResult &= cFile.WriteData(szLine, uiLength);
			uiTotal += uiLength;
		}
	}
	Rewind();
	//Opening does not empty an existing file, so cut off anything longer.
	bResult &= cFile.Truncate(uiTotal);
	bResult &= cFile.Close();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Stores a function's code in pieces of at most the store's MaxDataSize, each
// ending at a new line where it can.  Pieces left from code stored before for
// the function (*puiPieces) are taken out.  *puiPieces is how many there are.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Assembly::PutCode(CSpillStore* pcStore, lsize uiFunction, const char* szCode, lsize uiLength, uint16* puiPieces)
{
	uint8		auiKey[CPP65_CODE_KEY_SIZE];
	size		uiKeySize;
	size		uiMax;
	lsize		uiStart;
	lsize		uiPieceLength;
	const char*	szLast;
	uint16		uiPiece;
	bool		bResult;

	uiMax = pcStore->MaxDataSize();
	bResult = true;
	uiPiece = 0;
	for (uiStart = 0; uiStart < uiLength; uiStart += uiPieceLength)
	{
		uiPieceLength = uiLength - uiStart;
		if (uiPieceLength > uiMax)
		{
			uiPieceLength = uiMax;
			for (szLast = szCode + uiStart + uiMax - 1; (szLast > szCode + uiStart) && (*szLast != '\n'); szLast--)
			{
			}
			if (*szLast == '\n')
			{
				uiPieceLength = (lsize)(szLast + 1 - (szCode + uiStart));
			}
		}
		uiKeySize = Key(auiKey, uiFunction, uiPiece);
		bResult &= pcStore->Put(auiKey, uiKeySize, szCode + uiStart, (size)uiPieceLength);
		uiPiece++;
	}
	RemoveCode(pcStore, uiFunction, uiPiece, *puiPieces);
	*puiPieces = uiPiece;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends a function's code (in uiPieces pieces) to pszDest.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Assembly::GetCode(CSpillStore* pcStore, lsize uiFunction, uint16 uiPieces, CChars* pszDest)
{
	uint8	auiKey[CPP65_CODE_KEY_SIZE];
	size	uiKeySize;
	size	uiMax;
	size	uiStart;
	size	uiLength;
	uint16	uiPiece;
	bool	bResult;

	uiMax = pcStore->MaxDataSize();
	bResult = true;
	for (uiPiece = 0; uiPiece < uiPieces; uiPiece++)
	{
		uiKeySize = Key(auiKey, uiFunction, uiPiece);
		uiStart = pszDest->Length();
		pszDest->Append(' ', uiMax);
		uiLength = 0;
		bResult &= pcStore->Get(auiKey, uiKeySize, pszDest->Text(uiStart), uiMax, &uiLength);
		pszDest->SetLength(uiStart + uiLength);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes pieces uiFrom up to (not including) uiTo of a function's code out.
//
//////////////////////////////////////////////////////////////////////////
void CCPP65Assembly::RemoveCode(CSpillStore* pcStore, lsize uiFunction, uint16 uiFrom, uint16 uiTo)
{
	uint8	auiKey[CPP65_CODE_KEY_SIZE];
	size	uiKeySize;
	uint16	uiPiece;

	for (uiPiece = uiFrom; uiPiece < uiTo; uiPiece++)
	{
		uiKeySize = Key(auiKey, uiFunction, uiPiece);
		pcStore->Remove(auiKey, uiKeySize);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A piece's key: the kind, then the function's index and the piece's number,
// most significant byte first (so a function's pieces are together).
//
//////////////////////////////////////////////////////////////////////////
size CCPP65Assembly::Key(uint8* puiKey, lsize uiFunction, uint16 uiPiece)
{
	puiKey[0] = CPP65_CODE_KEY;
	puiKey[1] = (uint8)(uiFunction >> 24);
	puiKey[2] = (uint8)(uiFunction >> 16);
	puiKey[3] = (uint8)(uiFunction >> 8);
	puiKey[4] = (uint8)uiFunction;
	puiKey[5] = (uint8)(uiPiece >> 8);
	puiKey[6] = (uint8)uiPiece;
	return CPP65_CODE_KEY_SIZE;
}
