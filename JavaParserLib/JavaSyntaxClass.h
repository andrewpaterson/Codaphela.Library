#ifndef __JAVA_SYNTAX_CLASS_H__
#define __JAVA_SYNTAX_CLASS_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxType.h"
#include "JavaSyntaxClassCommon.h"
#include "JavaSyntaxClassBlock.h"


class CJavaSyntaxClass : public CJavaSyntaxClassCommon
{
CONSTRUCTABLE(CJavaSyntaxClass);
protected:
	CJavaSyntaxType*		mpcType;
	CJavaSyntaxClassBlock*	mpcBlock;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*	GetType(void) override;
	void	Print(CChars* pszDest, int iDepth);

	BOOL	IsClass(void) override;

	void	SetSyntaxType(CJavaSyntaxType* pcType);
	void	SetBlock(CJavaSyntaxClassBlock* pcBlock);
};


#endif // !__JAVA_SYNTAX_CLASS_H__

