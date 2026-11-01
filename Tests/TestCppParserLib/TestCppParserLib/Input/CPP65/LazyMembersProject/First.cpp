//Compiled first: its headers are parsed and indexed.
#include "Box.h"
#include "Scale.h"


int FirstValue(void)
{
	CBox<int>	cBox;

	cBox.Set(1);
	return cBox.Get();
}
