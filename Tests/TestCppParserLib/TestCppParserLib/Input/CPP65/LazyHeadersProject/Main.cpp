#include "Thing.h"
#include "Colour.h"


//An enumerator as an array size, before anything else in Colour.h is named.
int	gaiTable[COLOUR_Count];


int main()
{
	CThing	cThing;
	CHolder	cHolder;

	cThing.Set(5);
	cHolder.Set(7);
	return UseThing(&cThing) + ColourValue(COLOUR_Blue) + (int)(sizeof(gaiTable) / sizeof(int)) * 1000 + cThing.FromHolder(&cHolder) * 10000;
}
