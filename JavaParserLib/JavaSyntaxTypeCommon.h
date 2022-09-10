#ifndef __JAVA_SYNTAX_TYPE_COMMON_H__
#define __JAVA_SYNTAX_TYPE_COMMON_H__
#include "JavaTokenIdentifier.h"
#include "JavaSyntax.h"


class CJavaSyntaxTypeCommon : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxTypeCommon);
protected:
	CJavaTokenIdentifier*	mpcName;

public:
	void 					Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 					Kill(void);

	char*					GetType(void) override;
	bool					IsTypeCommon(void);

	void					TypePrint(CChars* pszDest, int iDepth) override;
	void					PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	void					SetName(CJavaTokenIdentifier* pcName);
	CJavaTokenIdentifier*	GetName(void);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxTypeCommon, 6>	CTypeCommonArray;


#endif // !__JAVA_SYNTAX_TYPE_H__

