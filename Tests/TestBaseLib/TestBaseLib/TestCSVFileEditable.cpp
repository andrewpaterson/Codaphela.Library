#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/CSVFileEditable.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCSVFileEditableRead(void)
{
	CCSVFileEditable	cCSVFile;
	CMemoryFile*		pcMemoryFile;
	size					iNumRows;
	SCSVRowEditable*	psRow;
	CChars*				pszCell;
	size					iFileLen;
	char				szFile[] = "\
Header1|Header2\n\
234|7782\n";

	iFileLen = strlen(szFile);
	pcMemoryFile = MemoryFile(szFile, iFileLen);

	cCSVFile.Init('|');
	cCSVFile.Open(pcMemoryFile);

	cCSVFile.ReadAllLines();

	iNumRows = cCSVFile.NumRows();
	AssertSize(2, iNumRows);
	psRow = cCSVFile.Get(0);
	AssertSize(2, psRow->NumCells());

	pszCell = psRow->Get(0);
	AssertString("Header1", pszCell->Text());
	pszCell = psRow->Get(1);
	AssertString("Header2", pszCell->Text());

	psRow = cCSVFile.Get(1);
	AssertSize(2, psRow->NumCells());

	pszCell = psRow->Get(0);
	AssertString("234", pszCell->Text());
	pszCell = psRow->Get(1);
	AssertString("7782", pszCell->Text());

	cCSVFile.Close();
	cCSVFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCSVFileEditable(void)
{
	BeginTests();

	TestCSVFileEditableRead();

	TestStatistics();
}

