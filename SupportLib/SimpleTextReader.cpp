#include "BaseLib/NaiveFile.h"
#include "BaseLib/UnicodeWriter.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/UTF16.h"
#include "TextBuilder.h"
#include "SimpleTextReader.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextReader::Init(CFont* pcFont)
{
	mpcFont = pcFont;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextReader::Kill(void)
{
	mpcFont = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnicodeReader* CSimpleTextReader::CreateReader(char* szFilename)
{
	CUTF8* pcUTF8;
	CUTF16* pcUTF16;
	CNaiveFile			cFile;
	bool				bResult;
	void* pvData;
	size				uiLength;
	EUnicodeEncoding	eEncoding;
	CUnicodeReader* pcReader;

	cFile.Init();
	bResult = cFile.Read(szFilename);
	if (!bResult)
	{
		cFile.Kill();
		return NULL;
	}

	pvData = cFile.Get();
	uiLength = (size)cFile.Length();

	eEncoding = GetUnicodeEncoding(pvData, uiLength);
	if (eEncoding == UE_UTF8 || eEncoding == UE_UTF8BOM)
	{
		pcUTF8 = NewMalloc<CUTF8>();
		pcUTF8->Init((uint8*)pvData, uiLength);
		if (eEncoding == UE_UTF8BOM)
		{
			bResult = pcUTF8->GetByteOrderMark();
			if (!bResult)
			{
				pcUTF8->Kill();
				free(pcUTF8);
				cFile.Kill();
				return NULL;
			}
		}
		pcReader = pcUTF8;
	}
	else if (eEncoding == UE_UTF16LE || eEncoding == UE_UTF16BE)
	{
		pcUTF16 = NewMalloc<CUTF16>();
		pcUTF16->Init((uint16*)pvData, uiLength);
		bResult = pcUTF16->GetByteOrderMark();
		if (!bResult)
		{
			pcUTF16->Kill();
			free(pcUTF16);
			cFile.Kill();
			return NULL;
		}
		pcReader = pcUTF16;
	}
	else
	{
		cFile.Kill();
		pcReader = NULL;
	}

	return pcReader;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CText> CSimpleTextReader::Read(char* szFilename)
{
	CUnicodeReader*		pcUnicodeReader;
	Ptr<CText>			pText;
	uint32				cCodePoint32;
	size				uiUTFLength;
	uint8				auiBuffer[0x13];
	size				uiCodePointsLength;
	size				uiNumWritten;
	CTextBuilder		cTextBuilder;
	bool				bResult;

	pcUnicodeReader = CreateReader(szFilename);
	if (pcUnicodeReader == NULL)
	{
		return NULL;
	}

	pText = OMalloc<CText>();
	cTextBuilder.Init(pcUnicodeReader, &pText, mpcFont);
	
	uiNumWritten = 0;
	uiUTFLength = pcUnicodeReader->PeekUTFBytes();
	while (uiUTFLength != 0)
	{
		if (uiUTFLength <= 4)
		{
			cCodePoint32 = pcUnicodeReader->GetCodePointUint32();
			if (pcUnicodeReader->IsError(cCodePoint32))
			{
				return NULL;
			}

			if (cCodePoint32 == UNICODE_LINE_FEED)
			{
				bResult = cTextBuilder.PushNewLine();
				if (!bResult)
				{
					return NULL;
				}
			}
			else if (cCodePoint32 == UNICODE_LINE_FEED)
			{
				// Do nothing.
			}
			else
			{
				bResult = cTextBuilder.Push(cCodePoint32);
				if (!bResult)
				{
					return NULL;
				}
			}
		}
		else
		{
			uiCodePointsLength = pcUnicodeReader->GetCodePointMulti(auiBuffer, 0x13);
			bResult = cTextBuilder.Push(auiBuffer, uiCodePointsLength);
			if (!bResult)
			{
				return NULL;
			}
		}
		uiUTFLength = pcUnicodeReader->PeekUTFBytes();
		uiNumWritten++;
	}

	bResult = cTextBuilder.Done();
	if (!bResult)
	{
		return NULL;
	}
	cTextBuilder.Kill();

	return pText;
}

