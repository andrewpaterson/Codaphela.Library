#include <string.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/MemoryDrive.h"
#include "BaseLib/Fat32.h"
#include "BaseLib/FatImageFile.h"
#include "TestLib/Assert.h"


#define TEST_FAT_IMAGE_FILE		"Output" _FS_ "FatImage.img"
#define TEST_FAT_IMAGE_SECTORS	16384		//8MB, FAT16 with 1KB clusters.


//////////////////////////////////////////////////////////////////////////
//
// The bytes of szName on the image file's volume, which must be uiSize long.
//
//////////////////////////////////////////////////////////////////////////
static void TestFatImageFileRead(const char* szName, uint8* puiExpected, uint32 uiSize)
{
	CDiskFile		cImage;
	CMemoryDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8*			puiRead;
	lsize			uiRead;

	cImage.Init(TEST_FAT_IMAGE_FILE);
	AssertTrue(cImage.Open(EFM_Read));
	AssertLong((int64)TEST_FAT_IMAGE_SECTORS * 512, cImage.Size());
	AssertTrue(cDrive.Init((uint64)TEST_FAT_IMAGE_SECTORS * 512, 512));
	AssertTrue(cImage.Read(cDrive.GetMemory(), (lsize)TEST_FAT_IMAGE_SECTORS * 512, 1) == 1);
	cImage.Close();
	cImage.Kill();

	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open(szName, FAT_FILE_ACCESS_READ));
	puiRead = (uint8*)malloc(uiSize + 16);
	uiRead = 0;
	AssertInt(FAT_SUCCESS, cFile.Read(puiRead, uiSize + 16, &uiRead));
	AssertLong(uiSize, (int64)uiRead);
	AssertTrue(memcmp(puiRead, puiExpected, uiSize) == 0);
	free(puiRead);
	AssertInt(FAT_SUCCESS, cFile.Close());
	AssertInt(FAT_SUCCESS, cVolume.Unmount());
	cDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A file put on a formatted image is there when it is mounted again, and
// putting it again replaces it.  An image that isn't there, or has no
// volume, isn't changed.
//
//////////////////////////////////////////////////////////////////////////
void TestFatImageFilePut(void)
{
	CFileUtil		cFileUtil;
	CMemoryDrive	cDrive;
	CDiskFile		cImage;
	uint8			auiFirst[3000];
	uint8			auiSecond[700];
	uint32			i;

	for (i = 0; i < sizeof(auiFirst); i++)
	{
		auiFirst[i] = (uint8)(i * 7 + 1);
	}
	for (i = 0; i < sizeof(auiSecond); i++)
	{
		auiSecond[i] = (uint8)(i * 13 + 5);
	}

	cFileUtil.MakeDir("Output");
	AssertTrue(cDrive.Init((uint64)TEST_FAT_IMAGE_SECTORS * 512, 512));
	AssertInt(FAT_SUCCESS, FatFormat(FAT_FS_TYPE_FAT16, "Image", 2, &cDrive));
	cImage.Init(TEST_FAT_IMAGE_FILE);
	AssertTrue(cImage.Open(EFM_Write_Create));
	AssertTrue(cImage.Write(cDrive.GetMemory(), (lsize)TEST_FAT_IMAGE_SECTORS * 512, 1) == 1);
	AssertTrue(cImage.Close());
	cImage.Kill();
	cDrive.Kill();

	AssertTrue(FatPutFileOnImage(TEST_FAT_IMAGE_FILE, "\\boot.exe", auiFirst, sizeof(auiFirst)));
	TestFatImageFileRead("\\boot.exe", auiFirst, sizeof(auiFirst));
	AssertTrue(FatPutFileOnImage(TEST_FAT_IMAGE_FILE, "\\boot.exe", auiSecond, sizeof(auiSecond)));
	TestFatImageFileRead("\\boot.exe", auiSecond, sizeof(auiSecond));

	AssertFalse(FatPutFileOnImage("Output" _FS_ "Missing.img", "\\boot.exe", auiFirst, sizeof(auiFirst)));
	cImage.Init("Output" _FS_ "NotFat.img");
	AssertTrue(cImage.Open(EFM_Write_Create));
	AssertTrue(cImage.Write(auiFirst, sizeof(auiFirst), 1) == 1);
	AssertTrue(cImage.Close());
	cImage.Kill();
	AssertFalse(FatPutFileOnImage("Output" _FS_ "NotFat.img", "\\boot.exe", auiSecond, sizeof(auiSecond)));
}


//////////////////////////////////////////////////////////////////////////
//
// A file written on a FAT16 volume is there once it is unmounted and
// mounted again: Unmount writes the cached root directory sector.
//
//////////////////////////////////////////////////////////////////////////
void TestFatImageFileRemount(void)
{
	CMemoryDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	uint8			auiData[100];

	memset(auiData, 0x5A, sizeof(auiData));
	cDrive.Init((uint64)TEST_FAT_IMAGE_SECTORS * 512, 512);
	AssertInt(FAT_SUCCESS, FatFormat(FAT_FS_TYPE_FAT16, "Image", 2, &cDrive));
	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\boot.exe", FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE));
	AssertInt(FAT_SUCCESS, cFile.Write(auiData, sizeof(auiData)));
	AssertInt(FAT_SUCCESS, cFile.Close());
	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\boot.exe", FAT_FILE_ACCESS_READ));
	AssertInt(FAT_SUCCESS, cFile.Close());
	AssertInt(FAT_SUCCESS, cVolume.Unmount());
	AssertInt(FAT_SUCCESS, cVolume.Mount(&cDrive));
	cFile.Init(&cVolume);
	AssertInt(FAT_SUCCESS, cFile.Open("\\boot.exe", FAT_FILE_ACCESS_READ));
	AssertInt(FAT_SUCCESS, cFile.Close());
	AssertInt(FAT_SUCCESS, cVolume.Unmount());
	cDrive.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestFatImageFile(void)
{
	BeginTests();

	TestFatImageFileRemount();
	TestFatImageFilePut();

	TestStatistics();
}

