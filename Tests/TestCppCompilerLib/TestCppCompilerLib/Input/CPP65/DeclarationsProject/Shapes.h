//A dozen classes with bases and overrides, and a class that is the name of a template's instance: what the
//units do with them is counted (TestCPP65DeclarationsCounts).
#pragma once


template<class M>
class CHolderTemplate
{
public:
	M	mcHeld;

	int	Count(void) { return 1; }
};


class CGauge
{
public:
	int	miLevel;
};


class CGaugeHolder : public CHolderTemplate<CGauge>
{
};


class CShape
{
public:
	int	miSize;

	virtual int	Area(void);
	virtual int	Sides(void);
};


class CSquare : public CShape
{
public:
	int	Area(void) override;
	int	Sides(void) override;
};


class CTriangle : public CShape
{
public:
	int	Area(void) override;
	int	Sides(void) override;
};


class CCircle : public CShape
{
public:
	int	Area(void) override;
	int	Sides(void) override;
};


class CHexagon : public CShape
{
public:
	int	Area(void) override;
	int	Sides(void) override;
};


class COctagon : public CShape
{
public:
	int	Area(void) override;
	int	Sides(void) override;
};


class CCube : public CSquare
{
public:
	int	Area(void) override;
};


class CPrism : public CTriangle
{
public:
	int	Area(void) override;
};


class CBall : public CCircle
{
public:
	int	Area(void) override;
};


class CTile : public CSquare
{
public:
	int	miColour;
};


class CCoin : public CCircle
{
public:
	int	miValue;
};


int First(void);
int SquareArea(CSquare* pcSquare);
int Named(CTriangle* pcTriangle, CCircle* pcCircle, CHexagon* pcHexagon, COctagon* pcOctagon, CCube* pcCube, CPrism* pcPrism, CBall* pcBall, CTile* pcTile, CCoin* pcCoin, CGaugeHolder* pcHolder);
int Held(CGaugeHolder* pcHolder);
int Late(int x);
