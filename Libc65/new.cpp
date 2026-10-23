//The global allocation operators for the 65C64, in C++ for the 65C64 generator: new Type and new Type[count] get
//their memory here, delete and delete[] give it back.  A program can define its own instead.
#include <new>
#include <stdlib.h>


void* operator new(size_t uiBytes)
{
	return malloc(uiBytes);
}


void* operator new[](size_t uiBytes)
{
	return malloc(uiBytes);
}


void operator delete(void* pv) noexcept
{
	free(pv);
}


void operator delete[](void* pv) noexcept
{
	free(pv);
}
