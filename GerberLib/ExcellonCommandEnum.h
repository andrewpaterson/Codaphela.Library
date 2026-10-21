#ifndef __EXCELLON_COMMAND_ENUM_H__
#define __EXCELLON_COMMAND_ENUM_H__


//The Excellon drill commands written by KiCad.
enum EExcellonCommandEnum
{
	EC_Unknown,
	EC_Comment,				//; Comment
	EC_FileAttribute,		//; #@! TF.  Gerber X2 file attribute.
	EC_ApertureAttribute,	//; #@! TA.  Gerber X2 aperture attribute of the next tool.
	EC_DeleteAttribute,		//; #@! TD  Deletes the aperture attributes.
	EC_HeaderStart,			//M48
	EC_Format,				//FMAT,n
	EC_Unit,				//INCH or METRIC
	EC_ToolDefine,			//TnCd  Defines tool n with diameter d.
	EC_HeaderEnd,			//% or M95
	EC_Absolute,			//G90
	EC_DrillMode,			//G05
	EC_ToolSelect,			//Tn  Tool 0 unloads the tool.
	EC_Hit,					//XxYy  Drills a hole in drill mode.  Continues the route in route mode.
	EC_RouteMove,			//G00XxYy  Selects route mode and moves to x, y with the tool up.
	EC_RouteLinear,			//G01XxYy  Routes in a straight line to x, y.
	EC_ToolDown,			//M15
	EC_ToolUp,				//M16
	EC_Slot,				//XxYyG85XxYy  A canned slot from the first to the second point.
	EC_EndOfProgram,		//M30
};


enum EExcellonUnit
{
	EU_Unknown,
	EU_Inches,
	EU_Millimeters,
};


#endif // __EXCELLON_COMMAND_ENUM_H__

