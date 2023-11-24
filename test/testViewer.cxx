#include "ActEventPainter.h"

void testViewer()
{
    auto painter {new ActRoot::EventPainter(gClient->GetRoot(), 800, 600)};
    painter->SetDataAndDetector("../configs/merger.runs", "../configs/e796.detector");
    painter->SetIsVerbose();
}
