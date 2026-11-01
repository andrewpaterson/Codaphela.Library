#include "GeneralPins.h"
#include "SDCard.h"


#define  SD_SLEEP 1


void SSDCardPins::Init(int iSDClkPin, int iSDCmdPin, int iSDDat0Pin, int iSDDat1Pin, int iSDDat2Pin, int iSDDat3Pin)
{
    this->iSDClkPin = iSDClkPin;
    this->iSDCmdPin = iSDCmdPin;

    this->iSDDat0Pin = iSDDat0Pin;
    this->iSDDat1Pin = iSDDat1Pin;
    this->iSDDat2Pin = iSDDat2Pin;
    this->iSDDat3Pin = iSDDat3Pin;
}


void sd_card_init(SSDCardPins* pPins)
{
    gpio_init(pPins->iSDClkPin);
    gpio_set_dir(pPins->iSDClkPin, GPIO_OUT);

    gpio_init(pPins->iSDCmdPin);
    gpio_init(pPins->iSDDat0Pin);
    gpio_init(pPins->iSDDat1Pin);
    gpio_init(pPins->iSDDat2Pin);
    gpio_init(pPins->iSDDat3Pin);
}


static uint8_t crc7(uint8_t uiCRC, uint8_t uiData) 
{
	uiCRC ^= uiData;
	for (uint8_t i = 0; i < 8; i++) 
    {
		if (uiCRC & 0x80) 
        {
            uiCRC ^= 0x89;
        }
		uiCRC <<= 1;
	}

	return uiCRC;
}


uint8_t crc7(uint8_t* pvData, int iLength)
{
	uint8_t uiCRC = 0;

	for (int i = 0; i < iLength; i++) 
    {
        uiCRC = crc7(uiCRC, pvData[i]);
    }

	return uiCRC >> 1;
}


uint16_t crc16(uint8_t* pvData, int iLength) 
{
    uint16_t crc;
    uint8_t i;

    crc = 0;
	for (int iByte = 0; iByte < iLength; iByte++)
    {
        crc = crc ^ ((uint16_t) (pvData[iByte] << 8));
        for(int iBit = 0; iBit < 8; iBit++)
        {
            if( crc & 0x8000)
            {
                crc = (crc << 1) ^ 0x1021;
            }
            else
            {
                crc = crc << 1;
            }
        }
    }

    return crc;
}

void build_command(uint8_t* pvDest, int iCommand, int iArgument)
{
    pvDest[0] = 0x40 | (iCommand & 0x3f);  //0..1...command[5..0]...
    pvDest[1] = (iArgument >> 24) & 0xff;  //argument[31..24]...
    pvDest[2] = (iArgument >> 16) & 0xff;  //argument[23..16]...
    pvDest[3] = (iArgument >> 8) & 0xff;   //argument[15..8]...
    pvDest[4] = iArgument & 0xff;          //argument[7..0]...
    uint8_t uiCRC = crc7(pvDest, 5);
    pvDest[5] = (uiCRC << 1) | 1;
}


bool send_command(SSDCardPins* pPins, uint8_t* pvDest)
{
    gpio_set_dir(pPins->iSDCmdPin, GPIO_OUT);
    gpio_put(pPins->iSDClkPin, false);
    gpio_put(pPins->iSDCmdPin, true);
    sleep_us_high_power(SD_SLEEP);
    for (int iByte = 0; iByte <= 5; iByte++)
    {
        uint8_t uiByte = pvDest[iByte];
        for (int iBit = 7; iBit >=0; iBit--)
        {
            bool bBit = uiByte & 0x80;
            uiByte <<= 1;
            gpio_put(pPins->iSDClkPin, false);
            gpio_put(pPins->iSDCmdPin, bBit);
            sleep_us_high_power(SD_SLEEP);
            gpio_put(pPins->iSDClkPin, true);
            sleep_us_high_power(SD_SLEEP);
        }
    }
}


bool read_response(SSDCardPins* pPins, int iExpectedBytes, uint8_t* pvResponse)
{
    int iBit = 6;
    for (int iByte = 0; iByte < iExpectedBytes; iByte++)
    {
        pvResponse[iByte] = 0;
        for (; iBit >= 0; iBit--)
        {
            pvResponse[iByte] <<= 1;
            gpio_put(pPins->iSDClkPin, false);
            sleep_us_high_power(SD_SLEEP);
            gpio_put(pPins->iSDClkPin, true);
            bool bBit = gpio_get(pPins->iSDCmdPin);
            sleep_us_high_power(SD_SLEEP);
            pvResponse[iByte] |= bBit;
        }
        iBit = 7;
    }
    return true;
}


void sd_clock_tick(SSDCardPins* pPins, int iFullCycles, int iCmdDir)
{
    if (gpio_get_dir(pPins->iSDCmdPin) != iCmdDir)
    {
        gpio_set_dir(pPins->iSDCmdPin, iCmdDir);
    }

    if (iCmdDir == GPIO_OUT)
    {
        gpio_put(pPins->iSDCmdPin, true);
    }

    for (int i = 0; i < iFullCycles; i++)
    {
        gpio_put(pPins->iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        gpio_put(pPins->iSDClkPin, true);
        sleep_us_high_power(SD_SLEEP);
    }
}


void sd_initial_tick(SSDCardPins* pPins)
{
    gpio_set_dir(pPins->iSDCmdPin, GPIO_OUT);
    gpio_put(pPins->iSDCmdPin, true);

    for (int i = 0; i < 80; i++)
    {
        gpio_put(pPins->iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        gpio_put(pPins->iSDClkPin, true);
        sleep_us_high_power(SD_SLEEP);
    }
}


bool receive_response(SSDCardPins* pPins, int iExpectedBytes, uint8_t* pvResponse)
{
    if (gpio_get_dir(pPins->iSDCmdPin) == GPIO_OUT)
    {
        gpio_set_dir(pPins->iSDCmdPin, GPIO_IN);
    }

    bool bCardTransmit = false;
    for (int i = 0; i < 1000; i++)
    {
        gpio_put(pPins->iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        gpio_put(pPins->iSDClkPin, true);
        bool bBit = gpio_get(pPins->iSDCmdPin);
        sleep_us_high_power(SD_SLEEP);
        if (!bBit)
        {
            bCardTransmit = true;
            break;
        }
    }

    if (bCardTransmit)
    {
        bool bValidResponse = read_response(pPins, iExpectedBytes, pvResponse);
        if (bValidResponse)
        {
            sd_clock_tick(pPins, 4, GPIO_OUT);
        }
        return bValidResponse;
    }
    else
    {
        return false;
    }
}


void sd_cmd0_go_idle(SSDCardPins* pPins, bool bSDMode)
{
    uint8_t aCommand[6];

    build_command(aCommand, 0, 0);
    
    gpio_set_dir(pPins->iSDDat3Pin, GPIO_OUT);
    gpio_put(pPins->iSDDat3Pin, !bSDMode);

    send_command(pPins, aCommand);

    gpio_set_dir(pPins->iSDDat3Pin, GPIO_IN);

    sd_clock_tick(pPins, 8, GPIO_OUT);
}


#pragma pack (push, 1)
struct SSDResponseR1
{
    uint8_t uiCmd;  //Bit 7   : 0 (Start transmit)
                    //Bit 6   : 0 (Card transmit)
                    //Bit 5..0: cmd

    uint8_t uiStatus3;
    uint8_t uiStatus2;
    uint8_t uiStatus1;
    uint8_t uiStatus0;

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};
#pragma pack (push)


bool sd_cmd8_interface_condition(SSDCardPins* pPins)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 8, 0x000001AA);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if ((pResponse->uiCmd == 8) &&
            (pResponse->uiStatus1 == 1) &&
            (pResponse->uiStatus0 == 0xAA) &&
            (pResponse->uiCrc7 == 0x13))
        {
            return true;
        }
    }
    return false;
}


void calculate_status(SSDCardStatus* pStatus, SSDResponseR1* pResponse)
{
    pStatus->bOutOfRange = (pResponse->uiStatus3 & 0x80) == 0x80;
    pStatus->bAddressError = (pResponse->uiStatus3 & 0x40) == 0x40;
    pStatus->bBlockLengthError = (pResponse->uiStatus3 & 0x20) == 0x20;
    pStatus->bSequenceEraseError = (pResponse->uiStatus3 & 0x10) == 0x10;
    pStatus->bWriteBlockEraseError = (pResponse->uiStatus3 & 0x8) == 0x8;
    pStatus->bWriteProtectedViolation = (pResponse->uiStatus3 & 0x4) == 0x4;
    pStatus->bCardIsLocked = (pResponse->uiStatus3 & 0x2) == 0x2;
    pStatus->bLockOrUnlockFailed = (pResponse->uiStatus3 & 0x1) == 0x1;
    pStatus->bPreviousCRCFailed = (pResponse->uiStatus2 & 0x80) == 0x80;
    pStatus->bIllegalCommand = (pResponse->uiStatus2 & 0x40) == 0x40;
    pStatus->bInternalECCFailed = (pResponse->uiStatus2 & 0x20) == 0x20;
    pStatus->bInternalError = (pResponse->uiStatus2 & 0x10) == 0x10;
    pStatus->bGeneralError = (pResponse->uiStatus2 & 0x8) == 0x8;

    pStatus->bCSDOverwriteError = (pResponse->uiStatus2 & 0x1) == 0x1;
    pStatus->bErasePartiallyFailed = (pResponse->uiStatus1 & 0x80) == 0x80;
    pStatus->bInternalECCDisabled = (pResponse->uiStatus1 & 0x40) == 0x40;
    pStatus->bEraseNotExecuted = (pResponse->uiStatus1 & 0x20) == 0x20;

    pStatus->eState = (ESDState)((pResponse->uiStatus1 & 0x1E) >> 1);

    pStatus->bReadyForData = (pResponse->uiStatus1 & 0x1) == 0x1;
    pStatus->bExtensionFunctionEvent = (pResponse->uiStatus0 & 0x40) == 0x40;
    pStatus->bApplicationCommandExpected = (pResponse->uiStatus0 & 0x20) == 0x20;
    pStatus->bAuthenticationError = (pResponse->uiStatus0 & 0x8) == 0x8;
}


bool sd_cmd55_application_specific(SSDCardPins* pPins, u_int16_t uiRCA, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool bResult;

    build_command(aCommand, 55, uiRCA << 16);
    send_command(pPins, aCommand);

    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {   
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if ((pResponse->uiCmd == 55) &&
            ((pResponse->uiStatus1 & 1) == 1) &&
            ((pResponse->uiStatus0 & 0x20) == 0x20))
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                return true;
            }
        }
    }
    return false;
}


#pragma pack (push, 1)
struct SSDResponseR3
{
    uint8_t uiReserved1;  //Bit 7   : 0 (Start transmit)
                          //Bit 6   : 0 (Card transmit)
                          //Bit 5..0: 0b111111

    uint8_t uiOCR3;
    uint8_t uiOCR2;
    uint8_t uiOCR1;
    uint8_t uiOCR0;

    uint8_t uiReserved0;  //Bit 6..1: 0b1111111
                          //Bit 0   : 1 (Stop transmit)
};
#pragma pack (pop)


bool sd_cmd41_operating_condition(SSDCardPins* pPins, uint8_t* paResponse)
{
    uint8_t aCommand[6];
    bool bResult;

    build_command(aCommand, 41, 0x40100000);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, paResponse);
    if (bResult)
    {
        SSDResponseR3* pResponse = (SSDResponseR3*)paResponse;
        if ((pResponse->uiReserved1 == 0x3F) &&
            (pResponse->uiReserved0 == 0xFF))
        {
            return true;
        }
    }
    return false;
}


bool sd_acmd41_application_operating_condition(SSDCardPins* pPins, u_int16_t uiRCA, SSDOCR* pOCR)
{
    uint8_t         aResponse[6];
    bool            bResult;
    SSDCardStatus   sStatus;    

    bResult = sd_cmd55_application_specific(pPins, 0, &sStatus);
    if (!bResult)
    {
        return false;
    }

    bResult = sd_cmd41_operating_condition(pPins, aResponse);
    if (!bResult)
    {
        return false;
    }

    SSDResponseR3* pResponse = (SSDResponseR3*)aResponse;
    pOCR->bDonePoweringOn = (pResponse->uiOCR3 & 0x80) == 0x80;
    if (pOCR->bDonePoweringOn)
    {
        pOCR->b35_36Volts = (pResponse->uiOCR2 & 0x80) == 0x80;
        pOCR->b34_35Volts = (pResponse->uiOCR2 & 0x40) == 0x40;
        pOCR->b33_34Volts = (pResponse->uiOCR2 & 0x20) == 0x20;
        pOCR->b32_33Volts = (pResponse->uiOCR2 & 0x10) == 0x10;
        pOCR->b31_32Volts = (pResponse->uiOCR2 & 0x8) == 0x8;
        pOCR->b30_31Volts = (pResponse->uiOCR2 & 0x4) == 0x4;
        pOCR->b29_30Volts = (pResponse->uiOCR2 & 0x2) == 0x2;
        pOCR->b28_29Volts = (pResponse->uiOCR2 & 0x1) == 0x1;
        pOCR->b27_28Volts = (pResponse->uiOCR1 & 0x80) == 0x80;

        pOCR->bSwitchTo18Volts = (pResponse->uiOCR3 & 0x1) == 0x1;
        pOCR->bOver2TBytes = (pResponse->uiOCR3 & 0x8) == 0x8;
        pOCR->bUHSIIStatus = (pResponse->uiOCR3 & 0x20) == 0x20;
        pOCR->bCardCapacityStatus = (pResponse->uiOCR3 & 0x40) == 0x40;
    }
    return true;
}


bool repeat_sd_acmd41_application_operating_condition(SSDCardPins* pPins, u_int16_t uiRCA, SSDOCR* pOCR)
{
    bool bResult;

    for (int iCount = 0; iCount < 1000; iCount++)
    {
        bResult = sd_acmd41_application_operating_condition(pPins, 0, pOCR);
        if (bResult)
        {
            if (pOCR->bDonePoweringOn)
            {
                return true;
            }
        }
        else
        {
            return false;
        }
    }
    return false;
}


#pragma pack (push, 1)
struct SSDResponseR2CID
{
    uint8_t     uiReserved; //Bit 7   : 0 (Start transmit)
                            //Bit 6   : 0 (Card transmit)
                            //Bit 5..0: 0b111111

    uint8_t     uiManufacturerID;
    uint16_t    uiApplicationID;
    char        acProductName[5];
    uint8_t     uiProductRevision;
    uint8_t     auiProductSerialNumber[4];

    uint16_t    uiManufactureDate;  //Bit 15..12: 0bXXXX
                                    //Bit 11..4 : Year (+2000)
                                    //Bit 3..0  : Month

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};
#pragma pack (pop)


bool sd_cmd2_send_cid(SSDCardPins* pPins, SSDCID* pCID)
{
    uint8_t aCommand[6];
    uint8_t aResponse[17];
    bool    bResult;

    build_command(aCommand, 2, 0);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 17, aResponse);
    if (bResult)
    {
        SSDResponseR2CID* pResponse = (SSDResponseR2CID*)aResponse;
        if (pResponse->uiReserved == 0x3F)
        {
            uint8_t uiExpectedCRC = crc7(&aResponse[1], 15);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                pCID->uiManufacturerID = pResponse->uiManufacturerID;
                pCID->uiApplicationID = pResponse->uiApplicationID;
                pCID->uiProductRevision = pResponse->uiProductRevision;
                
                memcpy(pCID->szProductName, pResponse->acProductName, 5);
                pCID->szProductName[5] = '\0';
                
                pCID->iProductSerialNumber = *((int*)pResponse->auiProductSerialNumber);

                pCID->uiManufactureYear = ((pResponse->uiManufactureDate >> 4) & 0xFF) + 2000;
                pCID->uiManufactureMonth =  pResponse->uiManufactureDate & 0xF;

                return true;
            }
        }
    }
    return false;
}


#pragma pack (push, 1)
struct SSDResponseR6
{
    uint8_t uiCmd;  //Bit 7   : 0 (Start transmit)
                    //Bit 6   : 0 (Card transmit)
                    //Bit 5..0: cmd

    uint8_t uiRCA1;
    uint8_t uiRCA0;
    uint8_t uiStatus1;  
    uint8_t uiStatus0;

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};
#pragma pack (pop)


bool sd_cmd3_publish_relative_address(SSDCardPins* pPins, uint16_t* puiAddress, SSDR6Status* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 3, 0);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR6* pResponse = (SSDResponseR6*)aResponse;
        if (pResponse->uiCmd == 3)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                *puiAddress = *((uint16_t*)&pResponse->uiRCA1);
                pStatus->bPreviousCRCFailed = (pResponse->uiStatus1 & 0x80) == 0x80;
                pStatus->bIllegalCommand = (pResponse->uiStatus1 & 0x40) == 0x40;
                pStatus->bGeneralError = (pResponse->uiStatus1 & 0x20) == 0x20;
                pStatus->eState = (ESDState)((pResponse->uiStatus1 & 0x1E) >> 1);
                
                pStatus->bReadyForData = (pResponse->uiStatus0 & 0x100) == 0x100;
                pStatus->bExtensionFunctionEvent = (pResponse->uiStatus0 & 0x40) == 0x40;
                pStatus->bApplicationCommandExpected = (pResponse->uiStatus0 & 0x20) == 0x20;
                pStatus->bAuthenticationError = (pResponse->uiStatus0 & 0x8) == 0x8;
                return true;
            }   
        }
    }
    return false;
}


int address_argument(uint16_t uiAddress)
{
    int iArgument = uiAddress;
    iArgument <<= 16;

    return iArgument;
}


int calculate_nanosecond_multiplier(uint8_t uiValue)
{
    if (uiValue == 0)
    {
        return 1;  //nano
    }
    if (uiValue == 1)
    {
        return 10;
    }
    if (uiValue == 2)
    {
        return 100;
    }
    if (uiValue == 3)
    {
        return 1000;  //micro
    }
    if (uiValue == 4)
    {
        return 10000;
    }
    if (uiValue == 5)
    {
        return 100000;
    }
    if (uiValue == 6)
    {
        return 1000000;  //milli
    }
    if (uiValue == 7)
    {
        return 10000000;
    }
    return 0;
}


float calculate_digit_multiplier(uint8_t uiValue)
{
    if (uiValue == 1)
    {
        return 1.0f;
    }
    if (uiValue == 2)
    {
        return 1.2f;
    }
    if (uiValue == 3)
    {
        return 1.3f;
    }
    if (uiValue == 4)
    {
        return 1.5f;
    }
    if (uiValue == 5)
    {
        return 2.0f;
    }
    if (uiValue == 6)
    {
        return 2.5f;
    }
    if (uiValue == 7)
    {
        return 3.0f;
    }
    if (uiValue == 8)
    {
        return 3.5f;
    }
    if (uiValue == 9)
    {
        return 4.0f;
    }
    if (uiValue == 10)
    {
        return 4.5f;
    }
    if (uiValue == 11)
    {
        return 5.0f;
    }
    if (uiValue == 12)
    {
        return 5.5f;
    }
    if (uiValue == 13)
    {
        return 6.0f;
    }
    if (uiValue == 14)
    {
        return 7.0f;
    }
    if (uiValue == 15)
    {
        return 8.0f;
    }
    return -1.0f;
}


int calculate_transmission_speed_multiplier(uint8_t uiValue)
{
    if (uiValue == 0)
    {
        return 100'000;  //100 Kbits/second
    }
    if (uiValue == 1)
    {
        return 1'000'000;  //1 Mbits/second
    }
    if (uiValue == 2)
    {
        return 10'000'000;
    }
    if (uiValue == 3)
    {
        return 100'000'000;
    }
    return 0;
}


#pragma pack (push, 1)
struct SSDResponseR2CSD
{
    uint8_t     uiReserved; //Bit 7   : 0 (Start transmit)
                            //Bit 6   : 0 (Card transmit)
                            //Bit 5..0: 0b111111

    uint8_t     uiCSDStructure; //Bit 7..6 : 0b00 (CSD structure)                          
                                //Bit 5..0 : 0b000000

    uint8_t     uiDataReadAccessTime1;
    uint8_t     uiDataReadAccessTime0;  //In clock ticks
    uint8_t     uiMaxDataTransferRate;  //0x32 or 0x5a
    uint8_t     uiCardCommandClasses1;  //          0b01X11011
    uint8_t     uiMaxReadBlockLength;   //Bit 7..4: 0b0101
                                        //Bit 3..0: 0xX (Length)
                                        
    uint8_t     uiFlags4;   //Bit 7    : Read partial block
                            //Bit 6    : Write misaligned block
                            //Bit 5    : Read misaligned block
                            //Bit 4    : DSR implemented
                            //Bit 3..2 : Reserved
                            //Bit 1..0 : Device size [11..10]

    uint8_t     uiDeviceSize;  // Device size [9..2]

    uint8_t     uiCurrentRatings1;  //Bit 7..6  : Device size [1..0]
                                    //Bit 5..3  : Max read current at VDD_min
                                    //Bit 2..0  : Max read current at VDD_max
    uint8_t     uiCurrentRatings0;  //Bit 7..5  : Max write current at VDD_min
                                    //Bit 4..2  : Max write current at VDD_max
                                    //Bit 1..0  : Device size multiplier [2..1]

    uint8_t     uiFlags3;   //Bit 7    : Device size multiplier [0]
                            //Bit 6    : Erase single block
                            //Bit 5..0 : Erase sector size [5..1]

    uint8_t     uiFlags2;   //Bit 7    : Erase sector size [0]
                            //Bit 6..0 : Write protect group size

    uint8_t     uiFlags1;   //Bit 7    : Write protect group
                            //Bit 6..5 : 0b00
                            //Bit 4..2 : Write speed factor
                            //Bit 1..0 : Max write data block length [3..2]

    uint8_t     uiWriteBlock;   //Bit 7..6 : Max write block length [1..0]
                                //Bit 5    : Write block partial
                                //Bit 4..0 : 0b00000

    uint8_t     uiFlags0;   //Bit 7    : File format group
                            //Bit 6    : Copy
                            //Bit 5    : Permanent write protection                            
                            //Bit 4    : Temporary write protection
                            //Bit 3..2 : File format
                            //Bit 1..0 : 0b00

    uint8_t uiCrc7;  //Bit 6..1: CRC7
                     //Bit 0   : 1 (Stop transmit)
};
#pragma pack (pop)


bool sd_cmd9_send_csd(SSDCardPins* pPins, uint16_t uiAddress, SSDCSD* pCSD)
{
    uint8_t aCommand[6];
    uint8_t aResponse[17];
    bool bResult;

    build_command(aCommand, 9, address_argument(uiAddress));
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 17, aResponse);
    if (bResult)
    {
        SSDResponseR2CSD* pResponse = (SSDResponseR2CSD*)aResponse;
        if (pResponse->uiReserved == 0x3F)
        {
            uint8_t uiExpectedCRC = crc7(&aResponse[1], 15);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                pCSD->eCSDStructure = (ESDRegisterStructure)(pResponse->uiCSDStructure >> 6);

                int iMultiplier = calculate_nanosecond_multiplier(pResponse->uiDataReadAccessTime1 & 0x7);
                float fDigit = calculate_digit_multiplier((pResponse->uiDataReadAccessTime1 >> 3) & 0xF);
                pCSD->fAsynchronousDataAccessTime = iMultiplier * fDigit;

                iMultiplier = calculate_nanosecond_multiplier(pResponse->uiDataReadAccessTime0 & 0x7);
                fDigit = calculate_digit_multiplier((pResponse->uiDataReadAccessTime0 >> 3) & 0xF);
                pCSD->fWorstCaseDataAccessTime = iMultiplier * fDigit;

                iMultiplier = calculate_transmission_speed_multiplier(pResponse->uiMaxDataTransferRate & 0x7);
                fDigit = calculate_digit_multiplier((pResponse->uiMaxDataTransferRate >> 3) & 0xF);
                pCSD->fSingleDataMaxTransmissionSpeed = iMultiplier * fDigit;

                uint16_t uiCardClasses = (pResponse->uiCardCommandClasses1 << 4) | (pResponse->uiMaxReadBlockLength >> 4);

                pCSD->bCommandClassSwitch = (uiCardClasses & (1 << 10)) == 1 << 10;
                pCSD->bCommandClassWriteProtect = (uiCardClasses & (1 << 6)) == 1 << 6;
                pCSD->bCommandClassLockCard = (uiCardClasses & (1 << 7)) == 1 << 7;
                pCSD->bCommandClassExtensions = (uiCardClasses & (1 << 11)) == 1 << 11;

                pCSD->iMaxReadBlockLength = 1 << (pResponse->uiMaxReadBlockLength & 0xF);

                pCSD->bReadPartialBlock = (pResponse->uiFlags4 & 0x80) == 0x80;
                pCSD->bWriteMisalignedBlock = (pResponse->uiFlags4 & 0x40) == 0x40;
                pCSD->bReadMisalignedBlock = (pResponse->uiFlags4 & 0x20) == 0x20;
                pCSD->bDSRImplemented = (pResponse->uiFlags4 & 0x10) == 0x10;

                int iCSize = (pResponse->uiFlags4 & 0x3) << 8;
                iCSize |= pResponse->uiDeviceSize;
                iCSize <<= 2;
                iCSize |= pResponse->uiCurrentRatings1 >> 6;

                int iCSizeMultiplier = pResponse->uiCurrentRatings0 & 0x3;
                iCSizeMultiplier <<= 1;
                iCSizeMultiplier |= (pResponse->uiFlags3 & 0x80) >> 7;
                iMultiplier = 1 << (iCSizeMultiplier+2);
                int iBlockNr = (iCSize + 1) * iMultiplier;

                pCSD->iDeviceSize = pCSD->iMaxReadBlockLength * iBlockNr;

                pCSD->uiWriteSpeedFactor = 1 << ((pResponse->uiFlags1 >> 2) & 0x7);

                int iMaxWriteBlocklength = (pResponse->uiFlags1 & 0x3) << 2;
                iMaxWriteBlocklength |= (pResponse->uiWriteBlock >> 6) & 0x3;
                pCSD->iMaxWriteBlocklength = 1 << iMaxWriteBlocklength;

                pCSD->bWritePartialBlock = (pResponse->uiWriteBlock & 0x20) == 0x20;

                bool bFileFormatGroup = (pResponse->uiFlags0 & 0x80) == 0x80;
                pCSD->bCopy = (pResponse->uiFlags0 & 0x40) == 0x40;
                pCSD->bPermanentWriteProtection = (pResponse->uiFlags0 & 0x20) == 0x20;
                pCSD->bTemporaryWriteProtection = (pResponse->uiFlags0 & 0x10) == 0x10;

                if (bFileFormatGroup)
                {
                    pCSD->eFileFormat = SDFF_Reserved;
                }
                else
                {
                    pCSD->eFileFormat = (ESDFileFormat)((pResponse->uiFlags0 >> 2) & 0x3);
                }

                return true;
            }
        }
    }
    return false;
}


bool sd_cmd7_select_or_deselect_card(SSDCardPins* pPins, uint16_t uiAddress, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 7, address_argument(uiAddress));
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 7)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                return true;
            }
        }
    }
    return false;
}


bool read_data_narrow(SSDCardPins* pPins, int iExpectedBytes, uint8_t* pvData)
{
    for (int iByte = 0; iByte < iExpectedBytes; iByte++)
    {
        pvData[iByte] = 0;
        for (int iBit = 7; iBit >= 0; iBit--)
        {
            pvData[iByte] <<= 1;

            gpio_put(pPins->iSDClkPin, false);
            sleep_us_high_power(0);

            bool bBit = gpio_get(pPins->iSDDat0Pin);

            gpio_put(pPins->iSDClkPin, true);
            sleep_us_high_power(0);

            pvData[iByte] |= bBit;
        }
    }
    return true;
}


bool wait_for_transmit_narrow(SSDCardPins* pPins)
{
    if (gpio_get_dir(pPins->iSDDat0Pin) == GPIO_OUT)
    {
        gpio_set_dir(pPins->iSDDat0Pin, GPIO_IN);
    }

    bool  bCardTransmit = false;
    for (int i = 0; i < 2000; i++)
    {
        gpio_put(pPins->iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        gpio_put(pPins->iSDClkPin, true);
        bool bBit = gpio_get(pPins->iSDDat0Pin);
        sleep_us_high_power(SD_SLEEP);
        if (!bBit)
        {
            bCardTransmit = true;
            break;
        }
    }

    return bCardTransmit;
}


bool receive_data_narrow(SSDCardPins* pPins, int iExpectedBytes, uint8_t* pvData)
{
    bool bCardTransmit = wait_for_transmit_narrow(pPins);

    if (bCardTransmit)
    {
        bool bValidResponse = read_data_narrow(pPins, iExpectedBytes, pvData);
        if (bValidResponse)
        {
            uint16_t    uiCRC16Read;
            uint8_t     aData[2];
            uint16_t    uiExpectedCRC16;
            bValidResponse = read_data_narrow(pPins, 2, aData);
            sd_clock_tick(pPins, 4, GPIO_OUT);
            uiCRC16Read = (aData[0] << 8) | aData[1];
            if (bValidResponse)
            {
                uiExpectedCRC16 = crc16(pvData, iExpectedBytes);
                if (uiExpectedCRC16 == uiCRC16Read)
                {
                    return true;
                }
            }
        }
    }
    return false;
}


bool sd_cmd17_read_single_block_narrow(SSDCardPins* pPins, int iBlock, int iExpectedBytes, uint8_t* pvData, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 17, iBlock);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 17)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                receive_data_narrow(pPins, iExpectedBytes, pvData);
                return true;
            }
        }
    }
    return false;
}


bool sd_cmd6_switch(SSDCardPins* pPins, bool bSwitch, uint8_t uiPowerLimit, uint8_t uiDriveStrength, uint8_t uiCommandSystem, uint8_t uiAccessMode, SSDCardStatus* pStatus, SSDFunctionSwitchStatus* pFunctionSwitchStatus)
{
    uint8_t     aCommand[6];
    uint8_t     aResponse[6];
    bool        bResult;
    uint8_t     aDataResponse[512];
    uint32_t    uiArgument;

    uiArgument = bSwitch ? 0x80000000 : 0;
    build_command(aCommand, 6, uiArgument | (uiPowerLimit << 12) | (uiDriveStrength << 8) | (uiCommandSystem << 4) | uiAccessMode);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 6)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                memset(aDataResponse, 0, 512);
                bResult = receive_data_narrow(pPins, 512, aDataResponse);
                {
                    pFunctionSwitchStatus->uiMaximumCurrentInMilliAmps = aDataResponse[0] << 8 | aDataResponse[1];

                    pFunctionSwitchStatus->uiFunctionGroup6SupportBits = aDataResponse[2] << 8 | aDataResponse[3];
                    pFunctionSwitchStatus->uiFunctionGroup5SupportBits = aDataResponse[4] << 8 | aDataResponse[5];
                    pFunctionSwitchStatus->uiFunctionGroup4SupportBits = aDataResponse[6] << 8 | aDataResponse[7];
                    pFunctionSwitchStatus->uiFunctionGroup3SupportBits = aDataResponse[8] << 8 | aDataResponse[9];
                    pFunctionSwitchStatus->uiFunctionGroup2SupportBits = aDataResponse[10] << 8 | aDataResponse[11];
                    pFunctionSwitchStatus->uiFunctionGroup1SupportBits = aDataResponse[12] << 8 | aDataResponse[13];

                    pFunctionSwitchStatus->uiFunctionGroup6Selection = aDataResponse[14] >> 4;
                    pFunctionSwitchStatus->uiFunctionGroup5Selection = aDataResponse[14] & 0xF;
                    pFunctionSwitchStatus->uiFunctionGroup4Selection = aDataResponse[15] >> 4;
                    pFunctionSwitchStatus->uiFunctionGroup3Selection = aDataResponse[15] & 0xF;
                    pFunctionSwitchStatus->uiFunctionGroup2Selection = aDataResponse[16] >> 4;
                    pFunctionSwitchStatus->uiFunctionGroup1Selection = aDataResponse[16] & 0xF;

                    pFunctionSwitchStatus->uiDataStructureVersion = aDataResponse[17];

                    pFunctionSwitchStatus->uiFunctionGroup6BusyStatus = aDataResponse[18] << 8 | aDataResponse[19];
                    pFunctionSwitchStatus->uiFunctionGroup5BusyStatus = aDataResponse[20] << 8 | aDataResponse[21];
                    pFunctionSwitchStatus->uiFunctionGroup4BusyStatus = aDataResponse[22] << 8 | aDataResponse[23];
                    pFunctionSwitchStatus->uiFunctionGroup3BusyStatus = aDataResponse[24] << 8 | aDataResponse[25];
                    pFunctionSwitchStatus->uiFunctionGroup2BusyStatus = aDataResponse[26] << 8 | aDataResponse[27];
                    pFunctionSwitchStatus->uiFunctionGroup1BusyStatus = aDataResponse[28] << 8 | aDataResponse[29];
                    
                    return true;
                }
            }
        }
    }
    return false;
}


bool check_sd_cmd6_switch(SSDCardPins* pPins, uint8_t uiPowerLimit, uint8_t uiDriveStrength, uint8_t uiCommandSystem, uint8_t uiAccessMode, SSDCardStatus* pStatus, SSDFunctionSwitchStatus* pFunctionSwitchStatus)
{
    bool bResult;

    bResult = sd_cmd6_switch(pPins, false, uiPowerLimit, uiDriveStrength, uiCommandSystem, uiAccessMode, pStatus, pFunctionSwitchStatus);
    if (!bResult)
    {
        return false;
    }

    bool bPowerLimitSelected = false;
    if ((uiPowerLimit == 0xF) || (pFunctionSwitchStatus->uiFunctionGroup4Selection == uiPowerLimit))
    {
        bPowerLimitSelected = true;
    }

    bool bDriveStrengthSelected = false;
    if ((uiDriveStrength == 0xF) || (pFunctionSwitchStatus->uiFunctionGroup3Selection == uiDriveStrength))
    {
        bDriveStrengthSelected = true;
    }

    bool bCommandSystemSelected = false;
    if ((uiCommandSystem == 0xF) || (pFunctionSwitchStatus->uiFunctionGroup2Selection == uiCommandSystem))
    {
        bCommandSystemSelected = true;
    }

    bool bAccessModeSelected = false;
    if ((uiAccessMode == 0xF) || (pFunctionSwitchStatus->uiFunctionGroup1Selection == uiAccessMode))
    {
        bAccessModeSelected = true;
    }

    if (bPowerLimitSelected && bDriveStrengthSelected && bCommandSystemSelected && bAccessModeSelected)
    {
        return true;
    }

    return false;
}


bool sd_cmd6_set_bus_width(SSDCardPins* pPins, uint8_t uiBusWidth, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 6, uiBusWidth);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 6)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                return true;
            }
        }
    }
    return false;
}


bool sd_acmd6_set_bus_width(SSDCardPins* pPins, u_int16_t uiAddress, uint8_t uiBusWidth, SSDCardStatus* pStatus)
{
    uint8_t aResponse[6];
    bool    bResult;

    bResult = sd_cmd55_application_specific(pPins, uiAddress, pStatus);
    if (!bResult)
    {
        return false;
    }

    bResult = sd_cmd6_set_bus_width(pPins, uiBusWidth, pStatus);
    if (!bResult)
    {
        return false;
    }

    return true;
}


bool read_data_wide(SSDCardPins* pPins, int iExpectedBytes, uint8_t* pvData)
{
    for (int iByte = 0; iByte < iExpectedBytes; iByte++)
    {
        pvData[iByte] = 0;
        for (int iBit = 1; iBit >= 0; iBit--)
        {
            gpio_put(pPins->iSDClkPin, false);
            sleep_us_high_power(0);

            bool bBit3 = gpio_get(pPins->iSDDat3Pin);
            bool bBit2 = gpio_get(pPins->iSDDat2Pin);
            bool bBit1 = gpio_get(pPins->iSDDat1Pin);
            bool bBit0 = gpio_get(pPins->iSDDat0Pin);

            gpio_put(pPins->iSDClkPin, true);
            sleep_us_high_power(0);

            pvData[iByte] <<= 1;
            pvData[iByte] |= bBit3;
            pvData[iByte] <<= 1;
            pvData[iByte] |= bBit2;
            pvData[iByte] <<= 1;
            pvData[iByte] |= bBit1;
            pvData[iByte] <<= 1;
            pvData[iByte] |= bBit0;
        }
    }
    return true;
}


void unstripe_data(uint8_t* pUnstripedData, int iOffset, int iExpectedBytes, uint8_t* pvSourceData)
{
    memset(pUnstripedData, 0xff, iExpectedBytes / 4);
    int iUnstripeByte = 0;
    int ui1, ui2, ui3, ui4;
    int iByte;
    uint8_t uiUnstripedUpper;
    uint8_t uiUnstripedLower;

    for (iByte = 0; iByte < iExpectedBytes; iByte += 4)
    {
        ui1 = pvSourceData[iByte] << iOffset;
        ui2 = pvSourceData[iByte + 1] << iOffset;
        ui3 = pvSourceData[iByte + 2] << iOffset;
        ui4 = pvSourceData[iByte + 3] << iOffset;

        uiUnstripedUpper = (ui1 & 0x80) |  ((ui2 & 0x80) >> 2) | ((ui3 & 0x80) >> 4) | ((ui4 & 0x80) >> 6);
        uiUnstripedLower = ((ui1 & 0x8) << 3) |  ((ui2 & 0x8) << 1) | ((ui3 & 0x8) >> 1) | ((ui4 & 0x8) >> 3);
        pUnstripedData[iUnstripeByte] = uiUnstripedUpper | uiUnstripedLower;
        iUnstripeByte++;    
    }
}


bool wait_for_transmit_wide(SSDCardPins* pPins)
{
    if (gpio_get_dir(pPins->iSDDat1Pin) == GPIO_OUT)
    {
        gpio_set_dir(pPins->iSDDat1Pin, GPIO_IN);
    }
    if (gpio_get_dir(pPins->iSDDat2Pin) == GPIO_OUT)
    {
        gpio_set_dir(pPins->iSDDat2Pin, GPIO_IN);
    }
    if (gpio_get_dir(pPins->iSDDat3Pin) == GPIO_OUT)
    {
        gpio_set_dir(pPins->iSDDat3Pin, GPIO_IN);
    }
    if (gpio_get_dir(pPins->iSDDat0Pin) == GPIO_OUT)
    {
        gpio_set_dir(pPins->iSDDat0Pin, GPIO_IN);
    }

    bool  bCardTransmit = false;
    for (int i = 0; i < 2000; i++)
    {
        gpio_put(pPins->iSDClkPin, false);
        sleep_us_high_power(SD_SLEEP);
        bool bBit0 = gpio_get(pPins->iSDDat0Pin);
        bool bBit1 = gpio_get(pPins->iSDDat1Pin);
        bool bBit2 = gpio_get(pPins->iSDDat2Pin);
        bool bBit3 = gpio_get(pPins->iSDDat3Pin);
        gpio_put(pPins->iSDClkPin, true);
        sleep_us_high_power(SD_SLEEP);
        if (!bBit3 && !bBit2 && !bBit1 && !bBit0)
        {
            bCardTransmit = true;
            break;
        }
    }

    return bCardTransmit;
}


bool receive_data_wide(SSDCardPins* pPins, int iExpectedBytes, uint8_t* pvData)
{
    uint16_t    uiCRC16Read;
    uint8_t     aCRC[8];
    uint8_t     aUnstripedData[512];  //Maximum allowed block size is 2048.
    uint8_t     aUnstripedCRC[2];
    uint16_t    uiExpectedCRC16;

    bool bCardTransmit = wait_for_transmit_wide(pPins);

    if (bCardTransmit)
    {
        bool bValidResponse = read_data_wide(pPins, iExpectedBytes, pvData);
        if (bValidResponse)
        {
            bValidResponse = read_data_wide(pPins, 8, aCRC);
            sd_clock_tick(pPins, 4, GPIO_OUT);
            
            if (bValidResponse)
            {
                unstripe_data(aUnstripedData, 0, iExpectedBytes, pvData);
                unstripe_data(aUnstripedCRC, 0, 8, aCRC);
                uiExpectedCRC16 = crc16(aUnstripedData, iExpectedBytes / 4);
                uiCRC16Read = (aUnstripedCRC[0] << 8) | aUnstripedCRC[1];
                if (uiExpectedCRC16 != uiCRC16Read)
                {
                    return false;
                }

                unstripe_data(aUnstripedData, 1, iExpectedBytes, pvData);
                unstripe_data(aUnstripedCRC, 1, 8, aCRC);
                uiExpectedCRC16 = crc16(aUnstripedData, iExpectedBytes / 4);
                uiCRC16Read = (aUnstripedCRC[0] << 8) | aUnstripedCRC[1];
                if (uiExpectedCRC16 != uiCRC16Read)
                {
                    return false;
                }

                unstripe_data(aUnstripedData, 2, iExpectedBytes, pvData);
                unstripe_data(aUnstripedCRC, 2, 8, aCRC);
                uiExpectedCRC16 = crc16(aUnstripedData, iExpectedBytes / 4);
                uiCRC16Read = (aUnstripedCRC[0] << 8) | aUnstripedCRC[1];
                if (uiExpectedCRC16 != uiCRC16Read)
                {
                    return false;
                }

                unstripe_data(aUnstripedData, 3, iExpectedBytes, pvData);
                unstripe_data(aUnstripedCRC, 3, 8, aCRC);
                uiExpectedCRC16 = crc16(aUnstripedData, iExpectedBytes / 4);
                uiCRC16Read = (aUnstripedCRC[0] << 8) | aUnstripedCRC[1];
                if (uiExpectedCRC16 == uiCRC16Read)
                {
                    return true;
                }
            }
        }
    }
    return false;
}


bool sd_cmd17_read_single_block_wide(SSDCardPins* pPins, int iBlock, int iExpectedBytes, uint8_t* pvData, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 17, iBlock);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 17)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                bResult = receive_data_wide(pPins, iExpectedBytes, pvData);
                return bResult;
            }
        }
    }
    return false;
}


bool sd_cmd_23_set_block_count(SSDCardPins* pPins, int iCount, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 23, iCount);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 23)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);
                return true;
            }
        }
    }
    return false;
}


bool sd_cmd18_read_multiple_blocks_wide(SSDCardPins* pPins, int iStartBlock, int iExpectedBlocks, int iBlockLength, uint8_t* pvData, SSDCardStatus* pStatus)
{
    uint8_t aCommand[6];
    uint8_t aResponse[6];
    bool    bResult;

    build_command(aCommand, 18, iStartBlock);
    send_command(pPins, aCommand);
    sd_clock_tick(pPins, 4, GPIO_IN);

    bResult = receive_response(pPins, 6, aResponse);
    if (bResult)
    {
        SSDResponseR1* pResponse = (SSDResponseR1*)aResponse;
        if (pResponse->uiCmd == 18)
        {
            uint8_t uiExpectedCRC = crc7(aResponse, 5);
            uiExpectedCRC <<= 1;
            uiExpectedCRC |= 1;
            if (pResponse->uiCrc7 == uiExpectedCRC)
            {
                calculate_status(pStatus, pResponse);

                for (int iCount = 0; iCount < iExpectedBlocks; iCount++)
                {
                    bResult = receive_data_wide(pPins, iBlockLength, pvData);
                    if (!bResult)
                    {
                        return false;
                    }
                    pvData = pvData + iBlockLength;
                }
                return true;
            }
        }
    }
    return false;
}

