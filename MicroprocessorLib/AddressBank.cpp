#include "AddressBank.h"


uint16 GetOffset(CW65C816* pcCPU)
{
    return 0;
}

/*@Override*/
        public:
            int GetBank(CW65C816* pcCPU)
            {
                return pcCPU->GetState().GetAddress().GetBank();
            }

            /*@Override*/
        public:
            void    Print(CChars* psz)
            {
                return "AAB,";
            }


