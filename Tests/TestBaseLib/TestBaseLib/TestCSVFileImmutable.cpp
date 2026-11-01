#include "BaseLib/FileUtil.h"
#include "BaseLib/MemoryFile.h"
#include "BaseLib/CSVFileImutable.h"
#include "BaseLib/CSVFile.h"
#include "TestLib/Assert.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCSVFileImmutableRead(void)
{
	CCSVFileImmutable	cCSVFile;
	CMemoryFile*		pcMemoryFile;
	size					iNumRows;
	SCSVRowImmutable*	psRow;
	char*				szCell;
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
	AssertSize(2, psRow->NumElements());

	szCell = psRow->Get(0);
	AssertString("Header1", szCell);
	szCell = psRow->Get(1);
	AssertString("Header2", szCell);

	psRow = cCSVFile.Get(1);
	AssertSize(2, psRow->NumElements());

	szCell = psRow->Get(0);
	AssertString("234", szCell);
	szCell = psRow->Get(1);
	AssertString("7782", szCell);

	cCSVFile.Close();
	cCSVFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A line much longer than the 256 byte chunks lines are read in (and than
// the stack buffer there used to be) is read whole.
//
//////////////////////////////////////////////////////////////////////////
void TestCSVFileImmutableLongLine(void)
{
	CCSVFileImmutable	cCSVFile;
	CMemoryFile*		pcMemoryFile;
	SCSVRowImmutable*	psRow;
	CChars				szFile;
	size				i;

	szFile.Init("Short|Line\n");
	for (i = 0; i < 2000; i++)
	{
		if (i != 0)
		{
			szFile.Append('|');
		}
		szFile.Append("Cell");
		szFile.Append((int32)(10000 + i));
	}
	szFile.Append("\nLast|Line\n");
	AssertSize(11 + 2000 * 10 - 1 + 1 + 10, szFile.Length());

	pcMemoryFile = MemoryFile(szFile.Text(), szFile.Length());
	cCSVFile.Init('|');
	cCSVFile.Open(pcMemoryFile);
	cCSVFile.ReadAllLines();

	AssertSize(3, cCSVFile.NumRows());
	psRow = cCSVFile.Get(1);
	AssertSize(2000, psRow->NumElements());
	AssertString("Cell10000", psRow->Get(0));
	AssertString("Cell11024", psRow->Get(1024));
	AssertString("Cell11999", psRow->Get(1999));
	psRow = cCSVFile.Get(2);
	AssertSize(2, psRow->NumElements());
	AssertString("Last", psRow->Get(0));
	AssertString("Line", psRow->Get(1));

	cCSVFile.Close();
	cCSVFile.Kill();
	szFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// ReadLine into a buffer cuts a longer line short (and terminates it), and
// the next line is still the next one read.
//
//////////////////////////////////////////////////////////////////////////
void TestCSVFileReadLineMaxLength(void)
{
	CCSVFile		cCSVFile;
	CMemoryFile*	pcMemoryFile;
	char			szLine[8];
	char			szFile[] = "Header1|Header2\n234|7782\n";

	pcMemoryFile = MemoryFile(szFile, strlen(szFile));
	cCSVFile.Init('|');
	cCSVFile.Open(pcMemoryFile);

	memset(szLine, 'x', 8);
	AssertTrue(cCSVFile.ReadLine(szLine, 8));
	AssertString("Header1", szLine);
	AssertTrue(cCSVFile.ReadLine(szLine, 8));
	AssertString("234|778", szLine);
	AssertFalse(cCSVFile.ReadLine(szLine, 8));

	cCSVFile.Close();
	cCSVFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void TestCSVFileImmutable(void)
{
	BeginTests();

	TestCSVFileImmutableRead();
	TestCSVFileImmutableLongLine();
	TestCSVFileReadLineMaxLength();

	TestStatistics();
}

