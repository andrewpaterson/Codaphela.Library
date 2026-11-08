#include "Shapes.h"


namespace
{
	int	giLocal = 1;

	int Local(void)
	{
		return giLocal;
	}
}


int Shapes::giCount = 10;


int Shapes::SBox::Area(void)
{
	return w * w;
}


int Shapes::SBox::Sides(void)
{
	return 4;
}


int Shapes::Scale(int a)
{
	return a * 2 + Local();
}


namespace Shapes
{
	int Scale(char c)
	{
		return c * 3;
	}

	namespace Inner
	{
		int Deep(void)
		{
			return giCount + 1;
		}
	}
}
