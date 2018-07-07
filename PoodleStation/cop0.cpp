#include <cstdio>
#include <cstdlib>
#include "cop0.hpp"

Cop0::Cop0()
{

}

void Cop0::reset()
{
    status.IEc = false;
    status.KUc = false;
    status.IEp = false;
    status.KUp = false;
    status.IEo = false;
    status.KUo = false;
    status.Im = 0;
    status.IsC = false;
    status.bev = true;

    cause.code = 0;
    cause.bd = false;
    cause.int_pending = 0;
}

uint32_t Cop0::mfc(int cop_reg)
{
    //printf("[COP0] MFC: Read from %d\n", cop_reg);
    switch (cop_reg)
    {
        case 12:
        {
            uint32_t reg = 0;
            reg |= status.IEc;
            reg |= status.KUc << 1;
            reg |= status.IEp << 2;
            reg |= status.KUp << 3;
            reg |= status.IEo << 4;
            reg |= status.KUo << 5;
            reg |= status.Im << 8;
            reg |= status.IsC << 16;
            reg |= status.bev << 22;
            return reg;
        }
        case 13:
        {
            uint32_t reg = 0;
            reg |= cause.code << 2;
            reg |= cause.int_pending << 8;
            reg |= cause.bd << 31;
            return reg;
        }
        case 14:
            return EPC;
        case 15:
            return 0x0;
        default:
            printf("[COP0] MFC: Unknown cop_reg %d\n", cop_reg);
            exit(1);
    }
}

void Cop0::mtc(int cop_reg, uint32_t value)
{
    //printf("[COP0] MTC: Write to %d of $%08X\n", cop_reg, value);
    switch (cop_reg)
    {
        case 12:
            status.IEc = value & 1;
            status.KUc = value & (1 << 1);
            status.IEp = value & (1 << 2);
            status.KUp = value & (1 << 3);
            status.IEo = value & (1 << 4);
            status.KUo = value & (1 << 5);
            status.Im = (value >> 8) & 0xFF;
            status.IsC = value & (1 << 16);
            status.bev = value & (1 << 22);
            break;
        default:
            printf("[IOP COP0] MTC: Unknown cop_reg %d\n", cop_reg);
            //exit(1);
    }
}
