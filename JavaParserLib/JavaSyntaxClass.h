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
	void 					Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 					Kill(void);

	char*					GetType(void) override;
	void					TypePrint(CChars* pszDest, int iDepth) override;
	void					PrettyPrint(CChars* pszDest, int iBlockDepth) override;
	BOOL					IsClass(void) override;
	BOOL					IsCompoundStatement(void);

	void					SetSyntaxType(CJavaSyntaxType* pcType);
	void					SetBlock(CJavaSyntaxClassBlock* pcBlock);

	CJavaSyntaxType*		GetSyntaxType(void);
	CJavaSyntaxClassBlock*	GetBlock(void);
};


#endif // !__JAVA_SYNTAX_CLASS_H__

