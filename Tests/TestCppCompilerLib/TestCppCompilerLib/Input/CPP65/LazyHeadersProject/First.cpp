//Compiled first: its headers are parsed and indexed, so the units after can leave them unparsed until used.
#include "Holder.h"
#include "Colour.h"
#include "Thing.h"


void CHolder::Set(int i)
{
	miValue = i;
}


int CHolder::Get(void)
{
	return miValue;
}


int ColourValue(EColour eColour)
{
	return (int)eColour * 100;
}


int UseThing(CThing* pcThing)
{
	return pcThing->Twice();
}
