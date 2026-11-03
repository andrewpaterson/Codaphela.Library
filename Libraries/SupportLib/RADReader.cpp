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
#include "BaseLib/EnumeratorVoid.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextParser.h"
#include "ImageReaderHelper.h"
#include "RADReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool AddChannelsFromText(CTextParser* pcTextParser, Ptr<CImage> pImport)
{
	CEnumeratorVoid		cChannels;
	CEnumeratorVoid		cTypes;
	size				iType;
	size				iChannel;
	TRISTATE			tResult;

	if (!pcTextParser->GetExactCharacter('{'))
	{ 
		return false;
	}

	cChannels.Init();
	cChannels.Add("Red",				IMAGE_DIFFUSE_RED);
	cChannels.Add("Green",				IMAGE_DIFFUSE_GREEN);
	cChannels.Add("Blue",				IMAGE_DIFFUSE_BLUE);
	cChannels.Add("Grey",				IMAGE_DIFFUSE_GREY);
	cChannels.Add("Hue",				CHANNEL(IP_Diffuse, CT_Hue));
	cChannels.Add("Saturation",			CHANNEL(IP_Diffuse, CT_Saturation));
	cChannels.Add("Value",				CHANNEL(IP_Diffuse, CT_Value));
	cChannels.Add("Oppacity",			IMAGE_OPACITY);
	cChannels.Add("SpecularIntensity",	IMAGE_SPECULAR);
	cChannels.Add("NormalX",			CHANNEL(IP_Normal, CT_X));
	cChannels.Add("NormalY",			CHANNEL(IP_Normal, CT_Y));
	cChannels.Add("NormalZ",			CHANNEL(IP_Normal, CT_Z));
	cChannels.Add("SelfIllumination",	CHANNEL(IP_Illumination, CT_Intensity));
	cChannels.Add("AdjustU", 			CHANNEL(IP_UVAdjust, CT_U));
	cChannels.Add("AdjustV", 			CHANNEL(IP_UVAdjust, CT_V));

	cTypes.Init();
	cTypes.Add("Char",		PT_int8);
	cTypes.Add("UByte",		PT_uint8);
	cTypes.Add("Short",		PT_int16);
	cTypes.Add("UShort",	PT_uint16);
	cTypes.Add("Int",		PT_int32);
	cTypes.Add("UInt",		PT_uint32);
	cTypes.Add("Float", 	PT_float32);
	cTypes.Add("Bit",		PT_bit);
	cTypes.Add("Crumb",		PT_crumb);
	cTypes.Add("Nybble", 	PT_nybble);
	cTypes.Add("Nickle", 	PT_nickle);

	while (pcTextParser->GetExactCharacter('}') != TRITRUE)
	{
		//Get the type...
		tResult = pcTextParser->GetEnumeratorIdentifier(&cTypes, &iType);
		if ((tResult == TRIERROR) || (tResult == TRIFALSE)) 
		{
			cTypes.Kill();
			cChannels.Kill();
			return false;
		}

		//Get the channel;
		tResult = pcTextParser->GetEnumeratorIdentifier(&cChannels, &iChannel);
		if ((tResult == TRIERROR) || (tResult == TRIFALSE)) 
		{
			cTypes.Kill();
			cChannels.Kill();
			return false;
		}

		pImport->AddChannel(iChannel, (EPrimitiveType)iType);

		//Optionally get a semicolon.
		pcTextParser->GetExactCharacter(';');
	}

	cTypes.Kill();
	cChannels.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool SetFromText(CTextParser* pcTextParser, Ptr<CImage> pImport, CChars* szRawFileName)
{
	int64		iWidth;
	int64		iHeight;
	const char*		szFileNameStart;
	const char*		szFileNameEnd;
	CFileUtil	cFileUtil;
	CChars		sz;

	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));

	ReturnOnErrorAndFalse(pcTextParser->GetHFExactIdentifierAndInteger("Width", &iWidth));
	pcTextParser->GetExactCharacter(';');

	ReturnOnErrorAndFalse(pcTextParser->GetHFExactIdentifierAndInteger("Height", &iHeight));
	pcTextParser->GetExactCharacter(';');

	pImport->SetSize((int)iWidth, (int)iHeight);
	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("Channels"));

	ReturnOnFalse(AddChannelsFromText(pcTextParser, pImport));
	pcTextParser->GetExactCharacter(';');

	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("FileName"));
	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('"'));
	szFileNameStart = pcTextParser->mszParserPos;
	ReturnOnErrorAndFalse(pcTextParser->FindExactCharacterSequence("\""));
	szFileNameEnd = pcTextParser->mszParserPos;
	pcTextParser->StepRight();
	pcTextParser->GetExactCharacter(';');
	sz.Init();
	sz.AppendSubString(szFileNameStart, szFileNameEnd);
	cFileUtil.AppendToPath(szRawFileName, sz.Text());
	sz.Kill();
	cFileUtil.FixSeparators(szRawFileName);
	cFileUtil.CollapsePath(szRawFileName);

	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('}'));
	pcTextParser->GetExactCharacter(';');
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CImage> LoadRAD(char *szFilename, bool bAddDebug)
{
	CTextParser 	cTextParser;
	CTextFile		cTextFile;
	CChars			szRawFilename;
	CChars			szRadFilename;
	lsize			iImageSize;
	CFileBasic		sFile;
	CFileUtil		cFileUtil;

	szRadFilename.Init(szFilename);
	cFileUtil.FullPath(&szRadFilename);

	cTextFile.Init();
	if (!cTextFile.Read(szFilename))
	{
		cTextFile.Kill();
		return NULL;
	}
	cTextFile.PassifyNewlines();
	cTextParser.Init(cTextFile.Text(), cTextFile.Length());

	//Start parsing now.
	if (!cTextParser.GetExactIdentifier("RawImage"))
	{
		cTextFile.Kill();
		cTextParser.Kill();
		return NULL;
	}

	Ptr<CImage> pImage = OMalloc<CImage>();
	pImage->BeginChange();

	//Create an image from the text descriptor.
	szRawFilename.Init(szRadFilename);
	cFileUtil.RemoveLastFromPath(&szRawFilename);
	szRadFilename.Kill();

	if (!SetFromText(&cTextParser, pImage, &szRawFilename))
	{
		pImage->Kill();
		szRawFilename.Kill();
		cTextFile.Kill();
		cTextParser.Kill();
		return NULL;
	}

	pImage->EndChange();

	if (szRawFilename.Empty())
	{
		pImage->Kill();
		szRawFilename.Kill();
		cTextFile.Kill();
		cTextParser.Kill();
		return NULL;
	}

	//Finished parsing the rad file 
	cTextFile.Kill();
	cTextParser.Kill();

	//Get the expected size of the source data.
	iImageSize = pImage->GetByteSize();

	sFile.Init(DiskFile(szRawFilename.Text()));
	szRawFilename.Kill();
	if (!sFile.Open(EFM_Read))
	{
		pImage->Kill();
		sFile.Kill();
		return NULL;
	}

	if ((lsize)sFile.Size() != iImageSize)
	{
		pImage->Kill();
		sFile.Close();
		sFile.Kill();
		return NULL;
	}

	if (pImage->GetChannels()->GetByteSize() != iImageSize)
	{
		pImage->Kill();
		sFile.Close();
		sFile.Kill();
		return NULL;
	}

	sFile.Read(pImage->GetChannels()->GetData(), iImageSize, 1);

	//Close the file
	sFile.Close();
	sFile.Kill();

	return pImage;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool LoadRAW(char *szFilename, Ptr<CImage> pImage)
{
	CFileBasic	sFile;
	lsize		iImageSize;
	filePos		iResult;

	sFile.Init(DiskFile(szFilename));
	if (!sFile.Open(EFM_Read))
	{
		sFile.Kill();
		return false;
	}

	iImageSize = pImage->GetByteSize();

	if ((lsize)sFile.Size() != iImageSize)
	{
		sFile.Close();
		sFile.Kill();
		return false;
	}

	if (pImage->GetChannels()->GetByteSize() != iImageSize)
	{
		sFile.Close();
		sFile.Kill();
		return false;
	}

	iResult = sFile.Read(pImage->GetChannels()->GetData(), iImageSize, 1);

	//Close the file
	sFile.Close();
	sFile.Kill();
	return iResult == 1;
}

