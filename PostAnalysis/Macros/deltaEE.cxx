#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActTypes.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TString.h"
#include "TSystem.h"

#include <string>

#include "../../Selector/Selector.h"
#include "../HistConfig.h"

void deltaEE()
{
    std::string target {"2H"};
    std::string light {"3H"};
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"/media/Data/E796v2/configs/data.conf"};
    // datman.SetRuns(155, 225);
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    // Get two sil pid
    auto two {df.Filter(
                    [](const ActRoot::MergerData& d)
                    {
                        if(d.fSilLayers.size() == 2)
                        {
                            if(d.fSilLayers[0] == "f0" && d.fSilLayers[1] == "f1")
                                return true;
                        }
                        return false;
                    },
                    {"MergerData"})
                  .Define("E0", "fSilEs[0]")
                  .Define("E1", "fSilEs[1]")};

    // Book PID histogram
    auto hPID {two.Histo2D(HistConfig::PIDTwo, "E1", "E0")};
    auto hTheta {two.Histo1D("fThetaLight")};
    // Apply cut
    ActRoot::CutsManager<int> cut;
    auto name {TString::Format("./pid_twosils_%s.root", light.c_str())};
    if(!gSystem->AccessPathName(name))
    {
        cut.ReadCut(0, name.Data());
        auto gated {two.Filter([&](float e0, float e1) { return cut.IsInside(0, e1, e0); }, {"E0", "E1"})};
        auto outname {TString::Format("./Outputs/tree_twosils_%s_%s.root", target.c_str(), light.c_str())};
        gated.Snapshot("PID_Tree", outname.Data());
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Two PID canvas"}};
    c0->DivideSquare(2);
    c0->cd(1);
    hPID->DrawClone("colz");
    cut.DrawAll();
}
