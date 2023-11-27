#include "ActEventPainter.h"

#include "TApplication.h"
#include "TGClient.h"

void VisualData()
{
    auto painter {new ActRoot::EventPainter(gClient->GetRoot(), 800, 600)};
    painter->SetDetectorAndData("./configs/e796.detector", "./configs/merger.runs");
}
