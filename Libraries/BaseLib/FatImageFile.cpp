#include <stdlib.h>
#include <string.h>
#include "DiskFile.h"
#include "FileUtil.h"
#include "MemoryDrive.h"
#include "FatVolume.h"
#include "FatFile.h"
#include "FatImageFile.h"


//////////////////////////////////////////////////////////////////////////
//
// Each folder szName (\Games\Mod\song.mod) is in is made if it isn't
// there: \Games, then \Games\Mod.  False if one can't be (a file has
// its name).
//
//////////////////////////////////////////////////////////////////////////
static bool FatMakeFolders(CFatVolume* pcVolume, const char* szName)
{
	char		szFolder[FAT_MAX_PATH + 1];
	size		uiLength;
	size		i;
	EFatCode	eResult;

	uiLength = strlen(szName);
	if (uiLength > FAT_MAX_PATH)
	{
		return false;
	}
	for (i = 1; i < uiLength; i++)
	{
		if ((szName[i] != '\\') && (szName[i] != '/'))
		{
			continue;
		}
		memcpy(szFolder, szName, i);
		szFolder[i] = '\0';
		eResult = pcVolume->CreateDirectory(szFolder);
		if ((eResult != FAT_SUCCESS) && (eResult != FAT_FILENAME_ALREADY_EXISTS))
		{
			return false;
		}
	}
	return true;
}


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

	if (bResult)
	{
		bResult = cVolume.Mount(&cDrive) == FAT_SUCCESS;
		if (bResult)
		{
			cFile.Init(&cVolume);
			bResult = FatMakeFolders(&cVolume, szName);
			bResult = bResult && cFile.Open(szName, FAT_FILE_ACCESS_CREATE | FAT_FILE_ACCESS_OVERWRITE | FAT_FILE_ACCESS_WRITE) == FAT_SUCCESS;
			bResult = bResult && (cFile.Write((uint8*)pvData, uiSize) == FAT_SUCCESS);
			bResult &= cFile.Close() == FAT_SUCCESS;
		}

		//Even after a mount that failed: Unmount frees the sector cache Mount made (and writes nothing then).
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


//////////////////////////////////////////////////////////////////////////
//
// False if szFile can't be read, or as FatPutFileOnImage.
//
//////////////////////////////////////////////////////////////////////////
bool FatPutDiskFileOnImage(const char* szImage, const char* szName, const char* szFile)
{
	CFileUtil	cFileUtil;
	size		uiSize;
	void*		pvData;
	bool		bResult;

	pvData = cFileUtil.ReadAll(szFile, &uiSize);
	if (pvData == NULL)
	{
		return false;
	}
	bResult = FatPutFileOnImage(szImage, szName, pvData, uiSize);
	free(pvData);
	return bResult;
}

