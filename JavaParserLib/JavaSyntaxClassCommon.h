#ifndef __JAVA_SYNTAX_TOP_LEVEL_H__
#define __JAVA_SYNTAX_TOP_LEVEL_H__
#include "BaseLib/ArrayTemplatePtr.h"
#include "JavaSyntaxStatement.h"
#include "JavaModifiers.h"
#include "JavaTokenIdentifier.h"



//FileBlock
//  class (final, abstract, public, package, strictfp)
//    ClassBlock
//  enum  (public, package, strictfp)
//  interface (abstract, public, package, strictfp)

//ClassBlock ( {, ...  }; )
//  Variable Declaraion (final static, public, protected, private, package)
//    Variable Assignment (=, += ...)
//  Code Block ( {, ...  }; )
//    class (final, abstract, static, package, strictfp)
//      ClassBlock
//  

class CJavaSyntaxClassCommon : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxClassCommon);
protected:
	CJavaModifiers	mcModifiers;

public:
	void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 				Kill(void);

	char*				GetType(void) override;
	void				TypePrint(CChars* pszDest, int iDepth) override;
	void				PrettyPrint(CChars* pszDest, int iBlockDepth) override;

	bool				IsClassCommon(void) override;

	CJavaModifiers*		GetModifiers(void);
	void				SetModifiers(CJavaModifiers cModifiers);
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxClassCommon, 6>	CClassCommonArray;


#endif // !__JAVA_SYNTAX_TOP_LEVEL_H__

