//A class that takes its base's operator= by a using-declaration.  Assigning a base to it must find that operator in
//a unit that has not needed the class whole before (TestCPP65DeclarationsHeaderAssignment).
#pragma once


class CAssignBase
{
public:
	int	miValue;

	CAssignBase&	operator = (const CAssignBase& cOther) { miValue = cOther.miValue + 1; return *this; }
};


class CAssignDerived : public CAssignBase
{
public:
	using CAssignBase::operator =;
};
