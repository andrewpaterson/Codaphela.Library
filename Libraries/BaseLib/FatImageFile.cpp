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
		bResult = cImage.Open(EFM_Write_Replace);
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



#define FAT_IMAGE_SECTOR_SIZE	512


//A disk image file as a drive that can only be read, a sector at a time from the file.
class CFatImageDrive : public CFileDrive
{
protected:
	CDiskFile*	mpcImage;

public:
	void	Init(CDiskFile* pcImage, uint64 uiSize)
	{
		CFileDrive::Init(FAT_IMAGE_SECTOR_SIZE);
		mpcImage = pcImage;
		muiMaxSector = uiSize / FAT_IMAGE_SECTOR_SIZE;
	}

	void	Kill(void)
	{
		mpcImage = NULL;
	}

	bool	Read(uint64 uiSector, void* pvData)
	{
		return Read(uiSector, 1, pvData);
	}

	bool	Read(uint64 uiSector, uint32 uiNumSectors, void* pvData)
	{
		if (uiSector + uiNumSectors > muiMaxSector)
		{
			return false;
		}
		return mpcImage->Seek((filePos)(uiSector * FAT_IMAGE_SECTOR_SIZE), EFSO_SET) && (mpcImage->Read(pvData, (lsize)uiNumSectors * FAT_IMAGE_SECTOR_SIZE, 1) == 1);
	}

	bool	Write(uint64 uiSector, void* pvData)								{ return false; }
	bool	Write(uint64 uiSector, uint32 uiNumSectors, void* pvData)			{ return false; }
	bool	Erase(uint64 uiStartSector, uint64 uiStopSectorInclusive)		{ return false; }
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool FatGetFileFromImage(const char* szImage, const char* szName, void** ppvData, size* puiSize)
{
	CDiskFile		cImage;
	CFatImageDrive	cDrive;
	CFatVolume		cVolume;
	CFatFile		cFile;
	filePos			iSize;
	lsize			uiRead;
	uint8*			puiData;
	bool			bResult;

	*ppvData = NULL;
	*puiSize = 0;
	cImage.Init(szImage);
	if (!cImage.Open(EFM_Read))
	{
		cImage.Kill();
		return false;
	}
	iSize = cImage.Size();
	cDrive.Init(&cImage, (iSize > 0) ? (uint64)iSize : 0);
	puiData = NULL;
	bResult = (iSize > 0) && (cVolume.Mount(&cDrive) == FAT_SUCCESS);
	if (bResult)
	{
		cFile.Init(&cVolume);
		bResult = cFile.Open(szName, FAT_FILE_ACCESS_READ) == FAT_SUCCESS;
		if (bResult)
		{
			*puiSize = cFile.Size();
			puiData = (uint8*)malloc((*puiSize > 0) ? *puiSize : 1);
			uiRead = 0;
			bResult = (*puiSize == 0) || ((cFile.Read(puiData, *puiSize, &uiRead) == FAT_SUCCESS) && (uiRead == *puiSize));
		}

		//Whether or not it opened: Open makes the file's cluster cache before it looks for the file, and Close is what
		//frees it.
		cFile.Close();
	}
	cVolume.Unmount();
	cDrive.Kill();
	cImage.Close();
	cImage.Kill();
	if (!bResult)
	{
		free(puiData);
		*puiSize = 0;
		return false;
	}
	*ppvData = puiData;
	return true;
}
