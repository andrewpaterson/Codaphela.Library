#ifndef __TEXT_POSITION_H__
#define __TEXT_POSITION_H__
#include "Define.h"
#include "PrimitiveTypes.h"
#include "ExternalString.h"


struct STextPosition
{
	char*	szPos;
	size	iLine;
	size	iColumn;

	void	Init(void);
	void	Init(char* szPos, size iLine, size iColumn);
	void	Init(char* szStartOfText, size iLength, char* szPosition);
	void	Clear(void);
};


class CChars;
class CTextPositionPrinter
{
protected:
	STextPosition		mcTextPosition;
	CExternalString		mcText;
	char*				mszFilename;

public:
	void	Init(CExternalString* pcText, STextPosition* pcTextPosition, char* szFilename = NULL);
	void	Kill(void);

	void	PrintPosition(CChars* pszDest);

protected:
	void	PrintPositionSingleLineParser(CChars* pszDest);
	void	PrintPositionMultilineParser(CChars* pszDest);
	size	UsedLength(void);
};


#endif // __TEXT_POSITION_H__

