#include "ActInputIterator.h"
void test()
{
    ActRoot::InputWrapper w {"./configs/filter.runs"};
    w.GoNext();
    w.GetIt().Print();
    w.GetTPCData()->Print();
}
