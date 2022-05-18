#ifndef __TEXT_POSITION_H__
#define __TEXT_POSITION_H__


struct STextPosition
{
	char* szParserPos;
	int			iLine;
	int			iColumn;

	void Init(void);
};


#endif // !__TEXT_POSITION_H__

