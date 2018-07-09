#include <cstdio>
#include <cstdlib>
#include "gte.hpp"

GTE::GTE()
{

}

void GTE::ctc(int index, uint32_t value)
{
    printf("[GTE] CTC: %d, $%08X\n", index, value);
}
