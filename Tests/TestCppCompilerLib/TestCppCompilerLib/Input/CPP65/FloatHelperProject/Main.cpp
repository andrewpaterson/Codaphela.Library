#include "BaseLib/FloatHelper.h"


//The results go to 0010:0000, a long or two each.
uint32*	gpuiResult = (uint32*)0x100000;


void Float32Result(float32 f)
{
	*gpuiResult++ = *(uint32*)&f;
}


void Float64Result(float64 d)
{
	unsigned long long	ulli;

	ulli = *(unsigned long long*)&d;
	*gpuiResult++ = (uint32)ulli;
	*gpuiResult++ = (uint32)(ulli >> 32);
}


void IntegerResult(int i)
{
	*gpuiResult++ = (uint32)i;
}


#define FLOAT32(x)	Float32Result(x)
#define FLOAT64(x)	Float64Result(x)
#define INTEGER(x)	IntegerResult(x)


int main()
{
#include "FloatHelperCalls.h"
	return (int)(gpuiResult - (uint32*)0x100000);
}

