#ifndef __INT2_H__
#define __INT2_H__
#include "Constructable.h"
#include "DataTypes.h"


class SInt2
{
CONSTRUCTABLE(SInt2);
public:
	int x, y;

	SInt2();
	SInt2(const SInt2& v);
	SInt2(int x, int y);

	void	Init(int x, int y);
	void	Init(const SInt2& v);
	void	Zero(void);

	bool operator == (const SInt2&) const;

	SInt2 operator + (const SInt2&) const;
	SInt2 operator - (const SInt2&) const;

	SInt2& operator += (const SInt2&);
	SInt2& operator -= (const SInt2&);

	bool	Save(CFileWriter* pcFile);
	bool	Load(CFileReader* pcFile);
};


#endif // __INT2_H__

