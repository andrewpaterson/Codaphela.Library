#ifndef __JAVA_SYNTAX_ERROR_H__
#define __JAVA_SYNTAX_ERROR_H__
#include "BaseLib/Chars.h"
#include "JavaSyntax.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxClassCommon.h"


class CJavaSyntaxError : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxError);
public:
	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);
	BOOL	IsError(void) override;
};


#endif // !__JAVA_SYNTAX_ERROR_H__

