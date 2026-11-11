#ifndef __COLOUR_H__
#define __COLOUR_H__


//Unscoped: the enumerators are names in the namespace.
enum EColour
{
	COLOUR_Red,
	COLOUR_Green,
	COLOUR_Blue,
	COLOUR_Count,
};


int ColourValue(EColour eColour);


#endif // __COLOUR_H__
