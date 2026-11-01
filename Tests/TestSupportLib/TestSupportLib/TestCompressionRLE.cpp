#include <string.h>
#include "BaseLib/FileBasic.h"
#include "BaseLib/MemoryFile.h"
#include "SupportLib/RLEPacketEncoder.h"
#include "SupportLib/RLEPacketDecoder.h"
#include "SupportLib/RLEBitmapDecoder.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void OpenMemoryFile(CFileBasic* pcFile, uint8* puiData, size uiDataSize)
{
	pcFile->Init(MemoryFile(puiData, uiDataSize));
	AssertTrue(pcFile->Open(EFM_Read));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CloseMemoryFile(CFileBasic* pcFile)
{
	pcFile->Close();
	pcFile->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEPacketEncoderSingleBytes(void)
{
	CRLEPacketEncoder	cEncoder;
	uint8				auiDest[300];
	uint8				auiSource[200];
	size				uiSize;
	int					i;

	cEncoder.Init(1);

	//A single run.
	uint8 auiRun[] = { 5, 5, 5, 5 };
	uint8 auiRunExpected[] = { 0x83, 5 };
	uiSize = cEncoder.Encode(auiRun, 4, auiDest);
	AssertSize(2, uiSize);
	AssertMemory(auiRunExpected, auiDest, uiSize);

	//A single raw packet.
	uint8 auiRaw[] = { 1, 2, 3, 4 };
	uint8 auiRawExpected[] = { 0x03, 1, 2, 3, 4 };
	uiSize = cEncoder.Encode(auiRaw, 4, auiDest);
	AssertSize(5, uiSize);
	AssertMemory(auiRawExpected, auiDest, uiSize);

	//One element is a raw packet.
	uint8 auiSingle[] = { 9 };
	uint8 auiSingleExpected[] = { 0x00, 9 };
	uiSize = cEncoder.Encode(auiSingle, 1, auiDest);
	AssertSize(2, uiSize);
	AssertMemory(auiSingleExpected, auiDest, uiSize);

	//A raw packet ends where a run of two starts.
	uint8 auiMixed[] = { 1, 2, 2, 3 };
	uint8 auiMixedExpected[] = { 0x00, 1, 0x81, 2, 0x00, 3 };
	uiSize = cEncoder.Encode(auiMixed, 4, auiDest);
	AssertSize(6, uiSize);
	AssertMemory(auiMixedExpected, auiDest, uiSize);

	//Runs longer than 128 are split.
	memset(auiSource, 7, 200);
	uint8 auiLongRunExpected[] = { 0xFF, 7, 0xC7, 7 };
	uiSize = cEncoder.Encode(auiSource, 200, auiDest);
	AssertSize(4, uiSize);
	AssertMemory(auiLongRunExpected, auiDest, uiSize);

	//Raw packets longer than 128 are split.
	for (i = 0; i < 130; i++)
	{
		auiSource[i] = (uint8)i;
	}
	uiSize = cEncoder.Encode(auiSource, 130, auiDest);
	AssertSize(132, uiSize);
	AssertInt(0x7F, auiDest[0]);
	AssertMemory(auiSource, &auiDest[1], 128);
	AssertInt(0x01, auiDest[129]);
	AssertInt(128, auiDest[130]);
	AssertInt(129, auiDest[131]);

	cEncoder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEPacketEncoderMultipleBytes(void)
{
	CRLEPacketEncoder	cEncoder;
	uint8				auiDest[64];
	size				uiSize;

	cEncoder.Init(3);

	uint8 auiPixels[] = { 1, 2, 3,  1, 2, 3,  4, 5, 6 };
	uint8 auiExpected[] = { 0x81, 1, 2, 3,  0x00, 4, 5, 6 };
	uiSize = cEncoder.Encode(auiPixels, 3, auiDest);
	AssertSize(8, uiSize);
	AssertMemory(auiExpected, auiDest, uiSize);

	//Elements that only partly match are not a run.
	uint8 auiPartial[] = { 1, 2, 3,  1, 2, 4 };
	uint8 auiPartialExpected[] = { 0x01, 1, 2, 3, 1, 2, 4 };
	uiSize = cEncoder.Encode(auiPartial, 2, auiDest);
	AssertSize(7, uiSize);
	AssertMemory(auiPartialExpected, auiDest, uiSize);

	//The worst case is a packet header for every element.
	AssertSize(40, cEncoder.GetMaxEncodedSize(10));

	cEncoder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEPacketDecoder(void)
{
	CFileBasic			cFile;
	CRLEPacketDecoder	cDecoder;
	bool				bRepeat;
	uint8				uiElement;
	uint8				auiElement[2];

	uint8 auiData[] = { 0x82, 9, 0x01, 1, 2 };
	OpenMemoryFile(&cFile, auiData, sizeof(auiData));
	cDecoder.Init(&cFile);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertTrue(cFile.ReadInt8(&uiElement));
	AssertInt(9, uiElement);

	//The rest of the run is repeated without reading.
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertTrue(bRepeat);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertTrue(bRepeat);
	AssertLong(2, cFile.GetFilePos());

	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertTrue(cFile.ReadInt8(&uiElement));
	AssertInt(1, uiElement);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertTrue(cFile.ReadInt8(&uiElement));
	AssertInt(2, uiElement);
	AssertLong(5, cFile.GetFilePos());
	AssertFalse(cDecoder.Next(&bRepeat));
	cDecoder.Kill();
	CloseMemoryFile(&cFile);

	uint8 auiPixels[] = { 0x81, 0xA, 0xB,  0x00, 0xC, 0xD };
	uint8 auiFirst[] = { 0xA, 0xB };
	uint8 auiSecond[] = { 0xC, 0xD };
	OpenMemoryFile(&cFile, auiPixels, sizeof(auiPixels));
	cDecoder.Init(&cFile);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertTrue(cFile.ReadData(auiElement, 2));
	AssertMemory(auiFirst, auiElement, 2);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertTrue(bRepeat);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertTrue(cFile.ReadData(auiElement, 2));
	AssertMemory(auiSecond, auiElement, 2);
	AssertFalse(cDecoder.Next(&bRepeat));
	cDecoder.Kill();
	CloseMemoryFile(&cFile);
}


//////////////////////////////////////////////////////////////////////////
//
// Truncated data fails rather than reading past the end.
//
//////////////////////////////////////////////////////////////////////////
void TestRLEPacketDecoderTruncated(void)
{
	CFileBasic			cFile;
	CRLEPacketDecoder	cDecoder;
	bool				bRepeat;
	uint8				uiElement;
	uint8				auiElement[2];

	//No packet header.
	uint8 auiEmpty[] = { 0 };
	OpenMemoryFile(&cFile, auiEmpty, 0);
	cDecoder.Init(&cFile);
	AssertFalse(cDecoder.Next(&bRepeat));
	cDecoder.Kill();
	CloseMemoryFile(&cFile);

	//A raw packet of three elements with only two present.
	uint8 auiRaw[] = { 0x02, 1, 2 };
	OpenMemoryFile(&cFile, auiRaw, sizeof(auiRaw));
	cDecoder.Init(&cFile);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertTrue(cFile.ReadInt8(&uiElement));
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertTrue(cFile.ReadInt8(&uiElement));
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertFalse(cFile.ReadInt8(&uiElement));
	cDecoder.Kill();
	CloseMemoryFile(&cFile);

	//A run header with only part of a two byte element.
	uint8 auiPartial[] = { 0x80, 1 };
	OpenMemoryFile(&cFile, auiPartial, sizeof(auiPartial));
	cDecoder.Init(&cFile);
	AssertTrue(cDecoder.Next(&bRepeat));
	AssertFalse(bRepeat);
	AssertFalse(cFile.ReadData(auiElement, 2));
	cDecoder.Kill();
	CloseMemoryFile(&cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEPacketRoundTrip(void)
{
	CRLEPacketEncoder	cEncoder;
	CRLEPacketDecoder	cDecoder;
	CFileBasic			cFile;
	uint8				auiSource[300 * 2];
	uint8				auiEncoded[300 * 3];
	uint8				auiElement[2];
	size				uiSize;
	bool				bRepeat;
	int					i;
	bool				bSame;

	//Runs of varying lengths, including some longer than a packet, between raw elements.
	for (i = 0; i < 300; i++)
	{
		auiSource[i * 2] = (uint8)((i < 150) ? (i / 7) : i);
		auiSource[i * 2 + 1] = (uint8)((i % 50 == 0) ? i : 0);
	}

	cEncoder.Init(2);
	uiSize = cEncoder.Encode(auiSource, 300, auiEncoded);
	AssertTrue(uiSize <= cEncoder.GetMaxEncodedSize(300));
	cEncoder.Kill();

	bSame = true;
	OpenMemoryFile(&cFile, auiEncoded, uiSize);
	cDecoder.Init(&cFile);
	for (i = 0; i < 300; i++)
	{
		if (!cDecoder.Next(&bRepeat) || (!bRepeat && !cFile.ReadData(auiElement, 2)) || (memcmp(auiElement, &auiSource[i * 2], 2) != 0))
		{
			bSame = false;
			break;
		}
	}
	AssertTrue(bSame);
	AssertLong((filePos)uiSize, cFile.GetFilePos());
	AssertFalse(cDecoder.Next(&bRepeat));
	cDecoder.Kill();
	CloseMemoryFile(&cFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool DecodeBitmapRLE(CRLEBitmapDecoder* pcDecoder, uint8* puiData, size uiDataSize, uint8* puiIndices)
{
	CFileBasic	cFile;
	bool		bResult;

	OpenMemoryFile(&cFile, puiData, uiDataSize);
	bResult = pcDecoder->Decode(&cFile, puiIndices);
	CloseMemoryFile(&cFile);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEBitmapDecoderRLE8(void)
{
	CRLEBitmapDecoder	cDecoder;
	uint8				auiIndices[4 * 3];
	bool				bResult;

	uint8 auiData[] = {
		0x03, 0x01,							//Row 0: three pixels of index 1.
		0x00, 0x00,							//End of line.
		0x00, 0x02, 0x01, 0x00,				//Row 1: delta one pixel right.
		0x02, 0x07,							//Two pixels of index 7.
		0x00, 0x00,							//End of line.
		0x00, 0x03, 0x04, 0x05, 0x06, 0x00,	//Row 2: absolute run of three pixels, padded.
		0x05, 0x09,							//Five pixels of index 9, clipped to one.
		0x00, 0x01,							//End of bitmap.
		0x02, 0x08 };						//Ignored.

	uint8 auiExpected[] = {
		0x01, 0x01, 0x01, 0xEE,
		0xEE, 0x07, 0x07, 0xEE,
		0x04, 0x05, 0x06, 0x09 };

	memset(auiIndices, 0xEE, sizeof(auiIndices));
	cDecoder.Init(4, 3, false);
	bResult = DecodeBitmapRLE(&cDecoder, auiData, sizeof(auiData), auiIndices);
	AssertTrue(bResult);
	AssertMemory(auiExpected, auiIndices, sizeof(auiIndices));
	cDecoder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEBitmapDecoderRLE4(void)
{
	CRLEBitmapDecoder	cDecoder;
	uint8				auiIndices[8];
	bool				bResult;

	uint8 auiData[] = {
		0x03, 0xAB,							//Three pixels alternating index 0xA and 0xB.
		0x00, 0x05, 0xCD, 0xEF, 0x10, 0x00,	//Absolute run of five pixels in three bytes, padded.
		0x00, 0x01 };						//End of bitmap.

	uint8 auiExpected[] = { 0x0A, 0x0B, 0x0A, 0x0C, 0x0D, 0x0E, 0x0F, 0x01 };

	memset(auiIndices, 0xEE, sizeof(auiIndices));
	cDecoder.Init(8, 1, true);
	bResult = DecodeBitmapRLE(&cDecoder, auiData, sizeof(auiData), auiIndices);
	AssertTrue(bResult);
	AssertMemory(auiExpected, auiIndices, sizeof(auiIndices));
	cDecoder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestRLEBitmapDecoderEdges(void)
{
	CRLEBitmapDecoder	cDecoder;
	uint8				auiIndices[4];
	bool				bResult;

	cDecoder.Init(4, 1, false);

	//An absolute run past the end of the data fails.
	uint8 auiAbsolute[] = { 0x00, 0x04, 0x01, 0x02 };
	bResult = DecodeBitmapRLE(&cDecoder, auiAbsolute, sizeof(auiAbsolute), auiIndices);
	AssertFalse(bResult);

	//A delta past the end of the data fails.
	uint8 auiDelta[] = { 0x00, 0x02, 0x01 };
	bResult = DecodeBitmapRLE(&cDecoder, auiDelta, sizeof(auiDelta), auiIndices);
	AssertFalse(bResult);

	//A missing end of bitmap is allowed.
	uint8 auiNoEnd[] = { 0x02, 0x05 };
	uint8 auiNoEndExpected[] = { 0x05, 0x05, 0xEE, 0xEE };
	memset(auiIndices, 0xEE, sizeof(auiIndices));
	bResult = DecodeBitmapRLE(&cDecoder, auiNoEnd, sizeof(auiNoEnd), auiIndices);
	AssertTrue(bResult);
	AssertMemory(auiNoEndExpected, auiIndices, sizeof(auiIndices));

	//Pixels below the last row are ignored.
	uint8 auiBelow[] = { 0x00, 0x00, 0x02, 0x03, 0x00, 0x03, 0x04, 0x05, 0x06, 0x00 };
	uint8 auiBelowExpected[] = { 0xEE, 0xEE, 0xEE, 0xEE };
	memset(auiIndices, 0xEE, sizeof(auiIndices));
	bResult = DecodeBitmapRLE(&cDecoder, auiBelow, sizeof(auiBelow), auiIndices);
	AssertTrue(bResult);
	AssertMemory(auiBelowExpected, auiIndices, sizeof(auiIndices));

	cDecoder.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCompressionRLE(void)
{
	BeginTests();

	TestRLEPacketEncoderSingleBytes();
	TestRLEPacketEncoderMultipleBytes();
	TestRLEPacketDecoder();
	TestRLEPacketDecoderTruncated();
	TestRLEPacketRoundTrip();
	TestRLEBitmapDecoderRLE8();
	TestRLEBitmapDecoderRLE4();
	TestRLEBitmapDecoderEdges();

	TestStatistics();
}
