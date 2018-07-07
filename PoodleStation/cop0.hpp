#ifndef COP0_HPP
#define COP0_HPP

#include <cstdint>

struct Cop0_Status
{
    bool IEc;
    bool KUc;
    bool IEp;
    bool KUp;
    bool IEo;
    bool KUo;
    uint8_t Im;
    bool IsC;
    bool bev;
};

struct Cop0_Cause
{
    uint8_t code;
    uint8_t int_pending;
    bool bd;
};

class Cop0
{
    public:
        Cop0_Status status;
        Cop0_Cause cause;
        uint32_t EPC;
        Cop0();

        void reset();

        uint32_t mfc(int cop_reg);
        void mtc(int cop_reg, uint32_t value);
};

#endif // COP0_HPP
