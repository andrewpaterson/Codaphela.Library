#ifndef __JAVA_SYNTAX_TYPE_H__
#define __JAVA_SYNTAX_TYPE_H__
#include "JavaSyntaxTypeCommon.h"


class CJavaSyntaxGeneric;
class CJavaSyntaxType : public CJavaSyntaxTypeCommon
{
CONSTRUCTABLE(CJavaSyntaxType);
protected:
	CJavaSyntaxGeneric*		mpcGeneric;

public:
	void 					Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 					Kill(void);

	char*					GetType(void) override;
	void					TypePrint(CChars* pszDest, int iDepth) override;
	void					PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	BOOL					IsType(void) override;
	void					SetGeneric(CJavaSyntaxGeneric* pcGeneric);
	CJavaSyntaxGeneric*		GetGeneric(void);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxType,6>	CTypeArray;


#endif // !__JAVA_SYNTAX_TYPE_H__

