#ifndef __XML_PROLOG_H__
#define __XML_PROLOG_H__
#include "BaseLib/Bool.h"
#include "CharEncoding.h"


class CXMLProlog
{
protected:
	ECharEncoding	meEncoding;
	BOOL			mbStandalone;
	float			mfVersion;

public:
	void	Init(void);
	void	Kill(void);

	void	SetEncoding(ECharEncoding eEncoding);
	void	SetStandalone(BOOL bStandalone);
	void	SetVersion(float fVersion);
};


#endif // !__XML_PROLOG_H__

