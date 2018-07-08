#ifndef DMA_HPP
#define DMA_HPP
#include <cstdint>

struct DMA_Channel
{
    uint32_t addr;
    uint32_t block;

    //CHCR
    bool transfer_dir;
    bool step_back;
    bool chop;
    uint8_t sync_mode;
    uint8_t chop_dma_size;
    uint8_t chop_cpu_size;
    bool active; //is responding to DRQs
    bool busy; //is sending/receiving data

    //Internal registers
    int word_count;
    uint32_t next_addr;
};

struct DICR
{
    bool force_IRQ;
    bool master_IRQ_enable;
    uint8_t MASK, STAT;
};

class Emulator;
class GPU;

class DMA
{
    private:
        uint8_t* RAM;
        Emulator* e;
        GPU* gpu;
        DMA_Channel channels[7];
        uint32_t PCR;
        DICR ICR;

        void process_GPU();
        void process_OTC();

        void end_transfer(int index);
    public:
        DMA(Emulator* e, GPU* gpu);

        void reset(uint8_t* RAM);
        bool run();

        uint32_t read_control(int index);

        uint32_t read_PCR();
        uint32_t read_ICR();

        void write_addr(int index, uint32_t value);
        void write_block(int index, uint32_t value);
        void write_control(int index, uint32_t value);

        void write_PCR(uint32_t value);
        void write_ICR(uint32_t value);
};

#endif // DMA_HPP
