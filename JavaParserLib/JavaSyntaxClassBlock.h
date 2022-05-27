#ifndef __JAVA_SYNTAX_CLASS_BLOCK_H__
#define __JAVA_SYNTAX_CLASS_BLOCK_H__
#include "JavaSyntax.h"

// { Statement ; ... Statement ; }

// Variable Declaration
// Code Block
// Constructor Declaration
// Method Declaration
// Class
// Enum
// Interface

class CJavaSyntaxClassBlock : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxClassBlock);
protected:

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsClassBlock(void) override;
};


#endif // !__JAVA_SYNTAX_CLASS_BLOCK_H__
