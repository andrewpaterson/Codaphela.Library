#include "TextBuilder.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextBuilder::Init(CUnicodeReader* pcUTF, CText* pcText, CFont* pcFont)
{
	mpcUTF = pcUTF;
	mpcText = pcText;
	mpcFont = pcFont;
	mpcCommon = NULL;
	mpcRun = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CTextBuilder::Kill(void)
{
	if (mpcCommon)
	{
		PushCurrent();
	}
	mpcUTF = NULL;
	mpcText = NULL;
	mpcCommon = NULL;
	mpcRun = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextBuilder::Push(uint32 cCodePoint32)
{
	size	uiUTF16Length;
	uint32	uiUTF16Char;
	bool	bResult;

	uiUTF16Char = mpcUTF->GetUTF16ElementUint32(cCodePoint32);
	uiUTF16Length = mpcUTF->GetUTF16Length(uiUTF16Char);

	if (mpcUTF->IsError(uiUTF16Length))
	{
		return false;
	}

	if (mpcCommon)
	{
		if (uiUTF16Length == 1)
		{
			if (mpcCommon->IsUTF16Short())
			{
				((CTextBuilderUTF16Short*)mpcCommon)->Push((uint16)uiUTF16Char);
				return true;
			}
		}
		else if (uiUTF16Length == 2)
		{
			if (mpcCommon->IsUTF16Long())
			{
				((CTextBuilderUTF16Long*)mpcCommon)->Push(uiUTF16Char);
				return true;
			}
		}
		else
		{
			return false;
		}

		bResult = PushCurrent();
		if (!bResult)
		{
			return false;
		}
	}

	if (uiUTF16Length == 1)
	{
		mcShort.Init();
		mpcCommon = &mcShort;
		((CTextBuilderUTF16Short*)mpcCommon)->Push((uint16)uiUTF16Char);
		return true;
	}
	else if (uiUTF16Length == 2)
	{
		mcLong.Init();
		mpcCommon = &mcLong;
		((CTextBuilderUTF16Long*)mpcCommon)->Push(uiUTF16Char);
		return true;
	}
	else
	{
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextBuilder::PushCurrent(void)
{
	CTextBuilderUTF16Short*		pauiShort;
	CTextBuilderUTF16Long*		pauiLong;
	CTextUTF16Short*			pcShortRun;
	CTextUTF16Long*				pcLongRun;
	bool						bResult;

	if (mpcCommon->IsUTF16Short())
	{
		pauiShort = (CTextBuilderUTF16Short*)mpcCommon;
		pcShortRun = mpcText->AllocateUTF16Short(pauiShort->NumElements(), pauiShort->GetChars());
		pauiShort->Kill();

		bResult = EnsureRun();
		if ((!bResult) || !pcShortRun)
		{
			return false;
		}
		mpcRun->Add((CTextElement*)pcShortRun);

		mpcCommon = NULL;
		return pcShortRun != NULL;
	}
	else if (mpcCommon->IsUTF16Long())
	{
		pauiLong = (CTextBuilderUTF16Long*)mpcCommon;
		pcLongRun = mpcText->AllocateUTF16Long(pauiLong->NumElements(), pauiLong->GetChars());
		pauiLong->Kill();

		bResult = EnsureRun();
		if ((!bResult) || !pcLongRun)
		{
			return false;
		}
		mpcRun->Add((CTextElement*)pcLongRun);

		mpcCommon = NULL;
		return pcLongRun != NULL;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextBuilder::Push(uint8* puiCodePoint, size uiCodePointsLength)
{
	uint32				cCodePoint32;
	size				uiCodePointLength;
	uint32				cUTF16Element32;
	size				uiUTF16Length;
	size				uiNumWritten;
	CArrayUint16		auiUTF16;
	bool				bResult;
	CTextUTF16Multi*	pcUTFMulti;

	if (mpcCommon)
	{
		bResult = PushCurrent();
		if (!bResult)
		{
			return false;
		}
	}

	auiUTF16.Init();
	uiNumWritten = 0;
	for (;;)
	{
		uiCodePointLength = StrLen((char*)puiCodePoint);
		cCodePoint32 = mpcUTF->MakeCodePointUint32FromBuffer(puiCodePoint, uiCodePointLength);
		uiUTF16Length = mpcUTF->GetUTF16Length(cCodePoint32);
		if (mpcUTF->IsError(uiUTF16Length))
		{
			auiUTF16.Kill();
			return false;
		}

		cUTF16Element32 = mpcUTF->GetUTF16ElementUint32(cCodePoint32);
		if (mpcUTF->IsError(cUTF16Element32))
		{
			auiUTF16.Kill();
			return false;
		}

		if (uiUTF16Length == 1)
		{
			uiNumWritten++;
			auiUTF16.Push((uint16)cUTF16Element32);
		}
		else
		{
			uiNumWritten += uiUTF16Length;
			auiUTF16.InsertBlockAfterEnd((uint16*)&cUTF16Element32, uiUTF16Length);
		}

		puiCodePoint = (uint8*)NextString((char*)puiCodePoint);

		if (uiCodePointLength + 1 >= uiCodePointsLength)
		{
			break;
		}
		uiUTF16Length = mpcUTF->GetZWJLength(UE_UTF16LE) / sizeof(uint16);
		uiNumWritten += uiUTF16Length;
		auiUTF16.InsertBlockAfterEnd((uint16*)mpcUTF->GetZWJBytes(UE_UTF16LE), uiUTF16Length);
		uiCodePointsLength -= (uiCodePointLength + 1);
	}


	pcUTFMulti = mpcText->AllocateUTF16Multi(auiUTF16.ByteSize(), auiUTF16.GetData());
	auiUTF16.Kill();

	bResult = EnsureRun();
	if ((!bResult) || !pcUTFMulti)
	{
		return false;
	}
	mpcRun->Add((CTextElement*)pcUTFMulti);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextBuilder::PushNewLine(void)
{
	CTextNewLine*	pcNewLine;
	bool			bResult;

	if (mpcCommon)
	{
		bResult = PushCurrent();
		if (!bResult)
		{
			return false;
		}
	}

	pcNewLine = mpcText->AllocateNewLine();
	bResult = EnsureRun();
	if ((!bResult) || !pcNewLine)
	{
		return false;
	}

	mpcRun->Add((CTextElement*)pcNewLine);
	mpcRun->Done();
	mpcRun = NULL;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextBuilder::Done(void)
{
	bool bResult;

	if (mpcCommon)
	{
		bResult = PushCurrent();
		if (!bResult)
		{
			return false;
		}

		if (mpcRun)
		{
			mpcRun->Done();
			mpcRun = NULL;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CTextBuilder::EnsureRun(void)
{
	if (mpcRun == NULL)
	{
		mpcRun = mpcText->AllocateRun(mpcFont);

		return mpcRun != NULL;
	}
	return true;
}

