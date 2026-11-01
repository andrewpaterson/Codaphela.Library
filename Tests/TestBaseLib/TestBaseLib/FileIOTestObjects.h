#ifndef __FILE_IO_TEST_OBJECTS_H__
#define __FILE_IO_TEST_OBJECTS_H__
#include "BaseLib/Define.h"


class CFileIOTest
{
protected:
	char	msz[17];
	int		miNumber;

public:
	void Init(int iNumber);
	void Kill(void);

	bool IsOkay(int iNumber);
};

class CFileIOAlignedTest
{
protected:
	char	b[16];

public:
	void Init(char bShunt);
	void Kill(void);

	bool IsAligned(int iAlignment);
	bool IsOkay(char bShunt);
};


#endif // __FILE_IO_TEST_OBJECTS_H__

