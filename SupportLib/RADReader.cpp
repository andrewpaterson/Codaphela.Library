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
#include "BaseLib/EnumeratorVoid.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/FileUtil.h"
#include "CoreLib/TextParser.h"
#include "RADReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL AddChannelsFromText(CTextParser* pcTextParser, CImage* pcImport)
{
	CEnumeratorVoid	cChannels;
	CEnumeratorVoid	cTypes;
	int				iType;
	int				iChannel;
	TRISTATE		tResult;

	if (!pcTextParser->GetExactCharacter('{'))
	{ 
		return FALSE;
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
	cTypes.Add("Char",		PT_char);
	cTypes.Add("UChar",		PT_uchar);
	cTypes.Add("Short",		PT_short);
	cTypes.Add("UShort",	PT_ushort);
	cTypes.Add("Int",		PT_int);
	cTypes.Add("UInt",		PT_uint);
	cTypes.Add("Float", 	PT_float);
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
			return FALSE;
		}

		//Get the channel;
		tResult = pcTextParser->GetEnumeratorIdentifier(&cChannels, &iChannel);
		if ((tResult == TRIERROR) || (tResult == TRIFALSE)) 
		{
			cTypes.Kill();
			cChannels.Kill();
			return FALSE;
		}

		pcImport->AddChannel(iChannel, (EPrimitiveTypes)iType);

		//Optionally get a semicolon.
		pcTextParser->GetExactCharacter(';');
	}

	cTypes.Kill();
	cChannels.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SetFromText(CTextParser* pcTextParser, CImage* pcImport, CChars* szRawFileName)
{
	int				iWidth;
	int				iHeight;
	char*			szFileNameStart;
	char*			szFileNameEnd;
	CFileUtil		cFileUtil;
	CChars			sz;

	ReturnOnErrorAndFalse(pcTextParser->GetExactCharacter('{'));

	ReturnOnErrorAndFalse(pcTextParser->GetHFExactIdentifierAndInteger("Width", &iWidth));
	pcTextParser->GetExactCharacter(';');

	ReturnOnErrorAndFalse(pcTextParser->GetHFExactIdentifierAndInteger("Height", &iHeight));
	pcTextParser->GetExactCharacter(';');

	pcImport->SetSize(iWidth, iHeight);
	ReturnOnErrorAndFalse(pcTextParser->GetExactIdentifier("Channels"));

	ReturnOnFalse(AddChannelsFromText(pcTextParser, pcImport));
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
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL LoadRAD(CImage *pcImage, char *szFileName)
{
	CTextParser 	cTextParser;
	CTextFile		cTextFile;
	CChars			szRawFilename;
	CChars			szRadFilename;
	int				iImageSize;
	CFileBasic		sFile;
	CFileUtil		cFileUtil;

	szRadFilename.Init(szFileName);
	cFileUtil.FullPath(&szRadFilename);

	cTextFile.Init();
	if (!cTextFile.Read(szFileName))
	{
		cTextFile.Kill();
		return FALSE;
	}
	cTextFile.PassifyNewlines();
	cTextParser.Init(cTextFile.Text(), cTextFile.Length());

	//Start parsing now.
	if (!cTextParser.GetExactIdentifier("RawImage"))
	{
		cTextFile.Kill();
		cTextParser.Kill();
		return FALSE;
	}

	pcImage->Init();
	pcImage->BeginChange();

	//Create an image from the text descriptor.
	szRawFilename.Init(szRadFilename);
	cFileUtil.RemoveLastFromPath(&szRawFilename);

	if (!SetFromText(&cTextParser, pcImage, &szRawFilename))
	{
		pcImage->Kill();
		szRawFilename.Kill();
		cTextFile.Kill();
		cTextParser.Kill();
		return FALSE;
	}

	pcImage->EndChange();

	if (szRawFilename.Empty())
	{
		pcImage->Kill();
		szRawFilename.Kill();
		cTextFile.Kill();
		cTextParser.Kill();
		return FALSE;
	}

	//Finished parsing the rad file 
	cTextFile.Kill();
	cTextParser.Kill();

	//Get the expected size of the source data.
	iImageSize = pcImage->GetByteSize();

	sFile.Init(DiskFile(szRawFilename.Text()));
	if (!sFile.Open(EFM_Read))
	{
		pcImage->Kill();
		sFile.Kill();
		return FALSE;
	}

	if (sFile.GetFileSize() != iImageSize)
	{
		pcImage->Kill();
		sFile.Close();
		sFile.Kill();
		return FALSE;
	}

	if (pcImage->GetChannels()->GetByteSize() != iImageSize)
	{
		pcImage->Kill();
		sFile.Close();
		sFile.Kill();
		return FALSE;
	}

	sFile.Read(pcImage->GetChannels()->GetData(), iImageSize, 1);

	//Close the file
	sFile.Close();
	sFile.Kill();

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL LoadRAW(CImage *pcImage, char *szFileName)
{
	CFileBasic	sFile;
	int			iImageSize;
	filePos		iResult;

	sFile.Init(DiskFile(szFileName));
	if (!sFile.Open(EFM_Read))
	{
		sFile.Kill();
		return FALSE;
	}

	iImageSize = pcImage->GetByteSize();

	if (sFile.GetFileSize() != iImageSize)
	{
		sFile.Close();
		sFile.Kill();
		return FALSE;
	}

	if (pcImage->mcChannels.GetByteSize() != iImageSize)
	{
		sFile.Close();
		sFile.Kill();
		return FALSE;
	}

	iResult = sFile.Read(pcImage->mcChannels.GetData(), iImageSize, 1);

	//Close the file
	sFile.Close();
	sFile.Kill();
	return iResult == 1;
}