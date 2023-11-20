#include "ActInputData.h"
#include "ActDetectorManager.h"
#include "ActEventPainter.h"

#include <iostream>
#include <memory>

auto* input {new ActRoot::InputData("./configs/cluster.runs")};

auto* detman {new ActRoot::DetectorManager("./configs/e796.detector")};

void Cluster()
{
    auto painter {new ActRoot::EventPainter(gClient->GetRoot(), 800, 600)};
    painter->SetPainterAndData("./configs/e796.detector", input);
    painter->SetDetMan(detman);
}
