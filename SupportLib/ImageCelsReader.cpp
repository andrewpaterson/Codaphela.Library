/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela SupportLib

Codaphela SupportLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela SupportLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela SupportLib.  If not, see <http://www.gnu.org/licenses/>.

libpng is Copyright Glenn Randers-Pehrson
zlib is Copyright Jean-loup Gailly and Mark Adler

** ------------------------------------------------------------------------ **/
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextParser.h"
#include "ImageReader.h"
#include "ImageCelsReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetImageCelsQuotedString(CTextParser* pcTextParser, CChars* szDest)
{
	const char*		szStart;
	const char*		szEnd;

	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('"'));
	szStart = pcTextParser->mszParserPos;
	ReturnOnErrorAndFalse(pcTextParser->FindExactCharacterSequence("\""));
	szEnd = pcTextParser->mszParserPos;
	pcTextParser->StepRight();

	szDest->AppendSubString(szStart, szEnd);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetImageCelsIdentifierAndString(CTextParser* pcTextParser, char* szIdentifier, CChars* szDest)
{
	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier(szIdentifier));
	ReturnOnErrorAndFalse(GetImageCelsQuotedString(pcTextParser, szDest));
	pcTextParser->GetExactCharacter(';');
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetImageCelsIdentifierAndIntegers(CTextParser* pcTextParser, char* szIdentifier, int32* aiValues, size uiCount)
{
	size	i;

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier(szIdentifier));
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));
	for (i = 0; i < uiCount; i++)
	{
		if (i != 0)
		{
			ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter(','));
		}
		ReturnOnErrorAndFalse(pcTextParser->GetInteger(&aiValues[i]));
	}
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('}'));
	pcTextParser->GetExactCharacter(';');
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
// Reads a block with a FileName and a Directory into one path.
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetImageCelsPath(CTextParser* pcTextParser, char* szIdentifier, CChars* szPathName)
{
	CChars		szFileName;
	CChars		szDirectory;
	CFileUtil	cFileUtil;
	TRISTATE	tResult;

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier(szIdentifier));
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));

	szFileName.Init();
	szDirectory.Init();
	tResult = GetImageCelsIdentifierAndString(pcTextParser, "FileName", &szFileName);
	if (tResult == TRITRUE)
	{
		tResult = GetImageCelsIdentifierAndString(pcTextParser, "Directory", &szDirectory);
	}
	if (tResult == TRITRUE)
	{
		tResult = pcTextParser->GetExactCharacter('}');
	}

	if (tResult == TRITRUE)
	{
		szPathName->Append(szDirectory);
		cFileUtil.AppendToPath(szPathName, szFileName.Text());
		cFileUtil.FixSeparators(szPathName);
		cFileUtil.CollapsePath(szPathName);
	}
	szFileName.Kill();
	szDirectory.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE GetImageCelsCel(CTextParser* pcTextParser, Ptr<CImage> pImage, Ptr<CArrayImageCel> paCels)
{
	int64			iIndex;
	CChars			szName;
	int32			aiRect[4];
	int32			aiOffset[4];
	int32			aiAlign[2];
	TRISTATE		tResult;
	Ptr<CImageCel>	pCel;

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("Cel"));
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));

	//Index is optional.  If it is present it must be the position the cel is added at.
	tResult = pcTextParser->GetHFExactIdentifierAndInteger("Index", &iIndex);
	ReturnOnError(tResult);
	if (tResult == TRITRUE)
	{
		pcTextParser->GetExactCharacter(';');
		if ((iIndex < 0) || ((size)iIndex != paCels->NumElements()))
		{
			return TRIERROR;
		}
	}

	//Name is optional.
	szName.Init();
	tResult = GetImageCelsIdentifierAndString(pcTextParser, "Name", &szName);
	if (tResult == TRIERROR)
	{
		szName.Kill();
		return TRIERROR;
	}

	//Rect and Offset are {top, left, bottom, right}.  Align is {x, y}.
	tResult = GetImageCelsIdentifierAndIntegers(pcTextParser, "Rect", aiRect, 4);
	if (tResult == TRITRUE)
	{
		tResult = GetImageCelsIdentifierAndIntegers(pcTextParser, "Offset", aiOffset, 4);
	}
	if (tResult == TRITRUE)
	{
		tResult = GetImageCelsIdentifierAndIntegers(pcTextParser, "Align", aiAlign, 2);
	}
	if (tResult == TRITRUE)
	{
		tResult = pcTextParser->GetExactCharacter('}');
	}
	if (tResult != TRITRUE)
	{
		szName.Kill();
		return TRIERROR;
	}

	if (szName.Empty())
	{
		pCel = OMalloc<CImageCel>(pImage, aiRect[1], aiRect[0], aiRect[3], aiRect[2], aiOffset[1], aiOffset[0], aiOffset[3], aiOffset[2]);
	}
	else
	{
		pCel = ONMalloc<CImageCel>(szName.Text(), pImage, aiRect[1], aiRect[0], aiRect[3], aiRect[2], aiOffset[1], aiOffset[0], aiOffset[3], aiOffset[2]);
	}
	szName.Kill();
	if (pCel.IsNull())
	{
		return TRIERROR;
	}

	//The alignment is stored as it is, not as SUB_IMAGE_ALIGNMENT flags.
	pCel->GetSubImage()->msAlignment.Init(aiAlign[0], aiAlign[1]);
	paCels->Add(pCel);
	return TRITRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CArrayImageCel> CelsReader(char* szPathName)
{
	CTextParser 		cTextParser;
	CTextFile			cTextFile;
	CChars				szImagePathName;
	TRISTATE			tResult;
	Ptr<CImage>			pImage;
	Ptr<CArrayImageCel>	paCels;

	cTextFile.Init();
	if (!cTextFile.Read(szPathName))
	{
		cTextFile.Kill();
		return NULL;
	}
	cTextFile.PassifyNewlines();
	cTextParser.Init(cTextFile.Text(), cTextFile.Length());

	szImagePathName.Init();
	tResult = cTextParser.GetExactIdentifier("ImageCels");
	if (tResult == TRITRUE)
	{
		tResult = cTextParser.GetExactCharacter('{');
	}
	if (tResult == TRITRUE)
	{
		tResult = GetImageCelsPath(&cTextParser, "Image", &szImagePathName);
	}
	if (tResult != TRITRUE)
	{
		szImagePathName.Kill();
		cTextParser.Kill();
		cTextFile.Kill();
		return NULL;
	}

	pImage = ReadImage(szImagePathName.Text());
	szImagePathName.Kill();
	if (pImage.IsNull())
	{
		cTextParser.Kill();
		cTextFile.Kill();
		return NULL;
	}

	paCels = OMalloc<CArrayImageCel>();
	for (;;)
	{
		tResult = GetImageCelsCel(&cTextParser, pImage, paCels);
		if (tResult != TRITRUE)
		{
			break;
		}
	}

	if (tResult == TRIFALSE)
	{
		tResult = cTextParser.GetExactCharacter('}');
	}

	cTextParser.Kill();
	cTextFile.Kill();

	if (tResult != TRITRUE)
	{
		paCels->Kill();
		return NULL;
	}
	return paCels;
}

