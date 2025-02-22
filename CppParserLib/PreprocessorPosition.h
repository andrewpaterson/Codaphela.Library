#ifndef __PREPROCESSOR_POSITION_H__
#define __PREPROCESSOR_POSITION_H__


class CPreprocessorTokenParser;
class CSourceFile;
class CChars;
struct SPreprocessorPosition
{
	size	miLine;
	size	miColumn;
	char*	mszShortName;

	void	Init(CPreprocessorTokenParser* pcParser, char* szShortName);
	void	Init(size iLine, size iColumn, char* szShortName);

	CChars* Message(CChars* pszUninitialised);
};


#endif // !__PREPROCESSOR_POSITION_H__

