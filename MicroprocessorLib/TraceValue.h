#ifndef __TRACE_VALUE_H__
#define __TRACE_VALUE_H__


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
};


#endif // __TRACE_VALUE_H__

