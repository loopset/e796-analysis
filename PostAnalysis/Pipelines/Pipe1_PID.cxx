#include "ActJoinData.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"

#include <string>

#include "../HistConfig.h"

void Pipe1_PID(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::JoinData in {"/media/Data/E796v2/configs/merger.runs", "/media/Data/E796v2/configs/corrections.runs"};
    ROOT::RDataFrame df {*in.Get()};

    // Apply cuts
    ActPhysics::SilMatrix sm;
    ROOT::RDF::RNode vetoed {df};
    if(isSide)
    {
        auto veto {[&](const ActRoot::MergerData& d)
                   {
                       // Check size
                       bool hasSize {d.fSilLayers.size() == 1};
                       if(!hasSize)
                           return hasSize;
                       bool isL0 {d.fSilLayers.front() == "l0"};
                       return isL0;
                   }};
        vetoed = vetoed.Filter(veto, {"MergerData"});
    }
    else
    {
        // Read matrix
        sm.Read("/media/Data/E796v2/Macros/silmatrix.root");
        auto veto {[&](const ActRoot::MergerData& d)
                   {
                       // Check size
                       bool hasSize {d.fSilLayers.size() == 1};
                       if(!hasSize)
                           return hasSize;
                       bool isF0 {d.fSilLayers.front() == "f0"};
                       if(!isF0)
                           return isF0;
                       bool isVetoed {sm.IsInside(d.fSilNs.front(), d.fSP.Y(), d.fSP.Z())};
                       return isVetoed;
                   }};
        vetoed = vetoed.Filter(veto, {"MergerData"});
    }

    // Book histograms
    auto hPID {vetoed.Define("ESil0", "fSilEs.front()").Histo2D(HistConfig::PID, "ESil0", "fQave")};
    auto hSP {vetoed.Histo2D(HistConfig::SP, "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    // plotting
    auto* c10 {new TCanvas("c10", "Pipe1 canvas 0")};
    c10->DivideSquare(2);
    c10->cd(1);
    hPID->DrawClone("colz");
    c10->cd(2);
    hSP->DrawClone("colz");
}
