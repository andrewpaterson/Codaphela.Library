#ifndef __CHANNEL_DEBUG_DESCRIPTOR_H__
#define __CHANNEL_DEBUG_DESCRIPTOR_H__
#include "BaseLib/MapIntTemplate.h"


class CChannelDebugDescriptor
{
public:
	const char*	mszLongName;
	const char*	mszShortName;

	void Init(const char* szLongName, const char* szShortName);
	void Kill(void);
};


typedef CMapIntTemplate<CChannelDebugDescriptor>	CMapIntChannelDescriptor;


#endif // __CHANNEL_DEBUG_DESCRIPTOR_H__

