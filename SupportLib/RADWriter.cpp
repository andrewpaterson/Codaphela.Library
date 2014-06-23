/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "BaseLib/NaiveFile.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FileUtil.h"
#include "CoreLib/TypeNames.h"
#include "RADWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SaveRAD(Ptr<CImage> pcImage, char *szPathName)
{
	CTextFile	cTextFile;
	CChars*		szText;
	CChannel*	psChannel;
	int			j;
	CTypeNames	cTypeNames;
	char*		szTypeName;
	CChars		szRawName;
	CFileUtil	cFileUtil;
	BOOL		bResult;

	cTextFile.Init();
	szText = &cTextFile.mcText;

	szText->Append("RawImage\n{\n\tWidth\t");
	szText->Append(pcImage->GetWidth());
	szText->Append(";\n\tHeight\t");
	szText->Append(pcImage->GetHeight());
	szText->Append(";\n\tChannels\n\t{\n");

	cTypeNames.Init();

	for (j = 0; j < pcImage->GetChannels()->GetChannelOffsets()->NumElements(); j++)
	{
		psChannel = pcImage->GetChannels()->GetChannelOffsets()->Get(j);

		szTypeName = cTypeNames.GetPrettyName(psChannel->eType);
		szText->Append("\t\t");
		szText->Append(szTypeName);
		szText->Append("\t");
		
		if (psChannel->iChannel == IMAGE_DIFFUSE_RED) szText->Append("Red");
		else if (psChannel->iChannel == IMAGE_DIFFUSE_GREEN) szText->Append("Green");
		else if (psChannel->iChannel == IMAGE_DIFFUSE_BLUE) szText->Append("Blue");
		else if (psChannel->iChannel == IMAGE_DIFFUSE_GREY) szText->Append("Grey");
		else if (psChannel->iChannel == CHANNEL(IP_Diffuse, CT_Hue)) szText->Append("Hue");
		else if (psChannel->iChannel == CHANNEL(IP_Diffuse, CT_Saturation)) szText->Append("Saturation");
		else if (psChannel->iChannel == CHANNEL(IP_Diffuse, CT_Value)) szText->Append("Value");
		else if (psChannel->iChannel == IMAGE_OPACITY) szText->Append("Oppacity");
		else if (psChannel->iChannel == IMAGE_SPECULAR) szText->Append("SpecularIntensity");
		else if (psChannel->iChannel == CHANNEL(IP_Normal, CT_X)) szText->Append("NormalX");
		else if (psChannel->iChannel == CHANNEL(IP_Normal, CT_Y)) szText->Append("NormalY");
		else if (psChannel->iChannel == CHANNEL(IP_Normal, CT_Z)) szText->Append("NormalZ");
		else if (psChannel->iChannel == CHANNEL(IP_Illumination, CT_Intensity)) szText->Append("SelfIllumination");
		else if (psChannel->iChannel == CHANNEL(IP_UVAdjust, CT_U)) szText->Append("AdjustU");
		else if (psChannel->iChannel == CHANNEL(IP_UVAdjust, CT_V)) szText->Append("AdjustV");

		szText->Append(";\n");
	}
	szText->Append("\t}\n");
	szText->Append("\tFileName\t\"");

	szRawName.Init(szPathName);
	cFileUtil.RemovePath(&szRawName);
	cFileUtil.RemoveExtension(&szRawName);
	szRawName.Append(".raw");
	szText->Append(szRawName);
	szRawName.Kill();

	szText->Append("\";\n}\n");

	cTypeNames.Kill();

	cTextFile.Write(szPathName);
	cTextFile.Kill();

	szRawName.Init(szPathName);
	cFileUtil.RemoveExtension(&szRawName);
	szRawName.Append(".raw");

	bResult = SaveRAW(pcImage, szRawName.Text());
	szRawName.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SaveRAW(Ptr<CImage> pcImage, char* szFileName)
{
	CNaiveFile	cNaiveFile;
	BOOL		bResult;

	cNaiveFile.Init();
	cNaiveFile.Set(pcImage->GetData(), pcImage->GetByteSize());
	bResult = cNaiveFile.Write(szFileName);
	cNaiveFile.Kill();

	return bResult;
}


