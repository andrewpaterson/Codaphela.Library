#include "Shapes.h"


namespace
{
	int	giLocal = 2;

	int Local(void)
	{
		return giLocal;
	}
}


int Other::Scale(int a)
{
	return a * 100;
}


int Other::Local(void)
{
	return ::Local();
}
