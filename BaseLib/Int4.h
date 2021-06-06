#ifndef __INT4_H__
#define __INT4_H__
#include "DataIO.h"
#include "DataTypes.h"


class SInt4
{
CONSTRUCTABLE(SInt4);
public:
	int x, y, z, w;

	void	Init(int x, int y, int z, int w);
	void	Zero(void);

	BOOL	Save(CFileWriter* pcFile);
	BOOL	Load(CFileReader* pcFile);
};


#endif // __INT4_H__

