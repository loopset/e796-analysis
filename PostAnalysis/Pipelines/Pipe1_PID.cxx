#ifndef Pipe1_PID_cxx
#define Pipe1_PID_cxx

#include "ActColors.h"
#include "ActCutsManager.h"
#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActSilMatrix.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TROOT.h"
#include "TString.h"

#include <iostream>
#include <string>

#include "../Gates.cxx"
#include "../HistConfig.h"
#include "../Utils.cxx"

void Pipe1_PID(const std::string& beam, const std::string& target, const std::string& light, bool isSide)
{
    ROOT::EnableImplicitMT();
    // Read data
    ActRoot::DataManager datman {"/media/Data/E796v2/configs/data.conf"};
    auto chain {datman.GetJoinedData()};
    ROOT::RDataFrame df {*chain};

    // Apply cuts
    ActPhysics::SilMatrix* sm {};
    ROOT::RDF::RNode vetoed {df};
    if(isSide)
        vetoed = vetoed.Filter(E796Gates::left0, {"MergerData"});
    else
    {
        // Read matrix
        sm = E796Utils::GetEffSilMatrix(light);
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
    auto hSP {vetoed.Histo2D(HistConfig::SP, (isSide) ? "fSP.fCoordinates.fX" : "fSP.fCoordinates.fY",
                             "fSP.fCoordinates.fZ")};

    // Read PID cut
    ActRoot::CutsManager<std::string> cut;
    TString pidfile {};
    if(isSide)
        pidfile = TString::Format("./Cuts/pid_%s_side.root", light.c_str());
    else
        pidfile = TString::Format("./Cuts/pid_%s.root", light.c_str());
    cut.ReadCut(light, pidfile);
    std::cout << BOLDCYAN << "Reading light PID in : " << pidfile << RESET << '\n';

    if(cut.GetCut(light))
    {
        // Filter
        auto pid {vetoed.Filter([&](const ActRoot::MergerData& d)
                                { return cut.IsInside(light, d.fSilEs.front(), d.fQave); },
                                {"MergerData"})};
        auto filename {E796Utils::GetFileName(1, beam, target, light, isSide)};
        std::cout << BOLDCYAN << "Saving PID_Tree in file : " << filename << '\n';
        pid.Snapshot("PID_Tree", filename);

        // // Write
        // std::ofstream streamer {"./Hes_veto.dat"};
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
    sm->Draw();
}
#endif
