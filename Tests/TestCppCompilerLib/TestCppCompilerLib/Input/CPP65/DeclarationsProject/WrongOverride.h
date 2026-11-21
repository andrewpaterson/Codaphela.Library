//A class whose function is declared override and overrides nothing: reported by a unit that needs the class whole,
//not by one that only names it (TestCPP65DeclarationsHeaderClass).
#pragma once


class CRight
{
public:
	virtual int	Value(void) { return 1; }
};


class CWrong : public CRight
{
public:
	int	Other(void) override { return 2; }
};
