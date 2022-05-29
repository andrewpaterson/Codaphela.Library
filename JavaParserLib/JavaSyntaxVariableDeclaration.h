#ifndef __JAVA_SYNTAX_VARIABLE_DECLARATION_H__
#define __JAVA_SYNTAX_VARIABLE_DECLARATION_H__
#include "JavaModifiers.h"
#include "JavaTokenIdentifier.h"
#include "JavaTokenKeyword.h"
#include "JavaSyntaxVariableInitialiser.h"
#include "JavaSyntaxStatement.h"
#include "JavaSyntaxGeneric.h"


// int x;
// X x;
// X.Y.Z x;
// int[] x;
// X[] x;
// X.Y.Z[] x;

// int x = exp;
// X x = exp;
// X.Y.Z x = exp;
// int[] x = { exp, ..., exp };
// X[] x = { exp, ..., exp };
// X.Y.Z[] x = { exp, ..., exp };


class CJavaSyntaxVariableDeclaration : public CJavaSyntaxStatement
{
CONSTRUCTABLE(CJavaSyntaxVariableDeclaration);
protected:
	CJavaModifiers						mcModifiers;

	CJavaTokenKeyword*					mpcPrimitiveType;
	CIdentifierArray					mapcType;
	CJavaSyntaxGeneric*					mpcGeneric;
	int									miArrayDimension;

	CJavaTokenIdentifier*				mpcName;
	CJavaSyntaxVariableInitialiser*		mpcInitialiser;

public:
	void 	Init(CJavaSyntaxTree* pcTree, CJavaSyntax* pcParent);
	void 	Kill(void);

	char*				GetType(void) override;
	void				TypePrint(CChars* pszDest, int iDepth) override;
	void				PrettyPrint(CChars* pszDest, int iBlockDepth = 0) override;

	BOOL				IsVariableDeclaration(void) override;
	BOOL				IsCompoundStatement(void);

	void				SetModifiers(CJavaModifiers cModifiers);
	void				SetPrimitiveType(CJavaTokenKeyword* pcPrimitiveType);
	void				AddIdentifierType(CJavaTokenIdentifier* pcIdentifier);
	void				SetGeneric(CJavaSyntaxGeneric* pcGeneric);
	void				SetName(CJavaTokenIdentifier* pcName);
	void				SetArrayDimension(int iArrayDimension);

	void				SetInitialiser(CJavaSyntaxVariableInitialiser* pcInitialiser);

	BOOL				IsPrimitiveType(void);
	BOOL				IsReferenceType(void);
	BOOL				IsGeneric(void);
	int					GetArrayDepth(void);
	char*				GetName(void);
	CJavaModifiers*		GetModifiers(void);

	CIdentifierArray*	GetReferenceType(void);
	CJavaTokenKeyword*	GetPrimitiveType(void);
	CJavaSyntaxGeneric* GetGeneric(void);
};


#endif // !__JAVA_SYNTAX_VARIABLE_DECLARATION_H__

