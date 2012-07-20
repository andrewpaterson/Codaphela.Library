#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "NamedIndexesFileDumper.h"


///////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL DumpNamedIndexesFile(CChars* pszDest, char* szFileName, int iDataWidth, int iNumPerBlock)
{
	CFileBasic					cFile;
	int							iFileSize;
	void*						pvMem;
	void*						pvAllocated;
	int							iDataCount;
	int							i;
	unsigned long long int		id;
	char*						szName;
	CChars						szAligned;
	CChars						szTemp;

	cFile.Init(DiskFile(szFileName));
	cFile.Open(EFM_Read);

	iFileSize = (int)cFile.GetFileSize();
	if (iFileSize % iDataWidth != 0)
	{
		return FALSE;
	}

	pvMem = malloc(iFileSize);
	pvAllocated = pvMem;
	cFile.Read(pvMem, iFileSize, 1);

	szTemp.Init(szFileName);
	pszDest->Append(szTemp);
	pszDest->AppendNewLine();
	pszDest->Append('-', szTemp.Length());
	pszDest->AppendNewLine();

	iDataCount = iFileSize/iDataWidth;
	for (i = 0; i < iDataCount; i++)
	{
		id = *((unsigned long long int*)pvMem);
		pvMem = RemapSinglePointer(pvMem, sizeof(unsigned long long int));
		
		szName = (char*)pvMem;
		pvMem = RemapSinglePointer(pvMem, iDataWidth - sizeof(unsigned long long int));

		szAligned.Init();
		szAligned.Append(i);
		szAligned.RightAlign(' ', 4);
		pszDest->Append(szAligned);
		szAligned.Kill();

		pszDest->Append(": ");

		szAligned.Init();
		szAligned.Append(id);
		szAligned.RightAlign(' ', 6);
		pszDest->Append(szAligned);
		szAligned.Kill();

		pszDest->Append(" -> ");
		pszDest->Append(szName);
		pszDest->AppendNewLine();

		if (i % iNumPerBlock == (iNumPerBlock-1))
		{
			pszDest->AppendNewLine();
		}
	}

	if (pszDest->EndsWith("\n\n"))
	{
		pszDest->RemoveFromEnd(1);
	}

	pszDest->Append('-', szTemp.Length());
	pszDest->AppendNewLine();

	szTemp.Kill();

	free(pvAllocated);
	cFile.Close();
	cFile.Kill();
	return TRUE;
}

