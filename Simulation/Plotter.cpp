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
#include "TVirtualPad.h"

#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "/media/Data/E796v2/Simulation/Utils.cxx"

void Plotter(const std::vector<double>& Exs, const std::string& beam, const std::string& target,
             const std::string& light, const std::string& heavy, double T1, int neutronPS, int protonPS)
{
    ROOT::EnableImplicitMT();

    // Set if we should fit or not
    bool isPS {(neutronPS != 0 || protonPS != 0)};
    if(!isPS)
        std::cout << BOLDCYAN << "Plotting usual simulation" << RESET << '\n';
    else
        std::cout << BOLDCYAN << "Plotting a phase space" << RESET << '\n';

    // Save histograms
    std::vector<TH1D*> hsEx;
    std::vector<TH2D*> hsKin, hsSP;
    std::vector<TEfficiency*> effs;
    // Iterate
    int idx {1};
    for(const auto& Ex : Exs)
    {
        auto file {E796Simu::GetFile(beam, target, light, Ex, neutronPS, protonPS)};
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

        // Read efficiency
        auto* f {new TFile(file)};
        auto* eff {f->Get<TEfficiency>("eff")};
        if(!eff)
            throw std::runtime_error("Could not read TEfficiency named eff in file " + file);
        f->Close();

        // clone in order to save
        hsEx.push_back((TH1D*)hEx->Clone());
        hsKin.push_back((TH2D*)hKin->Clone());
        effs.push_back(eff);
        idx++;
    }
    // Fit to gaussians!
    auto* gsigma {new TGraphErrors()};
    double range {4.5}; // MeV autour de la moyenne
    for(int i = 0; i < hsEx.size(); i++)
    {
        if(isPS)
            continue;
        hsEx[i]->Fit("gaus", "0Q", "", Exs[i] - range, Exs[i] + range);
        auto* f {hsEx[i]->GetFunction("gaus")};
        gsigma->SetPoint(gsigma->GetN(), Exs[i], f->GetParameter("Sigma"));
        gsigma->SetPointError(gsigma->GetN() - 1, 0, f->GetParError(2));
    }

    // plot!
    std::vector<TCanvas*> cs;
    for(int i = 0; i < Exs.size(); i++)
    {
        cs.push_back(new TCanvas {TString::Format("c%d", i), TString::Format("Ex = %.2f MeV", Exs[i])});
        cs[i]->DivideSquare(4);
        // Get theoretical kinematics
        ActPhysics::Particle p1 {beam};
        double T1Total {T1 * p1.GetAMU()};
        ActPhysics::Kinematics kin(beam, target, light, heavy, T1Total, Exs[i]);
        auto* gtheo {kin.GetKinematicLine3()};
        // Kinematics
        cs[i]->cd(1);
        hsKin[i]->Draw("colz");
        gtheo->Draw("same");
        // Ex
        cs[i]->cd(2);
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
    }

    if(isPS)
        return;

    auto* csigma {new TCanvas("csigma", "Sigmas from fits")};
    gsigma->SetTitle(";E_{x} [MeV];#sigma in E_{x} [MeV]");
    gsigma->SetMarkerStyle(24);
    gsigma->SetLineColor(kViolet);
    gsigma->SetLineWidth(2);
    gsigma->Draw("apl0");
}
