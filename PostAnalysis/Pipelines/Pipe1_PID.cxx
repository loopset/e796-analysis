#ifndef Pipe1_PID_cxx
#define Pipe1_PID_cxx

#include "ActColors.h"
#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"

#include <iostream>
#include <string>

#include "../../Selector/Selector.h"
#include "../Gates.cxx"
#include "../HistConfig.h"
#include "../Utils.cxx"

void Pipe1_PID(const std::string& beam, const std::string& target, const std::string& light, bool isEl)
{
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"/media/Data/E796v2/configs/data.conf"};
    auto chain {datman.GetChain()};
    ROOT::RDataFrame df {*chain};

    // Apply cuts
    auto* sm {E796Utils::GetEffSilMatrix(target, light)};
    ROOT::RDF::RNode vetoed {df};
    if(isEl)
    {
        vetoed = vetoed.Filter(
            [&](const ActRoot::MergerData& d)
            {
                bool isL0 {E796Gates::left0(d)};
                if(!isL0)
                    return isL0;
                bool isInMatrix {sm->IsInside(d.fSilNs.front(), d.fSP.X(), d.fSP.Z())};
                return isInMatrix;
            },
            {"MergerData"});
    }
    else
    {
        // Build extra lambda to apply silicon matrix
        auto veto {[&](const ActRoot::MergerData& d)
                   {
                       bool isF0 {E796Gates::front0(d)};
                       if(!isF0)
                           return isF0;
                       bool isVetoed {sm->IsInside(d.fSilNs.front(), d.fSP.Y(), d.fSP.Z())};
                       return isVetoed;
                   }};
        vetoed = vetoed.Filter(veto, {"MergerData"});
    }

    // Book histograms
    auto hPID {vetoed.Define("ESil0", "fSilEs.front()").Histo2D(HistConfig::PID, "ESil0", "fQave")};
    auto hSP {
        vetoed.Histo2D(HistConfig::SP, (isEl) ? "fSP.fCoordinates.fX" : "fSP.fCoordinates.fY", "fSP.fCoordinates.fZ")};

    auto auxfile {TString::Format("/media/Data/E796v2/Publications/pid/Inputs/pid_%s.root", isEl ? "side" : "front")};
    // if file doesnt exist
    if(gSystem->AccessPathName(auxfile))
    {
        std::cout << BOLDGREEN << "Saving PID tree : " << auxfile << RESET << '\n';
        vetoed.Define("ESil0", "fSilEs.front()").Snapshot(
            "PID_Tree",
            TString::Format("/media/Data/E796v2/Publications/pid/Inputs/pid_%s.root", isEl ? "side" : "front").Data(),
            {"ESil0", "fQave"});
    }
    // // Write entries
    // std::cout << "Writing : " << vetoed.Count().GetValue() << " entries" << '\n';
    // std::ofstream streamer {
    //     TString::Format("./Entries/entries_%s_%s_%s_befRPDist.dat", beam.c_str(), target.c_str(), light.c_str())};
    // vetoed.Foreach([&](const ActRoot::MergerData& d) { streamer << d.fRun << " " << d.fEntry << '\n'; },
    //                {"MergerData"});
    // streamer.close();


    // Read PID cut
    ActRoot::CutsManager<std::string> cut;
    TString pidfile {};
    if(isEl)
        pidfile = TString::Format("./Cuts/LightPID/pid_%s_side.root", light.c_str());
    else
        pidfile = TString::Format("./Cuts/LightPID/pid_%s.root", light.c_str());
    cut.ReadCut(light, pidfile);
    std::cout << BOLDCYAN << "Reading light PID in : " << pidfile << RESET << '\n';

    if(cut.GetCut(light))
    {
        // Filter
        auto pid {vetoed.Filter([&](const ActRoot::MergerData& d)
                                { return cut.IsInside(light, d.fSilEs.front(), d.fQave); }, {"MergerData"})};
        auto filename {gSelector->GetAnaFile(1, beam, target, light, false)};
        pid.Snapshot("PID_Tree", filename);
    }
    // Write to file
    // hPID->SaveAs(TString::Format("/media/Data/E796v2/Publications/pid/Inputs/pid_%s.root", isEl ? "side" : "front"));

    // plotting
    auto* c10 {new TCanvas("c10", "Pipe1 canvas 0")};
    c10->DivideSquare(2);
    c10->cd(1);
    hPID->DrawClone("colz");
    cut.SetLineAttributes(light, kMagenta, 2);
    cut.DrawAll();
    c10->cd(2);
    hSP->DrawClone("colz");
    if(sm)
        sm->Draw();
}
#endif
