#include "Validation.h"
#include "PackFiles.h"
#include "DiskFile.h"
#include "FileUtil.h"
#include "IntegerHelper.h"
#include "Logger.h"
#include "PackFilePacker.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFilePacker::Pack(char* szDestPakFile, char* szSourceDirectory, char* szPackDirectory)
{
	CPackFiles	cPackFiles;
	CDiskFile*	pcDiskFile;
	CChars		szPackFileName;
	CFileUtil	cFileUtil;
	BOOL		bResult;

	szPackFileName.Init(szDestPakFile);
	cFileUtil.FullPath(&szPackFileName);

	cFileUtil.Delete(szPackFileName.Text());

	pcDiskFile = DiskFile(szPackFileName.Text());

	szPackFileName.Kill();

	cPackFiles.Init(pcDiskFile, PFM_Write);
	bResult = cPackFiles.AddDirectory(szSourceDirectory, szPackDirectory);
	cPackFiles.Kill();

	bResult = FixBool(bResult);
	if (!bResult)
	{
		gcLogger.Error("Failed to pack.");
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFilePacker::Unpack(char* szSourcePakFile, char* szDestDirectory, BOOL bRemoveDestDir)
{
	CPackFiles	cPackFiles;
	CDiskFile*	pcDiskFile;
	CFileUtil	cFileUtil;
	BOOL		bResult;
	CChars		szPackFileName;

	szPackFileName.Init(szSourcePakFile);
	cFileUtil.FullPath(&szPackFileName);
	pcDiskFile = DiskFile(szPackFileName.Text());
	szPackFileName.Kill();

	if (cFileUtil.Exists(pcDiskFile->mszFileName.Text()))
	{
		if (bRemoveDestDir)
		{
			cFileUtil.RemoveDir(szDestDirectory);
		}
	
		cPackFiles.Init(pcDiskFile, PFM_Read);
		bResult = cPackFiles.Unpack(szDestDirectory);
		cPackFiles.Kill();

		if (!bResult)
		{
			gcLogger.Error2("Failed to unpack [", szSourcePakFile, "] into [", StringToString(szDestDirectory), "].", NULL);
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		gcLogger.Error2("Pak file [", StringToString(szSourcePakFile), "]does not exist.", NULL);
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFilePacker::List(CPackFiles* pcPackFiles, CChars* pszDest)
{
	CArrayPackFileNodePtrs	acFileNodes;
	CFileNodePackFileNode*	pcFileNode;
	int						i;
	CChars					szName;
	CChars					szSize;
	CPackFileNode*			pcPackFileNode;
	filePos					iSize;

	acFileNodes.Init();
	pcPackFiles->GetFiles(&acFileNodes);

	for (i = 0; i < acFileNodes.NumElements(); i++)
	{
		pcFileNode = *acFileNodes.Get(i);
		pcPackFileNode = pcFileNode->File();

		iSize = pcPackFileNode->Size()/1024;
		if (iSize == 0)
		{
			iSize = 1;
		}

		szName.Init();

		pcFileNode->GetFullName(&szName);
		szSize.Init();
		szSize.Append(iSize);
		szSize.RightAlign(' ', 10);
		szSize.Append("KB  ");
		szName.Insert(0, &szSize);
		szSize.Kill();

		szName.AppendNewLine();

		pszDest->Append(szName);
		szName.Kill();
	}

	acFileNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFilePacker::List(char* szSourcePakFile, CChars* pszDest)
{
	CPackFiles	cPackFiles;
	CDiskFile*	pcDiskFile;
	CChars		szPackFileName;
	CFileUtil	cFileUtil;

	szPackFileName.Init(szSourcePakFile);
	cFileUtil.FullPath(&szPackFileName);
	pcDiskFile = DiskFile(szPackFileName.Text());
	szPackFileName.Kill();

	if (cFileUtil.Exists(pcDiskFile->mszFileName.Text()))
	{
		cPackFiles.Init(pcDiskFile, PFM_Read);
		List(&cPackFiles, pszDest);
		cPackFiles.Kill();
		return TRUE;
	}
	else
	{
		gcLogger.Error("File does not exist.");
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL PackDirectory(char* szDestPakFile, char* szSourceDirectory, char* szPackDirectory)
{
	CPackFilePacker	cPacker;

	return cPacker.Pack(szDestPakFile, szSourceDirectory, szPackDirectory);
}

