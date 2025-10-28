#ifndef __COLOUR_H__
#define __COLOUR_H__
#include "BaseLib/PrimitiveTypes.h"


struct SColourType
{
	uint16	uiType;
};


struct SColour16 : SColourType  //Includes SColour8
{
	uint16	ui;
};


struct SColour32 : SColourType
{
	uint8	ui1;
	uint8	ui2;
	uint8	ui3;
	uint8	ui4;
};


struct SColour64 : SColourType
{
	uint16	uiType;
	uint16	ui1;
	uint16	ui2;
	uint16	ui3;
	uint16	ui4;
};


struct SColour128 : SColourType
{
	uint16	uiType;
	uint32	ui1;
	uint32	ui2;
	uint32	ui3;
	uint32	ui4;
};


#endif // __COLOUR_H__

