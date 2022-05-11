#ifndef __JAVA_SYNTAX_ERROR_H__
#define __JAVA_SYNTAX_ERROR_H__
#include "BaseLib/Chars.h"
#include "JavaSyntax.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxError : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxError);
public:
	char*	GetType(void) override;
	BOOL	IsError(void) override;
};


#endif // !__JAVA_SYNTAX_ERROR_H__

