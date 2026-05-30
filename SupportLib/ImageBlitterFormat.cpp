#include "ImageBlitterFormat.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CImageBlitterFormat::Init(void)
{
	meColourOrder = CCO_Unknown;
	meSourceColourFormat = CFT_Unknown;
	meDestColourFormat = CFT_Unknown;
	meColourBits = CRGB_Unknown;
	meSourceAlphaBits = ARGB_Unknown;
	meDestAlphaBits = ARGB_Unknown;
	meSourceOpacity = CPO_Unknown;
}

