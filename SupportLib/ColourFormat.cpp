#include "ImageChannel.h"
#include "ColourFormat.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CColourFormatHelper::Init(EColourFormat eFormat, EColourOrder eOrder, ERGBColourBits eColourBits, ERGBAlphaBits eAlphaBits)
{
	meOrder = eOrder;
	meFormat = eFormat;
	meColourBits = eColourBits;
	meAlphaBits = eAlphaBits;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CColourFormatHelper::GetNumChannels(void)
{
	switch (meFormat)
	{
	case CF_R8G8B8:
		return 3;
	case CFT_RGBX:
	case CFT_XRGB:
	case CFT_RGBA:
	case CFT_ARGB:
		return 4;
	case CFT_Unknown:
		return 0;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EChannel CColourFormatHelper::GetChannel(size uiIndex)
{
	switch (meFormat)
	{
	case CF_R8G8B8:
		return GetColourChannel(uiIndex);

	case CFT_RGBX:
		if (uiIndex <= 2)
		{
			return GetColourChannel(uiIndex);
		}
		else if (uiIndex == 3)
		{
			return IMAGE_IGNORED;
		}
	case CFT_RGBA:
		if (uiIndex <= 2)
		{
			return GetColourChannel(uiIndex);
		}
		else if (uiIndex == 3)
		{
			return IMAGE_OPACITY;
		}

	case CFT_XRGB:
		if (uiIndex == 0)
		{
			return IMAGE_IGNORED;
		}
		else if (uiIndex <= 3)
		{
			return GetColourChannel(uiIndex - 1);
		}
	case CFT_ARGB:
		if (uiIndex == 0)
		{
			return IMAGE_OPACITY;
		}
		else if (uiIndex <= 3)
		{
			return GetColourChannel(uiIndex - 1);
		}
	}

	return IMAGE_CHANNEL_UNKNOWN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CColourFormatHelper::GetType(size uiIndex)
{
	switch (meFormat)
	{
	case CF_R8G8B8:
		return GetColourType(uiIndex);

	case CFT_RGBX:
		if (uiIndex <= 2)
		{
			return GetColourType(uiIndex);
		}
		else if (uiIndex == 3)
		{
			return GetAlphaType();
		}
	case CFT_RGBA:
		if (uiIndex <= 2)
		{
			return GetColourType(uiIndex);
		}
		else if (uiIndex == 3)
		{
			return GetAlphaType();
		}

	case CFT_XRGB:
		if (uiIndex == 0)
		{
			return GetAlphaType();
		}
		else if (uiIndex <= 3)
		{
			return GetColourType(uiIndex - 1);
		}
	case CFT_ARGB:
		if (uiIndex == 0)
		{
			return GetAlphaType();
		}
		else if (uiIndex <= 3)
		{
			return GetColourType(uiIndex - 1);
		}
	}

	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EChannel CColourFormatHelper::GetColourChannel(size uiIndex)
{
	switch (meOrder)
	{
	case CCO_RGB:
		switch (uiIndex)
		{
		case 0:
			return IMAGE_DIFFUSE_RED;
		case 1:
			return IMAGE_DIFFUSE_GREEN;
		case 2:
			return IMAGE_DIFFUSE_BLUE;
		}
		break;
	case CCO_BRG:
		switch (uiIndex)
		{
		case 1:
			return IMAGE_DIFFUSE_RED;
		case 2:
			return IMAGE_DIFFUSE_GREEN;
		case 0:
			return IMAGE_DIFFUSE_BLUE;
		}
		break;
	case CCO_GBR:
		switch (uiIndex)
		{
		case 2:
			return IMAGE_DIFFUSE_RED;
		case 0:
			return IMAGE_DIFFUSE_GREEN;
		case 1:
			return IMAGE_DIFFUSE_BLUE;
		}
		break;
	case CCO_BGR:
		switch (uiIndex)
		{
		case 2:
			return IMAGE_DIFFUSE_RED;
		case 1:
			return IMAGE_DIFFUSE_GREEN;
		case 0:
			return IMAGE_DIFFUSE_BLUE;
		}
		break;
	case CCO_RBG:
		switch (uiIndex)
		{
		case 0:
			return IMAGE_DIFFUSE_RED;
		case 2:
			return IMAGE_DIFFUSE_GREEN;
		case 1:
			return IMAGE_DIFFUSE_BLUE;
		}
		break;
	case CCO_GRB:
		switch (uiIndex)
		{
		case 1:
			return IMAGE_DIFFUSE_RED;
		case 0:
			return IMAGE_DIFFUSE_GREEN;
		case 2:
			return IMAGE_DIFFUSE_BLUE;
		}
		break;
	}
	return IMAGE_CHANNEL_UNKNOWN;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CColourFormatHelper::GetColourType(size uiIndex)
{
	if (uiIndex <= 2)
	{
		switch (meColourBits)
		{
		case CRGB_24bit:
			return PT_uint8;
		case CRGB_48bit:
			return PT_uint16;
		case CRGB_Float3:
			return PT_float32;
		case CRGB_8bit332:
			switch (uiIndex)
			{
			case 0:
				return PT_tribble;
			case 1:
				return PT_tribble;
			case 2:
				return PT_crumb;
			}
			break;
		case CRGB_8bit323:
			switch (uiIndex)
			{
			case 0:
				return PT_tribble;
			case 1:
				return PT_crumb;
			case 2:
				return PT_tribble;
			}
			break;
		case CRGB_8bit233:
			switch (uiIndex)
			{
			case 0:
				return PT_crumb;
			case 1:
				return PT_tribble;
			case 2:
				return PT_tribble;
			}
			break;
		case CRGB_6bit:
			return PT_crumb;
		case CRGB_15bit:
			return PT_nickle;
		case CRGB_16bit:
			switch (uiIndex)
			{
			case 0:
				return PT_nickle;
			case 1:
				return PT_sixbits;
			case 2:
				return PT_nickle;
			}
			break;
		}
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
EPrimitiveType CColourFormatHelper::GetAlphaType(void)
{
	switch (meAlphaBits)
	{
	case ARGB_8bit:
		return PT_uint8;
	case ARGB_16bit:
		return PT_uint16;
	case ARGB_Float:
		return PT_float32;
	case ARGB_2bit:
		return PT_crumb;
	}
	return PT_Undefined;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CColourFormatHelper::GetFirstColourIndex(void)
{
	switch (meFormat)
	{
	case CF_R8G8B8:
	case CFT_RGBX:
	case CFT_RGBA:
		return 0;
	case CFT_XRGB:
	case CFT_ARGB:
		return 1;
	}

	return SIZE_MAX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CColourFormatHelper::GetAlphaIndex(void)
{
	switch (meFormat)
	{
	case CFT_RGBA:
		return 3;
	case CFT_ARGB:
		return 0;
	case CF_R8G8B8:
	case CFT_RGBX:
	case CFT_XRGB:
		break;
	}

	return SIZE_MAX;
}

