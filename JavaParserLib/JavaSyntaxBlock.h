#ifndef __JAVA_SYNTAX_BLOCK_H__
#define __JAVA_SYNTAX_BLOCK_H__
#include "JavaSyntax.h"


class CJavaSyntaxBlock : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxBlock);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsBlock(void) override;
};


#endif // !__JAVA_SYNTAX_BLOCK_H__

