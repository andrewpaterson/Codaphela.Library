#include "ImageReaderHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void AddDebugChannel(Ptr<CImage> pImage, EPrimitiveType eType, EChannel eChannel, bool bAddDebug)
{
	char* szShortName;
	char* szLongName;

	if (bAddDebug)
	{
		szShortName = gmiszImageChannelShortNames.Get((int)eChannel);
		szLongName = gmiszImageChannelLongNames.Get((int)eChannel);
		pImage->AddChannel(eChannel, eType, szShortName, szLongName);
	}
	else
	{
		pImage->AddChannel(eChannel, eType);
	}
}

