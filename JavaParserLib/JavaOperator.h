#ifndef __JAVA_OPERATOR_H__
#define __JAVA_OPERATOR_H__
#include "BaseLib/Chars.h"
#include "JavaToken.h"


enum EJavaOperatorType
{
	JOT_Arithmetic,
	JOT_Unary,
	JOT_Assignment,
	JOT_Relational,
	JOT_Logical,
	JOT_Ternary,
	JOT_Bitwise,
	JOT_Unknown = -1
};


enum EJavaOperator
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

	JO_Unknown = -1
};


class CJavaOperatorDefinition;
class CJavaOperator : public CJavaToken
{
CONSTRUCTABLE(CJavaOperator);
protected:
	CJavaOperatorDefinition*	mpcOperator;

public:
	void 	Init(CJavaOperatorDefinition* pcOperator);
	void 	Kill(void);

	void	Print(CChars* pszDest);
	char*	GetType(void);
	BOOL	IsOperator(void);
	BOOL	Is(EJavaOperator eOperator);
};


class CJavaOperatorDefinition
{
protected:
	EJavaOperator		meOperator;
	EJavaOperatorType	meType;
	CChars				mszName;

public:
	void 				Init(EJavaOperatorType meType, EJavaOperator eOperator, char* szName);
	void 				Kill(void);
	EJavaOperatorType	GetType(void);
	char*				GetName(void);
	BOOL				Is(EJavaOperator eOperator);
};


#endif // !__JAVA_OPERATOR_H__

