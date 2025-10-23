#ifndef __COLOUR_FORMATS_H__
#define __COLOUR_FORMATS_H__


enum EColourFormat
{
	CF_Unknown,

	//Palettised colour.
	CF_Indexed,

	//24bit colour.
	CF_R8G8B8,  
	CF_R8G8B8X8,
	CF_X8R8G8B8,

	//15bit colour.
	CF_R5G5B5X1,
	CF_X1R5G5B5,

	//16bit colour.
	CF_R5G6B5,
	CF_R5G5B6,
	CF_R6G5B5,

	//8bit colour.
	CF_R3G3B2,
	CF_R3G2B3,
	CF_R2G3B3,
};


#endif // __COLOUR_FORMATS_H__

