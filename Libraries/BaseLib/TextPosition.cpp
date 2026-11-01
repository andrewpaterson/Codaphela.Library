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
void STextPosition::Init(const char* szPos, lsize iLine, lsize iColumn)
{
	this->szPos = szPos;
	this->iLine = iLine;
	this->iColumn = iColumn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void STextPosition::Init(const char* szStartOfText, lsize iLength, const char* szPosition)
{
	const char*	szLineStart;
	const char*	szCurrent;
	const char*	szEnd;
	lsize		iLineNumber;

	iLineNumber = 0;
	szLineStart = szStartOfText;
	szEnd = &szStartOfText[iLength];
	for (szCurrent = szStartOfText; szCurrent < szEnd; szCurrent++)
	{
		if (*szCurrent == '\n')
		{
			szLineStart = szCurrent + 1;
		}

		if (szCurrent == szPosition)
		{
			break;
		}

		if (*szCurrent == '\n')
		{
			iLineNumber++;
		}
	}

	Init(szPosition, iLineNumber, (lsize)(szPosition - szLineStart));
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
void CTextPositionPrinter::Init(CExternalString* pcText, STextPosition* pcTextPosition, const char* szFilename)
{
	Init(pcText->msz, pcText->miLen, pcTextPosition, szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::Init(const char* szText, lsize iTextLength, STextPosition* pcTextPosition, const char* szFilename)
{
	mcTextPosition = *pcTextPosition;
	mszText = szText;
	miTextLength = iTextLength;
	mszFilename = szFilename;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::Kill(void)
{
	mcTextPosition.Clear();
	mszText = NULL;
	miTextLength = 0;
	mszFilename = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::PrintPosition(CChars* pszDest)
{
	lsize	iSearchLength;
	lsize	iPreviousLineEnd;
	lsize	iLineEnd;
	lsize	iUsedLength;

	iSearchLength = SearchLength();

	iUsedLength = UsedLength();
	if (iUsedLength <= miTextLength)
	{
		iPreviousLineEnd = FindLineFeedBefore(mszText, iSearchLength, iUsedLength);
		iLineEnd = FindLineFeedFrom(mszText, iSearchLength, iUsedLength);

		if (iPreviousLineEnd == ARRAY_LARGE_ELEMENT_NOT_FOUND && iLineEnd == ARRAY_LARGE_ELEMENT_NOT_FOUND)
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
		iLineEnd = FindLineFeedFrom(mszText, iSearchLength, 0);
		if (iLineEnd == ARRAY_LARGE_ELEMENT_NOT_FOUND)
		{
			iLineEnd = miTextLength;
		}
		pszDest->Append(mszText, (size)iLineEnd);
		pszDest->Append(" [^ OUT OF BOUNDS]");
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextPositionPrinter::PrintPositionSingleLineParser(CChars* pszDest)
{
	lsize		iBefore;

	pszDest->Append(mszText, (size)miTextLength);
	iBefore = UsedLength();
	pszDest->AppendNewLine();
	pszDest->Append(' ', (size)iBefore);
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
	lsize				iSearchLength;
	lsize				iPreviousLineEnd;
	lsize				iLineEnd;
	lsize				iParserPos;
	CChars				szLine;
	lsize				iUsedLength;

	iSearchLength = SearchLength();

	iUsedLength = UsedLength();
	iPreviousLineEnd = FindLineFeedBefore(mszText, iSearchLength, iUsedLength);

	iLineEnd = FindLineFeedFrom(mszText, iSearchLength, iUsedLength);

	if (iPreviousLineEnd == ARRAY_LARGE_ELEMENT_NOT_FOUND)
	{
		iPreviousLineEnd = 0;
	}
	if (iLineEnd == ARRAY_LARGE_ELEMENT_NOT_FOUND)
	{
		iLineEnd = miTextLength;
	}

	iParserPos = iUsedLength;

	szParserLine.Init(&mszText[iPreviousLineEnd], 0, (size)(iLineEnd + 1 - iPreviousLineEnd));
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
	pszDest->Append(' ', (size)(iParserPos - iPreviousLineEnd));
	pszDest->Append('^');
	pszDest->AppendNewLine();

	szLine.Kill();
	szParserLine.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CTextPositionPrinter::UsedLength(void)
{
	return (lsize)(mcTextPosition.szPos - mszText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CTextPositionPrinter::SearchLength(void)
{
	//Line feeds are only looked for before the last character.
	return (miTextLength > 0) ? miTextLength - 1 : 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize FindLineFeedBefore(const char* szText, lsize iSearchLength, lsize iIndex)
{
	lsize	i;

	if ((iIndex == 0) || (iIndex - 1 > iSearchLength))
	{
		return ARRAY_LARGE_ELEMENT_NOT_FOUND;
	}

	i = (iIndex < iSearchLength) ? iIndex : iSearchLength;
	while (i > 0)
	{
		i--;
		if (szText[i] == '\n')
		{
			return i;
		}
	}
	return ARRAY_LARGE_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize FindLineFeedFrom(const char* szText, lsize iSearchLength, lsize iIndex)
{
	lsize	i;

	for (i = iIndex; i < iSearchLength; i++)
	{
		if (szText[i] == '\n')
		{
			return i;
		}
	}
	return ARRAY_LARGE_ELEMENT_NOT_FOUND;
}

