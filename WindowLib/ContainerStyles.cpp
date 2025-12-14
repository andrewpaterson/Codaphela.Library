#include "ContainerStyles.h"


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
int AlignComponentHorizontal(EContainerStyleVertical eAlignment, int iComponentWidth, int iAvailableWidth)
{
	if (eAlignment == CSV_Top)
	{
		return 0;
	}
	else if (eAlignment == CSV_Center)
	{
		return (iAvailableWidth - iComponentWidth) / 2;
	}
	else if (eAlignment == CSV_Bottom)
	{
		return iAvailableWidth - iComponentWidth;
	}
	else
	{
		return 0;
	}
}

