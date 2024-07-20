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
		szShortName = gmiszImageChannelShortNames.Get(eChannel);
		szLongName = gmiszImageChannelLongNames.Get(eChannel);
		pcImage->AddChannel(eChannel, eType, szShortName, szLongName);
	}
	else
	{
		pcImage->AddChannel(eChannel, eType);
	}
}

