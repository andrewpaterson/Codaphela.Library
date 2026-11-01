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
#include "ImageCelsWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AppendImageCelsIntegers(CLargeChars* szText, char* szIdentifier, int32* aiValues, size uiCount)
{
	size	i;

	szText->Append("\t\t");
	szText->Append(szIdentifier);
	szText->Append("\t{");
	for (i = 0; i < uiCount; i++)
	{
		if (i != 0)
		{
			szText->Append(", ");
		}
		szText->Append(aiValues[i]);
	}
	szText->Append("};\n");
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a block with the FileName and Directory of szPathName.
//
//////////////////////////////////////////////////////////////////////////
void AppendImageCelsPath(CLargeChars* szText, char* szIdentifier, char* szPathName)
{
	CChars		szFileName;
	CChars		szDirectory;
	CFileUtil	cFileUtil;

	szFileName.Init();
	szDirectory.Init();
	cFileUtil.SplitPath(szPathName, &szFileName, &szDirectory);

	//Always write '/' so the file is the same on every platform.  The reader fixes the separators.
	szDirectory.Replace('\\', '/');

	szText->Append("\t");
	szText->Append(szIdentifier);
	szText->Append("\n\t{\n\t\tFileName\t\"");
	szText->Append(szFileName);
	szText->Append("\";\n\t\tDirectory\t\"");
	szText->Append(szDirectory);
	szText->Append("\";\n\t}\n");

	szFileName.Kill();
	szDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CelsWriter(Ptr<CArrayImageCel> paCels, char* szImagePathName, char* szPathName)
{
	CTextFile		cTextFile;
	CLargeChars*	szText;
	size			i;
	size			uiNumCels;
	Ptr<CImageCel>	pCel;
	Ptr<CImage>		pImage;
	CSubImage*		pcSubImage;
	int32			aiValues[4];
	bool			bResult;

	uiNumCels = paCels->NumElements();
	for (i = 0; i < uiNumCels; i++)
	{
		pCel = paCels->Get(i);
		if (i == 0)
		{
			pImage = pCel->GetSourceImage();
		}
		else if (pCel->GetSourceImage() != pImage)
		{
			return false;
		}
	}

	cTextFile.Init();
	szText = &cTextFile.mcText;

	szText->Append("ImageCels\n{\n");
	AppendImageCelsPath(szText, "Image", szImagePathName);

	for (i = 0; i < uiNumCels; i++)
	{
		pCel = paCels->Get(i);
		pcSubImage = pCel->GetSubImage();

		szText->Append("\tCel\n\t{\n\t\tIndex\t");
		szText->Append((int32)i);
		szText->Append(";\n");

		if (pCel->IsNamed())
		{
			szText->Append("\t\tName\t\"");
			szText->Append(pCel->GetName());
			szText->Append("\";\n");
		}

		aiValues[0] = pcSubImage->mcImageRect.msTopLeft.y;
		aiValues[1] = pcSubImage->mcImageRect.msTopLeft.x;
		aiValues[2] = pcSubImage->mcImageRect.msBottomRight.y;
		aiValues[3] = pcSubImage->mcImageRect.msBottomRight.x;
		AppendImageCelsIntegers(szText, "Rect", aiValues, 4);

		aiValues[0] = pcSubImage->msOffsetTopLeft.y;
		aiValues[1] = pcSubImage->msOffsetTopLeft.x;
		aiValues[2] = pcSubImage->msOffsetBottomRight.y;
		aiValues[3] = pcSubImage->msOffsetBottomRight.x;
		AppendImageCelsIntegers(szText, "Offset", aiValues, 4);

		aiValues[0] = pcSubImage->msAlignment.x;
		aiValues[1] = pcSubImage->msAlignment.y;
		AppendImageCelsIntegers(szText, "Align", aiValues, 2);

		szText->Append("\t}\n");
	}
	szText->Append("}\n");

	bResult = cTextFile.Write(szPathName);
	cTextFile.Kill();
	return bResult;
}

