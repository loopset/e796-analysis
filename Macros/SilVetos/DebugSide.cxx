#include "TCanvas.h"
#include "TFile.h"
#include "TH2.h"
#include "ActSilSpecs.h"

#include <memory>
void DebugSide()
{
    // Read histogram
    auto f {std::make_unique<TFile>("./Outputs/side_sps.root")};
    auto h {f->Get<TH2D>("hSP")};
    h->SetDirectory(nullptr);

    // Read specs
    ActPhysics::SilSpecs specs;
    specs.ReadFile("../../configs/detailedSilicons.conf");
    auto side {specs.GetLayer("l0").GetSilMatrix()->Clone()};
    side->MoveZTo(160, {4, 5});

    // Draw
    auto* c0 {new TCanvas {"c0", "Debug side canvas"}};
    h->Draw("colz");
    side->Draw();
}
