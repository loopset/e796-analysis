#include "PhysSM.h"
void test()
{
    PhysUtils::ModelParser parser {{"./Inputs/SM/log_O20_O19_ysox_tr_j0p_m1p.txt"}};
    parser.ShiftEx();
}
