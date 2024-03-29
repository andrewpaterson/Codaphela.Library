#ifndef __JAVA_SYNTAX_STATEMENT_H__
#define __JAVA_SYNTAX_STATEMENT_H__
#include "BaseLib/ArrayTemplateEmbeddedPtr.h"
#include "JavaModifiers.h"
#include "JavaSyntax.h"


//Everything in a Class- or Code Block is a statement.
class  CJavaSyntaxStatement : public CJavaSyntax
{
CONSTRUCTABLE(CJavaSyntaxStatement);
protected:
public:
			void 				Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
			void 				Kill(void);

			char*				GetType(void) override;
			void				TypePrint(CChars* pszDest, int iDepth) override;

			void				PrintModifiers(CChars* pszDest, CJavaModifiers cModifiers);

			bool				IsStatement(void) override;
	virtual bool				IsCompoundStatement(void) =0;
};


typedef CArrayTemplateEmbeddedPtr<CJavaSyntaxStatement, 6> CStatementArray;


#endif // !__JAVA_SYNTAX_STATEMENT_H__

