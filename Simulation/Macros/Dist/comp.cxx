#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDF/RInterface.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"

#include <cstdlib>
#include <string>

#include "../../../PostAnalysis/HistConfig.h"
#include "../../../Selector/Selector.h"

double Fit(TH1D* h, double ex = 0, double w = 1.5)
{
    h->Fit("gaus", "0QR+", "", ex - w, ex + w);
    auto* fit {h->GetFunction("gaus")};
    if(!fit)
        return 0;
    fit->ResetBit(TF1::kNotDraw);
    return fit->GetParameter(2);
}

void comp()
{
    auto target {gSelector->GetTarget()};
    auto light {gSelector->GetLight()};

    // Read experimental
    ROOT::EnableImplicitMT();
    // Mine
    ROOT::RDataFrame exp {"Sel_Tree", gSelector->GetAnaFile(3)};
    ROOT::RDF::RNode juan {exp};
    if(target == "2H" && light == "3H")
    {
        // Juans
        ROOT::RDataFrame aux {"yield_tree", "/media/Data/E796v2/RootFiles/Old/FitJuan/20O_dt_19O_21_Feb_23_v1.root"};
        auto applyMassCuts = [&](double AmassH, double thetaCM, double Ex)
        {
            // fits to mass gaussians
            // A = 2
            double A2mass {2.05609 - 0.0328848 * Ex + 0.00288087 * Ex * Ex};
            double S2mass {0.251386 + 0.0171722 * Ex - 0.00191928 * Ex * Ex};
            // A = 3
            double A3mass {3.16142 - 0.034846 * Ex + 0.000415012 * Ex * Ex};
            double S3mass {0.306103 - 0.00590731 * Ex - 0.000331309 * Ex * Ex};

            int kA2 {1};
            int kA3 {2};
            // Mass cut
            bool A2cut {(A2mass - kA2 * S2mass < AmassH) && (AmassH < A2mass + kA2 * S2mass)};
            bool A3cut {(A3mass - kA3 * S3mass < AmassH) && (AmassH < A3mass + kA3 * S3mass)};
            // Cut in thetaCM (only centered detectors)
            // bool thetaCMcut {4. <= thetaCM && thetaCM <= 13};
            bool thetaCMcut {true};
            // Ex less than 10 MeV
            // bool Excut {Ex < 10};
            bool Excut {true};

            return (A3cut && thetaCMcut && Excut);
        };
        juan = aux.Filter(applyMassCuts, {"Amass_Hlike", "ThetaCM", "Ex"});
    }
    else
    {
        juan = ROOT::RDataFrame {1};
        juan = juan.Define("Ex", "0.").Define("ThetaCM", "0.");
    }
    // Get Ex histograms
    auto hEx {exp.Histo1D(HistConfig::Ex, "Ex")};
    auto hExJuan {juan.Histo1D(HistConfig::Ex, "Ex")};
    // Get sigmas to gate
    double Ex {0};
    double w {1.5};
    std::string label {(Ex == 0) ? "gs" : ("Ex = " + std::to_string(Ex))};
    auto sigma {Fit(hEx.GetPtr(), Ex, w)};
    auto sigmajuan {Fit(hExJuan.GetPtr(), Ex, w)};
    // Gate on GS
    auto gsana {exp.Filter([&](double ex) { return std::abs(ex - Ex) <= 3 * sigma; }, {"Ex"})};
    auto gsjuan {juan.Filter([&](double ex) { return std::abs(ex - Ex) <= 3 * sigmajuan; }, {"Ex"})};
    auto hExGS {gsana.Histo1D(HistConfig::Ex, "Ex")};
    hExGS->SetLineColor(8);

    // Simulation
    auto filesim {gSelector->GetSimuFiles()};
    // Append phase spaces according to reaction
    if(target == "2H" && light == "3H")
    {
        // 1n and 2n PS
        filesim.push_back(gSelector->GetSimuFile("20O", "2H", "3H", 0, 1).Data());
        filesim.push_back(gSelector->GetSimuFile("20O", "2H", "3H", 0, 2).Data());
    }
    ROOT::RDataFrame simu {"SimulationTTree", filesim};
    ROOT::RDataFrame gssimu {"SimulationTTree", gSelector->GetApproxSimuFile("20O", target, light, Ex)};

    // Book histograms!
    // RPx
    auto hRPx {exp.Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
    auto hRPxGS {gsana.Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
    auto hRPxSim {simu.Histo1D(HistConfig::RPx, "RPx")};
    auto hRPxGSSim {gssimu.Histo1D(HistConfig::RPx, "RPx")};

    // ThetaCM
    auto hCM {exp.Histo1D(HistConfig::ThetaCM, "ThetaCM")};
    auto hCMJuan {juan.Histo1D(HistConfig::ThetaCM, "ThetaCM")};
    auto hCMGS {gsana.Histo1D(HistConfig::ThetaCM, "ThetaCM")};
    auto hCMGSJuan {gsjuan.Histo1D(HistConfig::ThetaCM, "ThetaCM")};
    auto hCMSim {simu.Histo1D(HistConfig::ThetaCM, "theta3CM")};
    auto hCMGSSim {gssimu.Histo1D(HistConfig::ThetaCM, "theta3CM")};

    // Normalize
    for(auto h : {hRPx, hRPxGS, hRPxSim, hRPxGSSim, hCM, hCMJuan, hCMGS, hCMGSJuan, hCMSim, hCMGSSim})
    {
        h->Scale(1. / h->Integral());
    }

    // Colors
    for(auto h : {hRPx, hRPxGS, hCM, hCMGS})
        h->SetLineColor(8);
    for(auto h : {hCMJuan, hCMGSJuan})
        h->SetLineColor(kRed);

    // Draw
    auto* c0 {new TCanvas {"c0", "Comparison canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hRPx->SetTitle("RP.X() all");
    hRPx->DrawClone("hist");
    hRPxSim->DrawClone("hist same");
    c0->cd(2);
    hRPxGS->SetTitle(("RP.X() only " + label).c_str());
    hRPxGS->DrawClone("hist");
    hRPxGSSim->DrawClone("hist same");
    c0->cd(3);
    hCM->SetTitle("#theta_{CM} all");
    hCM->DrawClone("hist");
    hCMJuan->DrawClone("hist same");
    hCMSim->DrawClone("hist same");
    c0->cd(4);
    hCMGS->SetTitle(("#theta_{CM} only " + label).c_str());
    hCMGS->DrawClone("hist");
    hCMGSJuan->DrawClone("hist same");
    hCMGSSim->DrawClone("hist same");
    c0->cd(5);
    hEx->DrawClone();
    hExGS->DrawClone("same");

    gSelector->SendToWebsite("sim_to_ana.root", c0,
                             ("c" + std::string(Ex == 0 ? "gs" : "ex") + gSelector->GetShortStr()).c_str());
}
