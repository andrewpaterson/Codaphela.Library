#ifndef __JAVA_SYNTAX_MISMATCH_H__
#define __JAVA_SYNTAX_MISMATCH_H__
#include "BaseLib/Chars.h"
#include "JavaSyntax.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxTopLevel.h"


class CJavaSyntaxMismatch : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxMismatch);
public:
	char*	GetType(void) override;
	BOOL	IsMismatch(void) override;
};


#endif // !__JAVA_SYNTAX_MISMATCH_H__



