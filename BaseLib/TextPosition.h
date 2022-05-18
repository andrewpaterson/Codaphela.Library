#ifndef __TEXT_POSITION_H__
#define __TEXT_POSITION_H__
#include "Define.h"
#include "ExternalString.h"


struct STextPosition
{
	char*	szPos;
	int		iLine;
	int		iColumn;

	void	Init(void);
	void	Init(char* szPos, int iLine, int iColumn);
	void	Init(char* szStartOfText, int iLength, char* szPosition);
	void	Clear(void);
};


class CChars;
class CTextPositionPrinter
{
protected:
	STextPosition		mcTextPosition;
	CExternalString		mcText;
	char*				mszFileName;

public:
	void	Init(CExternalString* pcText, STextPosition* pcTextPosition, char* szFileName = NULL);
	void	Kill(void);

	void	PrintPosition(CChars* pszDest);

protected:
	void	PrintPositionSingleLineParser(CChars* pszDest);
	void	PrintPositionMultilineParser(CChars* pszDest);
	int		UsedLength(void);
};


#endif // !__TEXT_POSITION_H__

