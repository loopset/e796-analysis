#include "ActDetectorManager.h"
#include "ActEventPainter.h"
#include "ActHistogramPainter.h"
#include "ActInputIterator.h"
#include "ActOptions.h"
#include "ActTypes.h"

#include "TGClient.h"
// #include "TRint.h"
#include "TApplication.h"
#include "TROOT.h"

// Define globals
ActRoot::InputWrapper* in {};
ActRoot::DetectorManager* detman {};
ActRoot::HistogramPainter* server {};
ActRoot::EventPainter* painter {};

void actplot()
{
    ActRoot::Options::GetInstance(0, nullptr);
    ActRoot::Options::GetInstance()->SetMode(ActRoot::ModeType::EVisual);
    ActRoot::Options::GetInstance()->Print();


    // Init input wrapper
    in = new ActRoot::InputWrapper {"./configs/filter.runs", false};

    // Detector manager
    detman = new ActRoot::DetectorManager {ActRoot::Options::GetInstance()->GetMode()};
    detman->ReadDetectorFile("./configs/e796.detector");
    detman->SendWrapperData(in);

    server = new ActRoot::HistogramPainter;
    server->SendInputWrapper(in);
    server->SendParameters(detman);

    painter = new ActRoot::EventPainter {gClient->GetRoot(), 800, 600};
    server->SendCanvas(painter->GetCanvasPtr());
    painter->SendHistogramServer(server);
    painter->SendInputWrapper(in);
    painter->SendDetectorManager(detman);
}
