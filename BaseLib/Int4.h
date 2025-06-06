#ifndef __INT4_H__
#define __INT4_H__
#include "DataIO.h"
#include "Constructable.h"
#include "DataTypes.h"


class SInt4
{
CONSTRUCTABLE(SInt4);
public:
	int x;
	int y;
	int z;
	int w;

	void	Init(int x, int y, int z, int w);
	void	Zero(void);

	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);
};


#endif // __INT4_H__

