#ifndef CDROM_HPP
#define CDROM_HPP
#include <cstdint>

class Emulator;

class CDROM
{
    private:
        Emulator* e;
        int reg_index;

        uint8_t params[16];
        uint8_t response[16];
        int param_count;
        int responses_read;
        int response_size;
        uint8_t int_enable;
        uint8_t int_flag;
        uint8_t second_int_flag;
        int second_response_size;

        bool busy;
        uint8_t cmd;
        int cycles_left;

        void exec_command();
        void exec_test();
        void int_check(uint8_t interrupt);
    public:
        CDROM(Emulator* e);

        void reset();
        void run();

        uint8_t read_reg1();
        uint8_t read_reg2();
        uint8_t read_reg3();
        uint8_t read_reg4();

        void write_reg1(uint8_t value);
        void write_reg2(uint8_t value);
        void write_reg3(uint8_t value);
        void write_reg4(uint8_t value);
};

#endif // CDROM_HPP
