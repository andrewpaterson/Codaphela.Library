#include "LogString.h"
#include "StringHelper.h"
#include "IndexedGeneral.h"


char	gszInvalidIndex[] = "-1 (INVALID)";
char	gszNullIndex[] = "0 (NULL)";


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* IndexToString(OIndex oi)
{
	if (oi == INVALID_O_INDEX)
	{
		return gszInvalidIndex;
	}
	else if (oi == NULL_O_INDEX)
	{
		return gszNullIndex;
	}
	else
	{
		return LongToString(oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsValidIndex(OIndex oi)
{
	return oi != INVALID_O_INDEX && oi != NULL_O_INDEX;
}

