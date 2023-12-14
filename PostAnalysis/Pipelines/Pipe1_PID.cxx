#include "ActColors.h"
#include "ActCutsManager.h"
#include "ActJoinData.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include <fstream>
#include <iostream>
#include <string>

#include "../HistConfig.h"
#include "../Utils.cxx"

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
        sm.Read("/media/Data/E796v2/Macros/antiveto_matrix.root");
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
    auto hSP {vetoed.Histo2D(HistConfig::SP, (isSide) ? "fSP.fCoordinates.fX" : "fSP.fCoordinates.fY",
                             "fSP.fCoordinates.fZ")};

    // Read PID cut
    ActRoot::CutsManager<std::string> cut;
    TString srimfile {};
    if(isSide)
        srimfile = TString::Format("./Cuts/pid_%s_side.root", light.c_str());
    else
        srimfile = TString::Format("./Cuts/pid_%s.root", light.c_str());
    cut.ReadCut(light, srimfile);

    if(cut.GetCut(light))
    {
        // Filter
        auto pid {vetoed.Filter([&](const ActRoot::MergerData& d)
                                { return cut.IsInside(light, d.fSilEs.front(), d.fQave); },
                                {"MergerData"})};
        auto filename {E796Utils::GetFileName(1, beam, target, light, isSide)};
        std::cout << BOLDGREEN << "Saving PID_Tree in file : " << filename << '\n';
        pid.Snapshot("PID_Tree", filename);

        // // Write
        // std::ofstream streamer {"./debug_2H.dat"};
        // pid.Foreach([&](const ActRoot::MergerData& d) { streamer << d.fRun << " " << d.fEntry << '\n'; },
        //             {"MergerData"});
        // streamer.close();
    }

    // plotting
    auto* c10 {new TCanvas("c10", "Pipe1 canvas 0")};
    c10->DivideSquare(2);
    c10->cd(1);
    hPID->DrawClone("colz");
    cut.DrawAll();
    c10->cd(2);
    hSP->DrawClone("colz");
    sm.SetSyle();
    sm.Draw(true);
}
