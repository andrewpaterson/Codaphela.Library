#ifndef __TEXT_POSITION_H__
#define __TEXT_POSITION_H__
#include "Define.h"
#include "PrimitiveTypes.h"
#include "ExternalString.h"


struct STextPosition
{
	const char*	szPos;
	lsize	iLine;
	lsize	iColumn;

	void	Init(void);
	void	Init(const char* szPos, lsize iLine, lsize iColumn);
	void	Init(const char* szStartOfText, lsize iLength, const char* szPosition);
	void	Clear(void);
};


class CChars;
class CTextPositionPrinter
{
protected:
	STextPosition		mcTextPosition;
	const char*			mszText;
	lsize				miTextLength;
	const char*			mszFilename;

public:
	void	Init(CExternalString* pcText, STextPosition* pcTextPosition, const char* szFilename = NULL);
	void	Init(const char* szText, lsize iTextLength, STextPosition* pcTextPosition, const char* szFilename = NULL);
	void	Kill(void);

	void	PrintPosition(CChars* pszDest);

protected:
	void	PrintPositionSingleLineParser(CChars* pszDest);
	void	PrintPositionMultilineParser(CChars* pszDest);
	lsize	UsedLength(void);
	lsize	SearchLength(void);
};


lsize FindLineFeedBefore(const char* szText, lsize iSearchLength, lsize iIndex);
lsize FindLineFeedFrom(const char* szText, lsize iSearchLength, lsize iIndex);


#endif // __TEXT_POSITION_H__

