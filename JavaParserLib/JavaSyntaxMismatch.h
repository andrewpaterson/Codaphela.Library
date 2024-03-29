#ifndef __JAVA_SYNTAX_MISMATCH_H__
#define __JAVA_SYNTAX_MISMATCH_H__
#include "BaseLib/Chars.h"
#include "JavaSyntax.h"
#include "JavaSyntaxImport.h"
#include "JavaSyntaxPackage.h"
#include "JavaSyntaxClassCommon.h"


class CJavaSyntaxMismatch : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxMismatch);
public:
	char*	GetType(void) override;
	void	TypePrint(CChars* pszDest, int iDepth) override;
	void	PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;
	bool	IsMismatch(void) override;
};


#endif // !__JAVA_SYNTAX_MISMATCH_H__



