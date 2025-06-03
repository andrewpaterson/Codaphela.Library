#ifndef __TRACE_VALUE_H__
#define __TRACE_VALUE_H__
#include "BaseLib/Chars.h"


enum ETraceValue
{
	TV_Low,
	TV_High,
	TV_Unsettled,
	TV_Undriven,
};


struct STraceValue
{
	ETraceValue	eValue;

	void	Init(void);
	void	Init(ETraceValue eTraceValue);
	void	Init(bool bValue);
	bool	IsHigh(void);
	bool	IsLow(void);

	void	Invert(void);

	void	Print(CChars* psz);
};


#endif // __TRACE_VALUE_H__

