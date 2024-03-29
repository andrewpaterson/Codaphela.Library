#ifndef __JAVA_SYNTAX_PACKAGE_H__
#define __JAVA_SYNTAX_PACKAGE_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntax.h"
#include "JavaTokenIdentifier.h"


class CJavaSyntaxPackage : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxPackage);
protected:
	CIdentifierArray	mapcIdentifiers;

public:
	void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 				Kill(void);

	char*				GetType(void) override;
	void				TypePrint(CChars* pszDest, int iDepth) override;
	void				PrettyPrint(CChars* pszDest, int iBlockDepth = 0);

	void				AddIdentifier(CJavaTokenIdentifier* pcIdentifier);

	bool				IsPackage(void) override;

	CIdentifierArray*	GetIdentifiers(void);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxPackage, 6>	CPackageArray;


#endif // !__JAVA_SYNTAX_PACKAGE_H__

