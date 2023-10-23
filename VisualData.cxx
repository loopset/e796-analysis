#include "ActInputData.h"
#include "ActEventPainter.h"

#include "TGClient.h"
#include "TApplication.h"

#include <iostream>

//Data has to be defined outside macro's main function
//otherwise it will be emptied after {} (window remains working)
ActRoot::InputData input {};
void VisualData()
{
    //TApplication app {"visual", nullptr, nullptr};
    input.ReadConfiguration("./configs/visual.runs");

    auto painter {new ActRoot::EventPainter(gClient->GetRoot(), 800, 600)};
    painter->SetPainterAndData("./configs/e796.detector", &input);

    //app.Run();
    //std::cout<<"Exited"<<'\n';
}
