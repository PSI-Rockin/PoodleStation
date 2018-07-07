#ifndef DMA_HPP
#define DMA_HPP
#include <cstdint>

struct DICR
{
    bool force_IRQ;
    bool master_IRQ_enable;
    uint8_t MASK, STAT;
};

class DMA
{
    private:
        uint32_t PCR;
        DICR ICR;
    public:
        DMA();

        void reset();

        uint32_t read_PCR();
        uint32_t read_ICR();

        void write_PCR(uint32_t value);
        void write_ICR(uint32_t value);
};

#endif // DMA_HPP
