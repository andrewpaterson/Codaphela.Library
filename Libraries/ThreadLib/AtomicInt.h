#ifndef __ATOMIC_INT_H__
#define __ATOMIC_INT_H__


//WaitUntilAtLeast does not time out.
#define ATOMIC_WAIT_FOREVER	-1


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

	//Yields until the value is at least iValue.  Returns false if that takes
	//longer than iTimeoutMilliseconds.
	bool	WaitUntilAtLeast(int iValue, int iTimeoutMilliseconds);
};


#endif // __ATOMIC_INT_H__

