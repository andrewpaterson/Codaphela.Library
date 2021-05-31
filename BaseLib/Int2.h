#ifndef __INT2_H__
#define __INT2_H__
#include "DataTypes.h"


class SInt2
{
CONSTRUCTABLE(SInt2);
public:
	int x, y;

	SInt2();
	SInt2(int x, int y);

	void	Init(int x, int y);
	void	Zero(void);

	BOOL operator == (const SInt2&) const;

	SInt2 operator + (const SInt2&) const;
	SInt2 operator - (const SInt2&) const;

	SInt2& operator += (const SInt2&);
	SInt2& operator -= (const SInt2&);

	BOOL	Save(CFileWriter* pcFile);
	BOOL	Load(CFileReader* pcFile);
	BOOL	Save(CFileWriter* pcFile, size_t uiCount);
	BOOL	Load(CFileReader* pcFile, size_t uiCount);
};


#endif // __INT2_H__

