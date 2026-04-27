#include "StringHelper.h"
#include "UnicodeReader.h"
#include "UnicodeWriter.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnicodeWriter::WriteUTF16LE(CUnicodeReader* pcUnicodeReader, CArrayUint16* puiUTF16Dest, size uiMaxToWrite)
{
	uint32				cCodePoint32;
	uint16				cUTF16Element16;
	uint32				cUTF16Element32;
	size				uiUTFLength;
	size				uiUTF16Length;
	uint8				auiBuffer[0x13];
	uint8*				puiCodePoint;
	size				uiCodePointsLength;
	size				uiCodePointLength;
	size				uiNumWritten;
	
	uiNumWritten = 0;
	uiUTFLength = pcUnicodeReader->PeekUTFBytes();
	while ((uiUTFLength != 0) && (uiNumWritten < uiMaxToWrite))
	{
		if (uiUTFLength <= 2)
		{
			cCodePoint32 = pcUnicodeReader->GetCodePointUint32();
			uiUTF16Length = pcUnicodeReader->GetUTF16Length(cCodePoint32);
			if (uiUTF16Length <= 2)
			{
				cUTF16Element16 = pcUnicodeReader->GetUTF16ElementUint16(cCodePoint32);
				if (pcUnicodeReader->IsTooSmallOrError(cUTF16Element16))
				{
					return ARRAY_ELEMENT_NOT_FOUND;
				}

				if (uiUTF16Length == 1)
				{
					uiNumWritten++;
					puiUTF16Dest->Push(cUTF16Element16);
				}
				else
				{
					uiNumWritten += uiUTF16Length;
					puiUTF16Dest->InsertBlockAfterEnd(&cUTF16Element16, uiUTF16Length);
				}
			}
			else if (uiUTF16Length <= 4)
			{
				cUTF16Element32 = pcUnicodeReader->GetUTF16ElementUint32(cCodePoint32);
				if (pcUnicodeReader->IsTooSmallOrError(cUTF16Element32))
				{
					return ARRAY_ELEMENT_NOT_FOUND;
				}

				if (uiUTF16Length == 1)
				{
					uiNumWritten++;
					puiUTF16Dest->Push((uint16)cUTF16Element32);
				}
				else
				{
					uiNumWritten += uiUTF16Length;
					puiUTF16Dest->InsertBlockAfterEnd((uint16*)&cUTF16Element32, uiUTF16Length);
				}
			}
		}
		else
		{
			uiCodePointsLength = pcUnicodeReader->GetCodePointMulti(auiBuffer, 0x13);
			puiCodePoint = auiBuffer;
			for (;;)
			{
				uiCodePointLength = StrLen((char*)puiCodePoint);
				cCodePoint32 = pcUnicodeReader->MakeCodePointUint32FromBuffer(puiCodePoint, uiCodePointLength);
				uiUTF16Length = pcUnicodeReader->GetUTF16Length(cCodePoint32);
				cUTF16Element32 = pcUnicodeReader->GetUTF16ElementUint32(cCodePoint32);

				if (uiUTF16Length == 1)
				{
					uiNumWritten++;
					puiUTF16Dest->Push((uint16)cUTF16Element32);
				}
				else
				{
					uiNumWritten += uiUTF16Length;
					puiUTF16Dest->InsertBlockAfterEnd((uint16*)&cUTF16Element32, uiUTF16Length);
				}

				puiCodePoint = (uint8*)NextString((char*)puiCodePoint);

				if (uiCodePointLength + 1 >= uiCodePointsLength)
				{
					break;
				}
				uiUTF16Length = pcUnicodeReader->GetZWJLength(UE_UTF16LE) / sizeof(uint16);
				uiNumWritten += uiUTF16Length;
				puiUTF16Dest->InsertBlockAfterEnd((uint16*)pcUnicodeReader->GetZWJBytes(UE_UTF16LE), uiUTF16Length);
				uiCodePointsLength -= (uiCodePointLength + 1);
			}
		}

		uiUTFLength = pcUnicodeReader->PeekUTFBytes();
	}

	return uiNumWritten;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnicodeWriter::WriteUTF16BE(CUnicodeReader* pcUnicodeReader, CArrayUint16* puiUTF16Dest, size uiMaxToWrite)
{
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CUnicodeWriter::WriteUTF8(CUnicodeReader* pcUnicodeReader, CArrayUint8* puiUTF8Dest, size uiMaxToWrite)
{
	uint32				cCodePoint32;
	uint16				cUTF8Element16;
	uint32				cUTF8Element32;
	size				uiUTFLength;
	size				uiUTF8Length;
	uint8				auiBuffer[0x13];
	uint8*				puiCodePoint;
	size				uiCodePointsLength;
	size				uiCodePointLength;
	CArrayUint8			auiUTF8Dest;
	size				uiNumWritten;

	uiNumWritten = 0;
	uiUTFLength = pcUnicodeReader->PeekUTFBytes();
	while ((uiUTFLength != 0) && (uiNumWritten < uiMaxToWrite))
	{
		if (uiUTFLength <= 2)
		{
			cCodePoint32 = pcUnicodeReader->GetCodePointUint32();
			uiUTF8Length = pcUnicodeReader->GetUTF8Length(cCodePoint32);
			if (uiUTF8Length <= 2)
			{
				cUTF8Element16 = pcUnicodeReader->GetUTF8ElementUint16(cCodePoint32);
				if (pcUnicodeReader->IsTooSmallOrError(cUTF8Element16))
				{
					return ARRAY_ELEMENT_NOT_FOUND;
				}

				if (uiUTF8Length == 1)
				{
					uiNumWritten++;
					auiUTF8Dest.Push((uint8)cUTF8Element16);
				}
				else
				{
					uiNumWritten += uiUTF8Length;
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element16, uiUTF8Length);
				}
			}
			else if (uiUTF8Length <= 4)
			{
				cUTF8Element32 = pcUnicodeReader->GetUTF8ElementUint32(cCodePoint32);
				if (pcUnicodeReader->IsTooSmallOrError(cUTF8Element32))
				{
					return ARRAY_ELEMENT_NOT_FOUND;
				}

				if (uiUTF8Length == 1)
				{
					uiNumWritten++;
					auiUTF8Dest.Push((uint8)cUTF8Element32);
				}
				else
				{
					uiNumWritten += uiUTF8Length;
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element32, uiUTF8Length);
				}
			}
		}
		else
		{
			uiCodePointsLength = pcUnicodeReader->GetCodePointMulti(auiBuffer, 0x13);
			puiCodePoint = auiBuffer;
			for (;;)
			{
				uiCodePointLength = StrLen((char*)puiCodePoint);
				cCodePoint32 = pcUnicodeReader->MakeCodePointUint32FromBuffer(puiCodePoint, uiCodePointLength);
				uiUTF8Length = pcUnicodeReader->GetUTF8Length(cCodePoint32);
				cUTF8Element32 = pcUnicodeReader->GetUTF8ElementUint32(cCodePoint32);

				if (uiUTF8Length == 1)
				{
					uiNumWritten++;
					auiUTF8Dest.Push((uint8)cUTF8Element32);
				}
				else
				{
					uiNumWritten += uiUTF8Length;
					auiUTF8Dest.InsertBlockAfterEnd((uint8*)&cUTF8Element32, uiUTF8Length);
				}

				puiCodePoint = (uint8*)NextString((char*)puiCodePoint);

				if (uiCodePointLength + 1 >= uiCodePointsLength)
				{
					break;
				}

				uiUTF8Length = pcUnicodeReader->GetZWJLength(UE_UTF8);
				uiNumWritten += uiUTF8Length;
				auiUTF8Dest.InsertBlockAfterEnd(pcUnicodeReader->GetZWJBytes(UE_UTF8), uiUTF8Length);
				uiCodePointsLength -= (uiCodePointLength + 1);
			}
		}

		uiUTFLength = pcUnicodeReader->PeekUTFBytes();
	}
	return uiNumWritten;
}

