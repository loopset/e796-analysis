#include "ActClIMB.h"
#include "ActDataManager.h"
#include "ActInputIterator.h"
#include "ActMultiRegion.h"
#include "ActOptions.h"
#include "ActTypes.h"

#include <memory>

void testRegion()
{
    ActRoot::Options::GetInstance()->Print();
    std::cout << "Config : " << ActRoot::Options::GetInstance()->GetConfigDir() << '\n';

    // Read data
    ActRoot::DataManager datman {ActRoot::ModeType::EFilter};
    datman.ReadDataFile(ActRoot::Options::GetInstance()->GetConfigDir() + "data.conf");
    auto input {datman.GetInput()};
    // wrapper
    ActRoot::InputWrapper wrapper {&input};

    auto cl {std::make_shared<ActAlgorithm::ClIMB>()};
    cl->SetMinPoint(15);

    ActAlgorithm::MultiRegion mr;
    mr.ReadConfiguration();
    mr.SetClusterPtr(cl);
    mr.Print();

    while(wrapper.GoNext())
    {
        // wrapper.GetIt().Print();
        // Set data
        mr.SetTPCData(wrapper.GetTPCData());
        // Run!
        mr.Run();
    }
}
