#ifndef __JAVA_SYNTAX_TYPE_GENERIC_H__
#define __JAVA_SYNTAX_TYPE_GENERIC_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaSyntaxTypeCommon.h"


class CJavaSyntaxGeneric : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxGeneric);
protected:
	CTypeCommonArray	mapcGenerics;

public:
	void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 				Kill(void);

	char*				GetType(void) override;
	void				TypePrint(CChars* pszDest, int iDepth) override;
	void				PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	BOOL				IsGeneric(void) override;

	void				AddType(CJavaSyntaxTypeCommon* pcType);
	CTypeCommonArray*	GetGenerics(void);
};


#endif // !__JAVA_SYNTAX_TYPE_GENERIC_H__

