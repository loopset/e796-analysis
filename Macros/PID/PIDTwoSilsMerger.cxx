#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActModularData.h"
#include "ActSilData.h"
#include "ActSilSpecs.h"
#include "ActTypes.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TVirtualPad.h"

#include <string>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"
void PIDTwoSilsMerger()
{
    ActRoot::DataManager dataman {"../../configs/data.conf"};
    auto chain {dataman.GetChain()};

    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {*chain};

    // Apply sil matrix
    auto* sm {E796Utils::GetEffSilMatrix("2H", "3H")};

    // Gate on two sils
    auto gated {df.Filter(
                      [&](ActRoot::MergerData& mer)
                      {
                          if(mer.fSilLayers.size() != 2)
                              return false;
                          if(mer.fLightIdx == -1)
                              return false;
                          if(!sm->IsInside(mer.fSP.Y(), mer.fSP.Z()))
                              return false;
                          return true;
                      },
                      {"MergerData"})
                    .Define("ESil0", "fSilEs[0]")
                    .Define("ESil1", "fSilEs[1]")};

    // Get histogram
    auto hPID {gated.Histo2D(HistConfig::PIDTwo, "ESil1", "ESil0")};


    // Write to disk
    gated.Snapshot("PID_Tree", "../../Publications/pid/Inputs/pid_two.root", {"ESil0", "ESil1"});

    // Draw
    auto* c0 {new TCanvas {"c0", "Two PID canvas"}};
    gPad->SetLogz();
    hPID->DrawClone("colz");
}
