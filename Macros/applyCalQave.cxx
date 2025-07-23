#include "ActMergerData.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"


void applyCalQave()
{
    // Using 20O(d,d)
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"PID_Tree", "../Publications/pid/Inputs/pid_front.root"};

    auto redef {df.Define("ReQave", [](ActRoot::MergerData& d) { return 0.2516 + 0.011593 * d.fQave; }, {"MergerData"})
                    .Define("ESil", "fSilEs.front()")};

    auto hPIDCal {redef.Histo2D({"hPID", "PID;ESil [MeV];#DeltaE_{TPC} [keV / mm]", 300, 0, 30, 400, 0, 10}, "ESil0", "ReQave")};

    // Draw
    auto* c0 {new TCanvas {"c0", "Qave calibration"}};
    c0->DivideSquare(4);
    hPIDCal->DrawClone("colz");
}
