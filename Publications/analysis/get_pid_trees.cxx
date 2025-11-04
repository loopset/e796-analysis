#include "ActDataManager.h"
#include "ActMergerData.h"
#include "ActTypes.h"

#include "ROOT/RDFHelpers.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"

#include <initializer_list>
#include <string>

#include "/media/Data/E796v2/PostAnalysis/Gates.cxx"
#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/PostAnalysis/Utils.cxx"

void get_pid_trees()
{
    ActRoot::DataManager dataman {"../../configs/data.conf", ActRoot::ModeType::EMerge};
    auto chain {dataman.GetChain()};

    ActRoot::DataManager dataman1 {"../../configs/data.conf", ActRoot::ModeType::ECorrect};
    auto chain1 {dataman1.GetChain()};

    ROOT::EnableImplicitMT();

    ROOT::RDataFrame df {*chain};
    ROOT::RDataFrame df1 {*chain1};

    // Antiveto SM
    auto* sm {E796Utils::GetAntiVetoMatrix()};

    // Apply condition on layer
    auto fUncorr {df.Filter([](ActRoot::MergerData& mer) { return E796Gates::front0(mer); }, {"MergerData"})
                      .Define("ESil0", "fSilEs.front()")};
    auto fCorr {df1.Filter([](ActRoot::MergerData& mer) { return E796Gates::front0(mer); }, {"MergerData"})
                    .Define("ESil0", "fSilEs.front()")};
    auto gatedSm {fUncorr.Filter([&](ActRoot::MergerData& mer)
                                 { return sm->IsInside(mer.fSilNs.front(), mer.fSP.Y(), mer.fSP.Z()); },
                                 {"MergerData"})};
    // Book histos
    auto hPID {fUncorr.Histo2D(HistConfig::PID, "ESil0", "fQave")};
    auto hPIDSm {gatedSm.Histo2D(HistConfig::PID, "ESil0", "fQave")};
    auto hPID1 {fCorr.Histo2D(HistConfig::PID, "ESil0", "fQave")};

    // Process both
    ROOT::RDF::RunGraphs({hPID, hPIDSm, hPID1});

    // Write
    std::initializer_list<std::string> cols {"ESil0", "fQave"};
    fUncorr.Snapshot("PID_Tree", "./Inputs/pid_nocorr_noveto.root", cols);
    gatedSm.Snapshot("PID_Tree", "./Inputs/pid_nocorr_veto.root", cols);

    // Draw
    auto* c0 {new TCanvas {"c0", "PID canvas"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hPID->SetTitle("PID uncorr + no veto");
    hPID->DrawClone("colz");
    c0->cd(2);
    hPID1->SetTitle("PID corr + no veto");
    hPID1->DrawClone("colz");
    c0->cd(3);
    hPIDSm->SetTitle("PID uncorr + veto");
    hPIDSm->DrawClone("colz");
}
