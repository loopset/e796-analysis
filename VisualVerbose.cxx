#include "ActEventPainter.h"

void VisualVerbose()
{
    auto painter {new ActRoot::EventPainter(gClient->GetRoot(), 800, 600)};
    painter->SetDetectorAndData("./configs/e796.detector", "./configs/merger.runs");
    painter->SetIsVerbose();
}
