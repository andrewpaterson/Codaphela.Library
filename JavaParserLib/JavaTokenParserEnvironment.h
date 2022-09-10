#ifndef __JAVA_TOKEN_PARSER_ENVIRONMENT_H__
#define __JAVA_TOKEN_PARSER_ENVIRONMENT_H__
#include "BaseLib/MemoryFile.h"
#include "JavaTokenParser.h"


class CTokenParserEnvironment
{
protected:
	CJavaTokenParser		mcTokenParser;
	CJavaTokenMemory		mcTokenMemory;
	CJavaTokenDefinitions	mcTokenDefinitions;
	CLogger					mcLogger;
	CMemoryFile				mcMemoryLog;

public:
	void				Init(char* szFilename, char* szText, bool bBreakOnError);
	void				Init(char* szFilename, char* szText, int iTextLen, bool bBreakOnError);
	void				Kill(void);
	
	bool				Parse(bool bFailOnError = true);
	CJavaTokenParser*	GetParser(void);
	char*				GetOutput(int* piLength = NULL);
	char*				GetOutput(CChars* pszDest);

	void				DumpLog(void);
};


#endif // !__JAVA_TOKEN_PARSER_ENVIRONMENT_H__

