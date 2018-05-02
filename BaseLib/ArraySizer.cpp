#include "ArraySizer.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CalculateArraySized(int iUsedElements)
{
	if (iUsedElements <= 8)
	{
		if (iUsedElements <= 4)
		{
			if (iUsedElements <= 2)
			{
				if (iUsedElements <= 1)
				{
					return iUsedElements;
				}
				return 2;
			}
			return 4;
		}
		return 8;
	}
	else if (iUsedElements <= 60)
	{
		if (iUsedElements <= 40)
		{
			if (iUsedElements <= 27)
			{
				if (iUsedElements <= 18)
				{
					if (iUsedElements <= 12)
					{
						return 12;
					}
					return 18;
				}
				return 27;
			}
			return 40;
		}
		return 60;
	}
	else if (iUsedElements <= 691)
	{
		if (iUsedElements <= 461)
		{
			if (iUsedElements <= 307)
			{
				if (iUsedElements <= 201)
				{
					if (iUsedElements <= 136)
					{
						if (iUsedElements <= 91)
						{
							return 91;
						}
						return 136;
					}
					return 201;
				}
				return 307;
			}
			return 461;
		}
		return 691;
	}
	else if (iUsedElements <= 7882)
	{
		if (iUsedElements <= 5254)
		{
			if (iUsedElements <= 3503)
			{
				if (iUsedElements <= 2335)
				{
					if (iUsedElements <= 1556)
					{
						if (iUsedElements <= 1037)
						{
							return 91;
						}
						return 136;
					}
					return 201;
				}
				return 307;
			}
			return 461;
		}
		return 7882;
	}
	else
	{
		return iUsedElements;
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL ArrayMustBeResizedForAdd(int iNewElements, int iOldElements, int* piNewArraySize)
{
	return FALSE;
}

