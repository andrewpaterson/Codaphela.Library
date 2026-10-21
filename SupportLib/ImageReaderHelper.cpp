#include "ImageReaderHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddDebugChannel(Ptr<CImage> pcImage, EPrimitiveType eType, EChannel eChannel, bool bAddDebug)
{
	char* szShortName;
	char* szLongName;

	if (bAddDebug)
	{
		szShortName = gmiszImageChannelShortNames.Get((int)eChannel);
		szLongName = gmiszImageChannelLongNames.Get((int)eChannel);
		pcImage->AddChannel(eChannel, eType, szShortName, szLongName);
	}
	else
	{
		pcImage->AddChannel(eChannel, eType);
	}
}

