#ifndef __W65C816_PINS_H__
#define __W65C816_PINS_H__
#include "BaseLib/PrimitiveTypes.h"
#include "TraceValue.h"


class CTimeline;
class CW65C816Pins
{
protected:
	uint16	muiAddress;
	uint8	muiData;
	bool	mbVPB;
	bool	mbRDY;
	bool	mbABORTB;
	bool	mbIRQB;
	bool	mbNMIB;
	bool	mbMLB;
	bool	mbVPA;
	bool	mbVDA;
	bool	mbRESB;
	bool	mbMX;
	bool	mbPHI2;
	bool	mbBE;
	bool	mbE;
	bool	mbRWB;

public:
	void			Init(void);

	//Microprocessor side methods.
	void			WriteRWB(CTimeline* pcTimeline, bool bValue);
	void			WriteMX(CTimeline* pcTimeline, bool bValue);
	void			WriteVDA(CTimeline* pcTimeline, bool bValue);
	void			WriteVPA(CTimeline* pcTimeline, bool bValue);
	void			WriteMLB(CTimeline* pcTimeline, bool bValue);
	void			WriteVPB(CTimeline* pcTimeline, bool bValue);
	void			WriteE(CTimeline* pcTimeline, bool bValue);
	void			WriteRdy(CTimeline* pcTimeline, bool bValue);
	void			WriteAddress(CTimeline* pcTimeline, uint16 uiAddress);
	void			WriteData(CTimeline* pcTimeline, uint8 uiData);

	uint8			ReadData(CTimeline* pcTimeline);
	STraceValue		ReadRES(CTimeline* pcTimeline);
	STraceValue		ReadPhi2(CTimeline* pcTimeline);
	STraceValue		ReadNMI(CTimeline* pcTimeline);
	STraceValue		ReadIRQ(CTimeline* pcTimeline);
	STraceValue		ReadAbort(CTimeline* pcTimeline);
	STraceValue		ReadBE(CTimeline* pcTimeline);

	//Circuit side methods.
	void			Reset(void);
	void			HalfCycle(void);

	void			SetData(uint8 uiData);
	void			SetRDY(bool bValue);
	void			SetABORTB(bool bValue);
	void			SetIRQB(bool bValue);
	void			SetNMIB(bool bValue);
	void			SetRESB(bool bValue);
	void			SetPHI2(bool bValue);
	void			SetBE(bool bValue);

	uint8			GetData(void);
	uint16			GetAddress(void);

	bool			GetVPB(void);
	bool			GetRDY(void);
	bool			GetMLB(void);
	bool			GetVPA(void);
	bool			GetVDA(void);
	bool			GetMX(void);
	bool			GetE(void);
	bool			GetRWB(void);
	bool			GetPHI2(void);
};


#endif // __W65C816_PINS_H__

