#ifndef __JAVA_INTEGER_H__
#define __JAVA_INTEGER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaLiteral.h"


enum EJavaIntegerType
{
	JIT_int32,
	JIT_int64,

	JIT_Unknown = -1
};


class CJavaInteger : public CJavaLiteral
{
CONSTRUCTABLE(CJavaInteger);
protected:
	int64				miValue;
	EJavaIntegerType	meType;

public:
	void 	Init(int32 iValue);
	void 	Init(int64 iValue);
	void 	Kill(void);

	char*	GetType(void);
	BOOL	Is(int32 i);
	BOOL	Is(int64 lli);

	void	Print(CChars* pszDest);
	BOOL	IsInteger(void);
};


#endif // !__JAVA_INTEGER_H__

