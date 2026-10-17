#ifndef __ATOMIC_INT_H__
#define __ATOMIC_INT_H__


//A 32 bit integer with sequentially consistent atomic operations.  It is the
//same size as an int and has no constructor so it can be placed in shared
//memory and in memory that is cleared with memset.
class CAtomicInt
{
protected:
	volatile int	miValue;

public:
	void	Set(int iValue);
	int		Get(void);

	int		Increment(void);
	int		Decrement(void);
	int		Add(int iValue);
	bool	CompareExchange(int iExpected, int iValue);
};


#endif // __ATOMIC_INT_H__

