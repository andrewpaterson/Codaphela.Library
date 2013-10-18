#include "Chars.h"
#include "FileUtil.h"
#include "StringHelper.h"
#include "LogString.h"


#define LOG_TO_STRING_SCRATCH_PAD_SIZE	32
#define LOG_TO_STRING_MAX_LENGTH		8192


char gaszLogToStringScratchPad[LOG_TO_STRING_SCRATCH_PAD_SIZE][LOG_TO_STRING_MAX_LENGTH];
int  giLogToStringCount = 0;


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int IncrementLogToStringCount(void)
{
	int	iCount = giLogToStringCount;

	giLogToStringCount++;
	if (giLogToStringCount >= LOG_TO_STRING_SCRATCH_PAD_SIZE)
	{
		giLogToStringCount = 0;
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IntToString(int i)
{
	int iCount = IncrementLogToStringCount();

	IToA(i, gaszLogToStringScratchPad[iCount], 10);
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* LongLongToString(long long int lli)
{
	int iCount = IncrementLogToStringCount();

	IToA(lli, gaszLogToStringScratchPad[iCount], 10);
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* PointerToString(void* pv)
{
	int iCount = IncrementLogToStringCount();
	
	CChars	sz;

	sz.Init();
	sz.AppendHexHiLo(pv, sizeof(size_t));
	//IToA((size_t)pv, gaszLogToStringScratchPad[iCount], 16);
	strcpy(gaszLogToStringScratchPad[iCount], sz.Text());
	sz.Kill();
	return gaszLogToStringScratchPad[iCount];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* MethodToString(char* szFile, int iLine, char* szFunction)
{
	int		iCount = IncrementLogToStringCount();
	char*	sz = gaszLogToStringScratchPad[iCount];

	CChars		szOutput;
	CFileUtil	cFileUtil;

	szOutput.Init();
	szOutput.Append(szFile);
	cFileUtil.RemovePath(&szOutput);
	szOutput.Insert(0, '(');
	szOutput.Append(':');
	szOutput.Append(iLine);
	szOutput.Append(") ");
	szOutput.Append(szFunction);

	StrCpySafe(sz, szOutput.Text(), LOG_TO_STRING_MAX_LENGTH);
	szOutput.Kill();

	return sz;
}

