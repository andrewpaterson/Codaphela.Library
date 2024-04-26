#ifndef __XML_PROLOG_H__
#define __XML_PROLOG_H__
#include "CharEncoding.h"


class CXMLProlog
{
protected:
	ECharEncoding	meEncoding;
	bool			mbStandalone;
	float			mfVersion;

public:
	void	Init(void);
	void	Kill(void);

	void	SetEncoding(ECharEncoding eEncoding);
	void	SetStandalone(bool bStandalone);
	void	SetVersion(float fVersion);
};


#endif // __XML_PROLOG_H__

