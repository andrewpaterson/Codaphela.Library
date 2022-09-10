#include "BaseLib/LogString.h"
#include "BaseLib/StringHelper.h"
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
		return LongLongToString(oi);
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

