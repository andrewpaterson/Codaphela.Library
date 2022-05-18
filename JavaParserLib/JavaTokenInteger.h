#ifndef __JAVA_INTEGER_H__
#define __JAVA_INTEGER_H__
#include "BaseLib/Chars.h"
#include "BaseLib/PrimitiveTypes.h"
#include "JavaTokenLiteral.h"


enum EJavaTokenIntegerType
{
	JIT_int32,
	JIT_int64,

	JIT_Unknown = -1
};


class CJavaTokenInteger : public CJavaTokenLiteral
{
CONSTRUCTABLE(CJavaTokenInteger);
protected:
	int64				miValue;
	EJavaTokenIntegerType	meType;

public:
	void 	Init(STextPosition* psPosition, int32 iValue);
	void 	Init(STextPosition* psPosition, int64 iValue);
	void 	Kill(void);

	char*	GetType(void);
	BOOL	Is(int32 i);
	BOOL	Is(int64 lli);

	void	Print(CChars* pszDest);
	BOOL	IsInteger(void);
};


#endif // !__JAVA_INTEGER_H__

