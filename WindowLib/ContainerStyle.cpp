#include "ContainerStyle.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int AlignComponentVertical(EContainerStyleVertical eAlignment, int iComponentHeight, int iAvailableHeight)
{
	if (eAlignment == CSV_Top)
	{
		return 0;
	}
	else if (eAlignment == CSV_Center)
	{
		return (iAvailableHeight - iComponentHeight) / 2;
	}
	else if (eAlignment == CSV_Bottom)
	{
		return iAvailableHeight - iComponentHeight;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int AlignComponentHorizontal(EContainerStyleHorizontal eAlignment, int iComponentWidth, int iAvailableWidth)
{
	if (eAlignment == CSH_Left)
	{
		return 0;
	}
	else if (eAlignment == CSH_Center)
	{
		return (iAvailableWidth - iComponentWidth) / 2;
	}
	else if (eAlignment == CSH_Right)
	{
		return iAvailableWidth - iComponentWidth;
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 ContainerStyleMainAxis(SInt32Vec2* psVec, bool bHorizontal)
{
	return bHorizontal ? psVec->x : psVec->y;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int32 ContainerStyleCrossAxis(SInt32Vec2* psVec, bool bHorizontal)
{
	return bHorizontal ? psVec->y : psVec->x;
}

