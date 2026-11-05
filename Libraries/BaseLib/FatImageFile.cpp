#include "DiskFile.h"
#include "MemoryDrive.h"
#include "FatVolume.h"
#include "FatFile.h"
#include "FatImageFile.h"


//////////////////////////////////////////////////////////////////////////
//
// False if the image can't be read or written, has no FAT volume, or the
// file can't be written on it (the image is then left as it was).
//
//////////////////////////////////////////////////////////////////////////
bool FatPutFileOnImage(const char* szImage, const char* szName, void* pvData, size uiSize)
{
	CDiskFile		cImage;
	CMemoryDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	filePos			iSize;
	bool			bResult;

	cImage.Init(szImage);
	if (!cImage.Open(EFM_Read))
	{
		cImage.Kill();
		return false;
	}
	iSize = cImage.Size();
	bResult = (iSize > 0) && cDrive.Init((uint64)iSize, 512);
	bResult = bResult && (cImage.Read(cDrive.GetMemory(), (lsize)iSize, 1) == 1);
	cImage.Close();

	bResult = bResult && (cVolume.Mount(&cDrive) == FAT_SUCCESS);
	if (bResult)
	{
		cFile.Init(&cVolume);
		bResult = cFile.Open(szName, FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE) == FAT_SUCCESS;
		bResult = bResult && (cFile.Write((uint8*)pvData, uiSize) == FAT_SUCCESS);
		bResult &= cFile.Close() == FAT_SUCCESS;
		bResult &= cVolume.Unmount() == FAT_SUCCESS;
	}

	if (bResult)
	{
		bResult = cImage.Open(EFM_Write_Create);
		bResult = bResult && (cImage.Write(cDrive.GetMemory(), (lsize)iSize, 1) == 1);
		bResult &= cImage.Close();
	}
	cDrive.Kill();
	cImage.Kill();
	return bResult;
}

