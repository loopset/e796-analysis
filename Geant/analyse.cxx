#include "ActKinematics.h"
#include "ActSRIM.h"

#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "ROOT/RVec.hxx"

#include "TCanvas.h"
#include "TF1.h"
#include "TMath.h"
#include "TRandom.h"
#include "TStyle.h"

#include "Math/Point3D.h"
#include "Math/Vector3D.h"
#include "Math/Vector3Dfwd.h"

#include "../PostAnalysis/HistConfig.h"

void analyse()
{
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame df {"ActGeant", "./Outputs/simu.root"};

    // SRIM
    ActPhysics::SRIM srim;
    // srim.ReadTable("light", "../Calibrations/SRIMData/raw/3H_952mb_mixture.txt");
    srim.ReadTable("light", "./Outputs/dedx/table_triton_GasMixture.txt", false);
    srim.Draw();

    // Kinematics
    ActPhysics::Kinematics kin {"20O(d,t)@700|0"};
    std::vector<ActPhysics::Kinematics> vkins {df.GetNSlots()};
    for(auto& k : vkins)
        k = kin;

    // Gate on events that reach silicons
    auto gated {df.Filter("SilIdx >= 0")};
    // Define variables
    auto def {gated
                  .Define("TL",
                          [](ROOT::RVecD& tpc, ROOT::RVecD& sil)
                          {
                              ROOT::Math::XYZPointD ini {tpc[0], tpc[1], tpc[2]};
                              ROOT::Math::XYZPointD end {sil[0], sil[1], sil[2]};
                              return (ini - end).R();
                          },
                          {"TPCIni", "SilIni"})
                  .Define("thetaLab",
                          [](ROOT::RVecD& tpc, ROOT::RVecD& sil)
                          {
                              ROOT::Math::XYZPointD ini {tpc[0], tpc[1], tpc[2]};
                              ROOT::Math::XYZPointD end {sil[0], sil[1], sil[2]};
                              auto dir {end - ini};
                              ROOT::Math::XYZVectorD beam {1, 0, 0};
                              auto dot {dir.Unit().Dot(beam.Unit())};
                              return TMath::ACos(dot) * TMath::RadToDeg();
                          },
                          {"TPCIni", "SilIni"})
                  .Define("EVertex",
                          [&](double SilDeltaE, double TL)
                          {
                              // SilDeltaE = gRandom->Gaus(SilDeltaE, 0.25 * TMath::Sqrt(SilDeltaE / 5.5));
                              return srim.EvalInitialEnergy("light", SilDeltaE, TL);
                          },
                          {"SilDeltaE", "TL"})
                  .DefineSlot("Ex",
                              [&vkins](unsigned int slot, double theta, double e, double ebeam)
                              {
                                  auto& k {vkins[slot]};
                                  k.SetBeamEnergy(ebeam);
                                  return k.ReconstructExcitationEnergy(e, theta * TMath::DegToRad());
                              },
                              {"thetaLab", "EVertex", "EBeam"})};

    // Book histograms
    auto hKin {df.Histo2D(HistConfig::Kin, "theta3", "T3")};
    auto hKinRec {def.Histo2D(HistConfig::KinSimu, "thetaLab", "EVertex")};
    auto hEx {def.Filter("thetaLab <= 40").Histo1D(HistConfig::Ex, "Ex")};
    // Fit to a gaussian
    hEx->Fit("gaus", "0QR+");
    hEx->GetFunction("gaus")->ResetBit(TF1::kNotDraw);

    gStyle->SetOptFit();
    auto* c0 {new TCanvas {"c0", "Analysis of simulation"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hKin->DrawClone("colz");
    kin.GetKinematicLine3()->Draw("l");
    c0->cd(2);
    hKinRec->DrawClone("colz");
    kin.GetKinematicLine3()->Draw("l");
    c0->cd(3);
    hEx->DrawClone();

    srim.Draw();
}