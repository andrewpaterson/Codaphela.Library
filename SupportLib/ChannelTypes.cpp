#include "ChannelTypes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void PrintChannelType(EChannelType eType, CChars* psz)
{
	switch (eType)
	{
	case CT_Red:
		psz->Append("Red");
		break;
	case CT_Green:
		psz->Append("Green");
		break;
	case CT_Blue:
		psz->Append("Blue");
		break;
	case CT_Hue:
		psz->Append("Hue");
		break;
	case CT_Saturation:
		psz->Append("Saturation");
		break;
	case CT_Value:
		psz->Append("Value");
		break;
	case CT_Intensity:
		psz->Append("Intensity");
		break;
	case CT_X:
		psz->Append("X");
		break;
	case CT_Y:
		psz->Append("Y");
		break;
	case CT_Z:
		psz->Append("Z");
		break;
	case CT_U:
		psz->Append("U");
		break;
	case CT_V:
		psz->Append("V");
		break;
	case CT_Ignored:
		psz->Append("Ignored");
		break;
	case CT_Index:
		psz->Append("Index");
		break;
	default:
		psz->Append("INVALID");
		break;
	}
}

