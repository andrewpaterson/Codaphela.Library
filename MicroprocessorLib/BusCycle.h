#ifndef __BUS_CYCLE_H__
#define __BUS_CYCLE_H__
#include "AddressOffsetArray.h"
#include "OperationArray.h"
#include "DataOperation.h"


class CW65C816;
class CBusCycle
{
protected:
    CAddressOffsetArray     mapcAddressOffsets;
    COperationArray         mapcOperations;
    CDataOperation*         mpcDataOperation;
    int16                   miCycle;

public:
    void Init(AddressOffset[] addressOffsets, Operation... operations)
    {
        this->addressOffsets = Arrays.asList(addressOffsets);
        this->operations = Arrays.asList(operations);
        this->cycle = -1;

        validate();

        for (Operation operation : operations)
        {
            if (operation.isData())
            {
                dataOperation = (DataOperation)operation;
            }
        }
    }

private:
    void validate(void)
    {
        int dataBusOperation = 0;
        for (Operation operation : this->operations)
        {
            if (operation.isData())
            {
                dataBusOperation++;
            }
        }

        if (dataBusOperation != 1)
        {
            throw new SimulatorException("Exactly [1] data bus operation must be specified in a bus cycle.");
        }

        if (addressOffsets.size() == 0)
        {
            throw new SimulatorException("At least [1] address offset must be specified in a bus cycle.");
        }
    }

public:
    Address GetAddress(CW65C816* pcCpu)
    {
        return AddressOffset.GetAddress(pcCpu, addressOffsets);
    }

    public:
DataOperation getDataOperation()
    {
        for (Operation operation : operations)
        {
            if (operation.isData())
            {
                return (DataOperation)operation;
            }
        }
        return null;
    }

    public:
int getDone8()
    {
        int done8 = 0;
        for (Operation operation : operations)
        {
            done8 += operation.getDone8();
        }
        return done8;
    }

    public:
int getDone16()
    {
        int done16 = 0;
        for (Operation operation : operations)
        {
            done16 += operation.getDone16();
        }
        return done16;
    }

bool mustExecute(CW65C816* pcCPU)
    {
        bool mustExecute = true;
        for (Operation operation : operations)
        {
            if (!operation.mustExecute(cpu))
            {
                mustExecute = false;
            }
        }
        return mustExecute;
    }

String toAddressOffsetString()
    {
        StringBuilder string = new StringBuilder();
        bool first = true;
        for (AddressOffset addressOffset : addressOffsets)
        {
            if (!first && !(addressOffset instanceof ConstantOffset))
            {
                if (string.charAt(string.length() - 1) != ',')
                {
                    string.append("+");
                }
            }
            string.append(addressOffset.toString());

            first = false;
        }
        return string.toString();
    }

String toOperationString()
    {
        StringBuilder string = new StringBuilder();
        bool first = true;
        for (Operation operation : operations)
        {
            if (!first)
            {
                string.append(" ");
            }
            string.append(operation.toString());
            first = false;
        }
        return string.toString();
    }

String toDataString()
    {
        DataOperation operation = getDataOperation();
        return operation.toString();
    }

bool isFetchOpCode()
    {
        DataOperation dataOperation = getDataOperation();
        return dataOperation.isFetchOpCode();
    }

void setCycle(int cycle)
    {
        this->cycle = cycle;
    }

int getCycle()
    {
        return cycle;
    }

COperationArray getOperations()
    {
        return mapcOperations;
    }
};


#endif // __BUS_CYCLE_H__

