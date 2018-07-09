#include <cstdio>
#include <cstdlib>
#include "cdrom.hpp"
#include "emulator.hpp"

CDROM::CDROM(Emulator* e) : e(e)
{

}

void CDROM::reset()
{
    int_flag = 0;
    reg_index = 0;
    param_count = 0;
    busy = false;
    cycles_left = 0;
    responses_read = 0;
    cmd = 0;
}

void CDROM::run()
{
    if (busy)
    {
        cycles_left--;
        if (cycles_left <= 0)
        {
            busy = false;
            exec_command();
        }
    }
}

void CDROM::exec_command()
{
    printf("[CDROM] Executing command...\n");
    switch (cmd)
    {
        case 0x01:
            printf("[CDROM] GetStat\n");
            response[0] = 0;
            response_size = 1;
            int_check(0x3);
            break;
        case 0x19:
            printf("[CDROM] Test: $%02X\n", params[0]);
            exec_test();
            break;
        case 0x1A:
            printf("[CDROM] GetID\n");
            //Disk not in tray
            response[0] = 0;
            response[1] = 0x08;
            response[2] = 0x40;
            response[3] = 0x00;
            response[4] = 0x00;
            response[5] = 0x00;
            response[6] = 0x00;
            response[7] = 0x00;
            response[8] = 0x00;
            response_size = 1;
            second_int_flag = 0x5;
            second_response_size = 8;
            int_check(0x3);
            break;
        default:
            printf("[CDROM] Unrecognized command $%02X\n", cmd);
            exit(1);
    }
    busy = false;
}

void CDROM::exec_test()
{
    switch (params[0])
    {
        case 0x20:
            response[0] = 0x97;
            response[1] = 0x10;
            response[2] = 0x01;
            response[3] = 0xC2;
            response_size = 4;
            int_check(0x3);
            break;
        default:
            printf("[CDROM] Unrecognized test command $%02X\n", params[0]);
            exit(1);
    }
}

void CDROM::int_check(uint8_t interrupt)
{
    int_flag |= interrupt;
    if (int_flag & int_enable)
        e->request_IRQ(2);
}

uint8_t CDROM::read_reg1()
{
    uint8_t reg = reg_index;
    reg |= !param_count << 3;
    reg |= (param_count == 16) << 4;
    reg |= !response_size << 5;
    reg |= busy << 7;
    printf("[CDROM] Read reg1: $%02X\n", reg);
    return reg;
}

uint8_t CDROM::read_reg2()
{
    uint8_t value = response[responses_read];
    responses_read++;
    if (responses_read >= response_size)
    {
        if (second_int_flag && second_response_size)
        {
            response_size += second_response_size;
            second_response_size = 0;
            int_check(second_int_flag);
            second_int_flag = 0;
        }
        else
        {
            response_size = 0;
            responses_read = 0;
        }
    }
    printf("[CDROM] Read response FIFO: $%02X\n", value);
    return value;
}

uint8_t CDROM::read_reg4()
{
    printf("[CDROM] Read reg4: %d", reg_index);
    switch (reg_index)
    {
        case 0x1:
        case 0x3:
            printf("[CDROM] Int flag\n");
            return int_flag;
        default:
            exit(1);
    }
}

void CDROM::write_reg1(uint8_t value)
{
    printf("[CDROM] Write reg1: $%02X\n", value);
    reg_index = value & 0x3;
}

void CDROM::write_reg2(uint8_t value)
{
    printf("[CDROM] Write reg2: $%02X\n", value);
    switch (reg_index)
    {
        case 0x0:
            if (!busy)
            {
                printf("[CDROM] Send command $%02X\n", value);
                cmd = value;
                busy = true;
                cycles_left = 100;
                responses_read = 0;
                response_size = 0;
                second_int_flag = 0;
                second_response_size = 0;
            }
            break;
        default:
            printf("[CDROM] Unrecognized reg2 index %d\n", reg_index);
            exit(1);
    }
}

void CDROM::write_reg3(uint8_t value)
{
    printf("[CDROM] Write reg3: $%02X\n", value);
    switch (reg_index)
    {
        case 0x0:
            printf("[CDROM] Write param FIFO\n");
            params[param_count] = value;
            param_count++;
            break;
        case 0x1:
            printf("[CDROM] Int enable\n");
            int_enable = value & 0x1F;
            break;
        default:
            printf("[CDROM] Unrecognized reg3 index %d\n", reg_index);
            exit(1);
    }
}

void CDROM::write_reg4(uint8_t value)
{
    printf("[CDROM] Write reg4: $%02X\n", value);
    switch (reg_index)
    {
        case 0x1:
            printf("[CDROM] Int flag\n");
            int_flag &= ~value;
            if (value & (1 << 6)) //Reset param FIFO
                param_count = 0;
            break;
        default:
            printf("[CDROM] Unrecognized reg4 index %d\n", reg_index);
            exit(1);
    }
}
