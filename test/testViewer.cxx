#include "ActEventPainter.h"

void testViewer()
{
    auto painter {new ActRoot::EventPainter(gClient->GetRoot(), 800, 600)};
    painter->SetDataAndDetector("../configs/e796.detector", "../configs/merger.runs");
    painter->SetIsVerbose();
}
