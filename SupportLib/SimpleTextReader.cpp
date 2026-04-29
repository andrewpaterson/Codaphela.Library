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
	mcFile.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CSimpleTextReader::Kill(void)
{
	mcFile.Kill();
	mpcFont = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CUnicodeReader* CSimpleTextReader::CreateReader(char* szFilename)
{
	CUTF8*				pcUTF8;
	CUTF16*				pcUTF16;
	bool				bResult;
	void*				pvData;
	size				uiLength;
	EUnicodeEncoding	eEncoding;
	CUnicodeReader*		pcReader;

	mcFile.Init();
	bResult = mcFile.Read(szFilename);
	if (!bResult)
	{
		mcFile.Kill();
		return NULL;
	}

	pvData = mcFile.Get();
	uiLength = (size)mcFile.Length();

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
				mcFile.Kill();
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
			mcFile.Kill();
			return NULL;
		}
		pcReader = pcUTF16;
	}
	else
	{
		mcFile.Kill();
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
				cTextBuilder.Kill();
				SafeKill(pcUnicodeReader);
				return NULL;
			}

			if (cCodePoint32 == UNICODE_LINE_FEED)
			{
				bResult = cTextBuilder.PushNewLine();
				if (!bResult)
				{
					cTextBuilder.Kill();
					SafeKill(pcUnicodeReader);
					return NULL;
				}
			}
			else if (cCodePoint32 == UNICODE_CARRIAGE_RETURN)
			{
				// Do nothing.
			}
			else
			{
				bResult = cTextBuilder.Push(cCodePoint32);
				if (!bResult)
				{
					cTextBuilder.Kill();
					SafeKill(pcUnicodeReader);
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
				cTextBuilder.Kill();
				SafeKill(pcUnicodeReader);
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

	SafeKill(pcUnicodeReader);
	cTextBuilder.Kill();

	return pText;
}

