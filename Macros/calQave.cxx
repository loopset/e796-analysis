#include "ActLine.h"
#include "ActMergerData.h"
#include "ActSRIM.h"
#include "ActUtils.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TF1.h"
#include "TMath.h"
#include "TROOT.h"
#include "TString.h"


void calQave()
{
    // Using 20O(X,p) at front
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"PID_Tree", "../PostAnalysis/RootFiles/Pipe1/tree_20O_2H_1H_front.root"};

    // SRIM
    auto* srim {new ActPhysics::SRIM};
    srim->ReadTable("p", "../Calibrations/SRIMData/raw/1H_952mb_mixture.txt");
    auto def {df.Define("DeltaETPC",
                        [&](ActRoot::MergerData& d)
                        {
                            auto esil {d.fSilEs.front()};
                            auto theta {d.fThetaLight * TMath::DegToRad()};
                            auto phi {d.fPhiLight * TMath::DegToRad()};
                            ROOT::Math::XYZVector dir {TMath::Cos(theta), TMath::Sin(theta) * TMath::Sin(phi),
                                                       TMath::Sin(theta) * TMath::Cos(phi)};
                            ActRoot::Line line {d.fRP, ActRoot::CastXYZVector<float>(dir), 0};
                            auto bp {line.MoveToX(256)};
                            auto bpsp {(d.fSP - bp).R()};
                            auto rpbp {(d.fRP - bp).R()};
                            // SP to BP
                            auto TatBP {srim->EvalInitialEnergy("p", esil, bpsp)};
                            // BP to RP
                            auto TatRP {srim->EvalInitialEnergy("p", TatBP, rpbp)};
                            return (TatRP - TatBP) * 1e3 / rpbp;
                        },
                        {"MergerData"})};

    auto hCal {def.Histo2D({"hCal", "Cal;Qave [au];#DeltaE_{TPC} [keV / mm]", 400, 0, 5000, 400, 0, 10}, "fQave",
                           "DeltaETPC")};
    auto hProf {def.Profile1D({"hProf", "Cal;Qave [au];#DeltaE_{TPC} [keV / mm]", 400, 0, 5000}, "fQave", "DeltaETPC")};
    // Fit it
    TString func {"pol1"};
    hProf->Fit(func, "0");

    // Draw
    auto* c0 {new TCanvas {"c0", "Qave calibration"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hCal->DrawClone("colz");
    c0->cd(2);
    hProf->DrawClone("histe");
    hProf->GetFunction(func)->DrawClone("same");
}
