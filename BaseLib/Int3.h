#ifndef __INT3_H__
#define __INT3_H__
#include "Constructable.h"
#include "DataTypes.h"


class SInt3
{
CONSTRUCTABLE(SInt3);
public:
	int x;
	int y;
	int z;

	SInt3();
	SInt3(int x, int y, int z);

	void	Init(int x, int y, int z);
	void	Zero(void);

	bool operator == (const SInt3&) const;

	SInt3 operator + (const SInt3&) const;
	SInt3 operator - (const SInt3&) const;

	SInt3& operator += (const SInt3&);
	SInt3& operator -= (const SInt3&);

	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);
};


#endif // __INT3_H__

