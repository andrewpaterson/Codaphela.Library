#ifndef __PREPROCESSOR_POSITION_H__
#define __PREPROCESSOR_POSITION_H__


class CPreprocessorTokenParser;
class CSourceFile;
class CChars;
struct SPreprocessorPosition
{
	int		miLine;
	int		miColumn;
	char*	mszShortName;

	void	Init(CPreprocessorTokenParser* pcParser, CSourceFile* pcFile);
	void	Init(int iLine, int iColumn, char* mszShortName);

	CChars* Message(CChars* pszUninitialised);
};


#endif // !__PREPROCESSOR_POSITION_H__

