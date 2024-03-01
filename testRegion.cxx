#include "ActMultiRegion.h"
#include "ActOptions.h"

void testRegion()
{
    ActRoot::Options::GetInstance()->Print();
    std::cout << "Config : " << ActRoot::Options::GetInstance()->GetConfigDir() << '\n';

    ActAlgorithm::MultiRegion mr;
    mr.ReadConfiguration();
    mr.Print();
}
