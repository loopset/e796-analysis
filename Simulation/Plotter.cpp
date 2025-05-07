#include "ActColors.h"
#include "ActKinematics.h"
#include "ActParticle.h"

#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TLine.h"
#include "TROOT.h"
#include "TString.h"
#include "TSystem.h"
#include "TVirtualPad.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/Selector/Selector.h"

void Plotter(const std::vector<double>& Exs, const std::string& beam, const std::string& target,
             const std::string& light, double T1, int neutronPS, int protonPS)
{
    ROOT::EnableImplicitMT();

    // Set if we should fit or not
    bool isPS {(neutronPS != 0 || protonPS != 0)};
    if(!isPS)
        std::cout << BOLDCYAN << "Plotting usual simulation" << RESET << '\n';
    else
        std::cout << BOLDCYAN << "Plotting a phase space" << RESET << '\n';

    // Save histograms
    std::vector<TH1D*> hsEx, hsRPx, hsCM;
    std::vector<TH2D*> hsKin, hsSP, hsRP, hsRPCM;
    std::vector<TEfficiency*> effs;
    // Iterate
    int idx {1};
    for(const auto& Ex : Exs)
    {
        auto file {gSelector->GetSimuFile(beam, target, light, Ex, neutronPS, protonPS)};
        if(gSystem->AccessPathName(file))
            continue;
        // Get DF
        ROOT::RDataFrame df("SimulationTTree", file);

        // Book histograms
        auto hEx {
            df.Histo1D(HistConfig::ChangeTitle(HistConfig::Ex, TString::Format("%s(%s, %s) Ex = %.2f", beam.c_str(),
                                                                               target.c_str(), light.c_str(), Ex)),
                       "Eex", "weight")};
        auto hKin {df.Histo2D(
            HistConfig::ChangeTitle(HistConfig::KinSimu, TString::Format("%s(%s, %s) Ex = %.2f", beam.c_str(),
                                                                         target.c_str(), light.c_str(), Ex)),
            "theta3Lab", "EVertex", "weight")};
        auto hRPx {df.Histo1D(HistConfig::RPx, "RPx")};
        hRPx->Scale(1. / hRPx->Integral());
        hRPx->SetTitle(TString::Format("%.2f MeV;RP_{x} [mm];Normalized counts", Ex));
        auto hCM {df.Histo1D(HistConfig::ThetaCM, "theta3CM")};
        hCM->Scale(1. / hCM->Integral());
        hCM->SetTitle(TString::Format("%.2f MeV;#theta_{CM} [#circ];Normalized counts", Ex));
        auto hRPCM {df.Histo2D(HistConfig::RPxThetaCM, "RPx", "theta3CM")};

        // Read directly from file
        auto* f {new TFile(file)};
        auto* eff {f->Get<TEfficiency>("eff")};
        if(!eff)
            throw std::runtime_error("Could not read TEfficiency named eff in file " + file);
        auto* hRP {f->Get<TH2D>("hRP")};
        hRP->SetDirectory(nullptr);
        auto* hSP {f->Get<TH2D>("hSP")};
        hSP->SetDirectory(nullptr);
        f->Close();

        // clone in order to save
        hsEx.push_back((TH1D*)hEx->Clone());
        hsKin.push_back((TH2D*)hKin->Clone());
        hsRPx.push_back((TH1D*)hRPx->Clone());
        hsCM.push_back((TH1D*)hCM->Clone());
        effs.push_back(eff);
        hsRP.push_back(hRP);
        hsSP.push_back(hSP);
        hsRPCM.push_back((TH2D*)hRPCM->Clone());
        idx++;
    }
    // Fit to gaussians!
    auto* gsigmas {new TGraphErrors()};
    double range {4.5}; // MeV autour de la moyenne
    for(int i = 0; i < hsEx.size(); i++)
    {
        if(isPS)
            continue;
        hsEx[i]->Fit("gaus", "0Q", "", Exs[i] - range, Exs[i] + range);
        auto* f {hsEx[i]->GetFunction("gaus")};
        if(!f)
            continue;
        gsigmas->SetPoint(gsigmas->GetN(), Exs[i], f->GetParameter("Sigma"));
        gsigmas->SetPointError(gsigmas->GetN() - 1, 0, f->GetParError(2));
    }

    // Read experimental RPx distribution
    ROOT::RDataFrame exp {"Sel_Tree", gSelector->GetAnaFile(3, beam, target, light, true)};
    auto haux {exp.Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
    auto* hRPxExp {(TH1D*)haux->Clone()};
    hRPxExp->Scale(1. / hRPxExp->Integral());
    hRPxExp->SetLineColor(8);
    auto hauxx {exp.Histo1D(HistConfig::ThetaCM, "ThetaCM")};
    auto* hCMExp {(TH1D*)hauxx->Clone()};
    hCMExp->Scale(1. / hCMExp->Integral());
    hCMExp->SetLineColor(8);

    // Plot!
    std::vector<TCanvas*> cs;
    for(int i = 0; i < hsEx.size(); i++)
    {
        cs.push_back(new TCanvas {TString::Format("c%d", i), TString::Format("Ex = %.2f MeV", Exs[i])});
        cs[i]->DivideSquare(6);
        // Get theoretical kinematics
        ActPhysics::Particle p1 {beam};
        double T1Total {T1 * p1.GetAMU()};
        ActPhysics::Kinematics kin(beam, target, light, T1Total, Exs[i]);
        auto* gtheo {kin.GetKinematicLine3()};
        // Kinematics
        cs[i]->cd(1);
        hsKin[i]->Draw("colz");
        gtheo->Draw("same");
        // Ex
        cs[i]->cd(2);
        if(!isPS)
            hsEx[i]->GetXaxis()->SetRangeUser(Exs[i] - range, Exs[i] + range);
        hsEx[i]->Draw("hist");
        for(auto* o : *(hsEx[i]->GetListOfFunctions()))
            if(o)
                o->Draw("same");
        // Get a line centered at Ex
        gPad->Update();
        auto* line {new TLine(Exs[i], gPad->GetUymin(), Exs[i], gPad->GetUymax())};
        line->SetLineWidth(2);
        line->SetLineColor(kMagenta);
        line->Draw();
        // Efficiency
        cs[i]->cd(3);
        effs[i]->Draw("apl");
        // RPx distribution
        cs[i]->cd(4);
        hsRPx[i]->Draw("hist");
        if(i == 0) // only reliable for g.s!
            hRPxExp->Draw("hist same");
        cs[i]->cd(5);
        hsSP[i]->Draw();
        cs[i]->cd(6);
        hsRPCM[i]->Draw("colz");
        // hsCM[i]->Draw("hist");
        // hCMExp->Draw("hist same");
    }

    if(isPS)
        return;

    auto* csigma {new TCanvas("csigma", "Sigmas from fits")};
    csigma->DivideSquare(1 + hsRPx.size());
    csigma->cd(1);
    gsigmas->SetTitle(";E_{x} [MeV];#sigma in E_{x} [MeV]");
    gsigmas->SetMarkerStyle(24);
    gsigmas->SetLineColor(kViolet);
    gsigmas->SetLineWidth(2);
    gsigmas->Draw("apl0");
    for(int i = 0; i < hsRPx.size(); i++)
    {
        csigma->cd(2 + i);
        hsRPx[i]->Draw("hist");
        hRPxExp->Draw("hist same");
    }

    auto* file {new TFile {TString::Format("/media/Data/E796v2/Simulation/Outputs/%s/sigmas_%s_%s_%s.root",
                                           gSelector->GetFlag().c_str(), beam.c_str(), target.c_str(), light.c_str()),
                           "recreate"}};
    file->cd();
    gsigmas->Write("gsigmas");
    file->Close();
}
