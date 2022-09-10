#ifndef __JAVA_OPERATOR_H__
#define __JAVA_OPERATOR_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaTokenOperatorType
{
	JOT_Arithmetic,
	JOT_Unary,
	JOT_Assignment,
	JOT_Relational,
	JOT_Logical,
	JOT_Ternary,
	JOT_Bitwise,
	JOT_Method,
	JOT_Unknown = -1
};


enum EJavaTokenOperator
{
	JO_Plus,
	JO_Minus,
	JO_Divide,
	JO_Multipy,
	JO_Modulus,

	JO_Increment,
	JO_Decrement,

	JO_Assign,
	JO_PlusAssign,
	JO_MinusAssign,
	JO_MultiplyAssign,
	JO_DivideAssign,
	JO_ModulusAssign,
	JO_ExclusiveOrAssign,
	JO_LeftShiftAssign,
	JO_RightShiftAssign,
	JO_RightShiftZeroAssign,

	JO_Equal,
	JO_NotEqual,
	JO_LessThan,
	JO_GreaterThan,
	JO_LessThanOrEqual,
	JO_GreaterThanOrEqual,

	JO_LogicalNegate,
	JO_LogicalAnd,
	JO_LogicalOr,

	JO_TernaryCondition,
	JO_TernaryStatement,

	JO_BitwiseAnd,
	JO_BitwiseOr,
	JO_BitwiseExclusiveOr,
	JO_BitwiseNot,
	JO_LeftShift,
	JO_RightShift,
	JO_RightShiftZero,

	JO_MethodReference,
	JO_LambdaOperator,

	JO_Unknown = -1
};


class CJavaTokenOperatorDefinition;
class CJavaTokenOperator : public CJavaToken
{
CONSTRUCTABLE(CJavaTokenOperator);
protected:
	CJavaTokenOperatorDefinition*	mpcOperator;

public:
	void 	Init(STextPosition* psPosition, CJavaTokenOperatorDefinition* pcOperator);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
	bool	IsOperator(void);
	bool	Is(EJavaTokenOperator eOperator);
};


class CJavaTokenOperatorDefinition
{
protected:
	EJavaTokenOperator		meOperator;
	EJavaTokenOperatorType	meType;
	CChars				mszName;

public:
	void 				Init(EJavaTokenOperatorType meType, EJavaTokenOperator eOperator, char* szName);
	void 				Kill(void);
	EJavaTokenOperatorType	GetType(void);
	char*				GetName(void);
	bool				Is(EJavaTokenOperator eOperator);
};


#endif // !__JAVA_OPERATOR_H__

