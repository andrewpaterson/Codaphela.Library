#include "Shapes.h"


using Shapes::SBox;


int main(void)
{
	SBox				sBox;
	Shapes::TBox<int>	sInt;
	int					i;

	sBox.w = 3;
	sInt.t = 7;
	i = sBox.Area();							//9
	i += SBox::Sides();							//4
	i += Shapes::Square;						//3
	i += Shapes::Twice(5);						//10
	i += sInt.Get();							//7
	i += Shapes::Inner::Deep();					//11
	i += Shapes::Scale(1000);					//2001
	i += Shapes::Scale((char)2);				//6
	i += Other::Scale(5);						//500
	i += Other::Local();						//2
	return i;
}
