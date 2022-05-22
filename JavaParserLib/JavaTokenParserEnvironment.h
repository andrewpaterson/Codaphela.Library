#ifndef __JAVA_TOKEN_PARSER_ENVIRONMENT_H__
#define __JAVA_TOKEN_PARSER_ENVIRONMENT_H__
#include "JavaTokenParser.h"


class CTokenParserEnvironment
{
protected:
	CJavaTokenParser		mcTokenParser;
	CJavaTokenMemory		mcTokenMemory;
	CJavaTokenDefinitions	mcTokenDefinitions;
	CLogger					mcLogger;

public:
	void				Init(char* szFilename, char* szText, BOOL bBreakOnError);
	void				Init(char* szFilename, char* szText, int iTextLen, BOOL bBreakOnError);
	void				Kill(void);

	BOOL				Parse(BOOL bFailOnError = TRUE);
	CJavaTokenParser*	GetParser(void);
};


#endif // !__JAVA_TOKEN_PARSER_ENVIRONMENT_H__

