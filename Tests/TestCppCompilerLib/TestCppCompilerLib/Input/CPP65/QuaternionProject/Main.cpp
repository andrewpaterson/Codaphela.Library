#include "BaseLib/Quaternion.h"


//The results go to 0010:0000, a long each.
uint32*	gpuiResult = (uint32*)0x100000;


void QuaternionResult(SQuaternion q)
{
	*gpuiResult++ = *(uint32*)&q.x;
	*gpuiResult++ = *(uint32*)&q.y;
	*gpuiResult++ = *(uint32*)&q.z;
	*gpuiResult++ = *(uint32*)&q.w;
}


void IntegerResult(int i)
{
	*gpuiResult++ = (uint32)i;
}


#define QUATERNION(x)	QuaternionResult(x)
#define INTEGER(x)		IntegerResult(x)


int main()
{
#include "QuaternionCalls.h"
	return (int)(gpuiResult - (uint32*)0x100000);
}

