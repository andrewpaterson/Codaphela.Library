#ifndef __FILE_PATH_BUILDER_H__
#define __FILE_PATH_BUILDER_H__
#include "Chars.h"


class CFilePathBuilder
{
private:
	CChars	szPath;

public:
	char*	Build(char* szElements, ...);
	void	Kill(void);
};


#endif // __FILE_PATH_BUILDER_H__

