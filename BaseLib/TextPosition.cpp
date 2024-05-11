#include "Define.h"
#include "TextPosition.h"
#include "Chars.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextPosition::Init(void)
{
	Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextPosition::Init(char* szPos, size iLine, size iColumn)
{
	this->szPos = szPos;
	this->iLine = iLine;
	this->iColumn = iColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextPosition::Init(char* szStartOfText, size iLength, char* szPosition)
{
	CChars	sz;
	char*	szLineStart;
	size	iLineNumber;
	size	iColumnNumber;
		
	sz.Fake(szStartOfText, 0, iLength);
	szLineStart = sz.FindLineContaining(szPosition, &iLineNumber);
	
	iColumnNumber = szPosition - szLineStart;
	Init(szPosition, iLineNumber, iColumnNumber);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextPosition::Clear(void)
{
	szPos = NULL;
	iLine = 0;
	iColumn = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::Init(CExternalString* pcText, STextPosition* pcTextPosition, char* szFilename)
{
	mcTextPosition = *pcTextPosition;
	mcText = *pcText;
	mszFilename = szFilename;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::Kill(void)
{
	mcTextPosition.Clear();
	mcText.Clear();
	mszFilename = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::PrintPosition(CChars* pszDest)
{
	CChars	szParserText;
	size	iPreviousLineEnd;
	size	iLineEnd;
	size	iUsedLength;

	szParserText.Fake(mcText.msz, 0, mcText.miLen - 1);

	iUsedLength = UsedLength();
	if ((iUsedLength <= mcText.miLen) && (iUsedLength >= 0))
	{
		iPreviousLineEnd = szParserText.FindFromEnd(iUsedLength - 1, "\n");
		iLineEnd = szParserText.Find(iUsedLength, "\n");

		if (iPreviousLineEnd == -1 && iLineEnd == -1)
		{
			PrintPositionSingleLineParser(pszDest);
		}
		else
		{
			PrintPositionMultilineParser(pszDest);
		}
	}
	else
	{
		iLineEnd = szParserText.Find(0, "\n");
		if (iLineEnd == -1)
		{
			iLineEnd = mcText.miLen;
		}
		pszDest->Append(mcText.msz, iLineEnd);
		pszDest->Append(" [^ OUT OF BOUNDS]");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::PrintPositionSingleLineParser(CChars* pszDest)
{
	size		iBefore;

	pszDest->Append(&mcText);
	iBefore = mcTextPosition.szPos - mcText.msz;
	pszDest->AppendNewLine();
	pszDest->Append(' ', iBefore);
	pszDest->Append('^');
	pszDest->AppendNewLine();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::PrintPositionMultilineParser(CChars* pszDest)
{
	CChars				szParserLine;
	CExternalString		pac;
	CChars				szParserText;
	size				iPreviousLineEnd;
	size				iLineEnd;
	size				iParserPos;
	CChars				szLine;
	size				iUsedLength;

	szParserText.Fake(mcText.msz, 0, mcText.miLen - 1);

	iUsedLength = UsedLength();
	iPreviousLineEnd = szParserText.FindFromEnd(iUsedLength - 1, "\n");

	iLineEnd = szParserText.Find(iUsedLength, "\n");

	if (iPreviousLineEnd == -1)
	{
		iPreviousLineEnd = 0;
	}
	if (iLineEnd == -1)
	{
		iLineEnd = mcText.miLen;
	}

	pac.Init(&mcText.msz[iPreviousLineEnd], &mcText.msz[iLineEnd]);

	iParserPos = mcTextPosition.szPos - mcText.msz;

	szParserLine.Init(&pac);
	szParserLine.Replace("\n", " ");
	szParserLine.Replace("\r", " ");

	szLine.Init();
	szLine.Append('[');
	if (mszFilename)
	{
		szLine.Append(mszFilename);
		szLine.Append(':');
	}
	szLine.Append(mcTextPosition.iLine + 1);
	szLine.Append("]: ");

	pszDest->Append(szLine);
	pszDest->Append(szParserLine);
	pszDest->AppendNewLine();
	pszDest->Append(' ', szLine.Length());
	pszDest->Append(' ', iParserPos - iPreviousLineEnd);
	pszDest->Append('^');
	pszDest->AppendNewLine();

	szLine.Kill();
	szParserLine.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CTextPositionPrinter::UsedLength(void)
{
	return (size)(mcTextPosition.szPos - mcText.msz);
}

