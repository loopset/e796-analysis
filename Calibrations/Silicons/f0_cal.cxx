#include "ActSRIM.h"

#include "TCanvas.h"
#include "TChain.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "TROOT.h"
#include "TString.h"
#include "TStyle.h"
#include "TVirtualPad.h"

#include "CalibrationRunner.h"
#include "CalibrationSource.h"

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

std::vector<TH1D*> ReadData(const std::string& file, const std::string& dir, const std::string& label)
{
    auto* f {new TFile {file.c_str()}};
    // f->ls();
    auto updir {f->Get<TDirectory>("Raw")};
    updir->ls();
    auto lowdir {updir->Get<TDirectory>(dir.c_str())};
    lowdir->ls();
    // Read
    std::vector<TH1D*> ret;
    auto keys {lowdir->GetListOfKeys()};
    for(auto* key : *keys)
    {
        std::string str {key->GetName()};
        auto idx {str.find_first_of("_")};
        auto name {str.substr(0, idx)};
        if(!(name == label))
            continue;
        ret.push_back((TH1D*)lowdir->Get<TH1D>(key->GetName()));
    }
    return ret;
}

void CorrectSource(Calibration::Source* source, ActPhysics::SRIM* srim, const std::string& table, double thickness,
                   double angle = 0)
{
    auto& energies {source->GetRefToEnergies()};
    auto& limits {source->GetRefToLimits()};
    auto labels {source->GetLabels()};
    // Peak energies
    for(auto& source : energies)
        for(auto& energy : source)
            energy = srim->Slow(table, energy, thickness, angle);
    // Peak boundaries
    for(auto& [key, vals] : limits)
    {
        vals.first = srim->Slow(table, vals.first, thickness, angle);
        vals.second = srim->Slow(table, vals.second, thickness, angle);
    }
}

void f0_cal()
{
    std::string which {"f0"};
    std::string label {"F0"};
    // Read data
    auto hs {ReadData("./Inputs/histos_run_0417.root", "F0", label)};
    // Pick only necessary
    int isil {};
    std::vector<int> adcChannels {};
    for(auto it = hs.begin(); it != hs.end();)
    {
        // if(isil < 2 || isil > 6 || isil == 4)
        //     it = hs.erase(it);
        // else
        // {
        adcChannels.push_back(isil);
        it++;
        // }
        isil++;
    }
    // hs = {hs[2], hs[3], hs[5], hs[6]};

    // Source of ganil
    Calibration::Source source {};
    // source.Print();

    // Correct by energy losses in Al dead layer
    ActPhysics::SRIM srim {"al", "../SRIMData/raw/4He_silicon.txt"};
    CorrectSource(&source, &srim, "al", 0.5e-3); // 0.5 um to mm
    // source.Print();

    // Rebin
    std::vector<TH1D*> hsrebin;
    int idx {};
    for(auto& h : hs)
    {
        hsrebin.push_back((TH1D*)h->Clone());
        hsrebin.back()->Rebin(8);
        idx++;
    }
    // Runner per silicon
    std::vector<Calibration::Runner> runners;
    // Graph
    auto* gr {new TGraphErrors};
    gr->SetNameTitle("g", "Resolution;;#sigma ^{241}Am [keV]");
    // Save
    std::ofstream streamer {"./Outputs/thesis_" + which + ".dat"};
    streamer << std::fixed << std::setprecision(8);
    std::vector<std::shared_ptr<TH1D>> hfs;
    for(int s = 0; s < hsrebin.size(); s++)
    {
        // Extract sil idx from hist name
        std::string name {hs[s]->GetName()};
        auto it {name.find_first_of("_")};
        auto idxStr {name.substr(it + 1)};
        int idx {std::stoi(idxStr)};

        const auto& adcChannel {adcChannels[s]};
        runners.emplace_back(&source, hsrebin[s], hs[s], false);
        auto& run {runners.back()};
        run.SetGaussPreWidth(60);
        run.SetRange(1500, 3000);
        run.DisableXErrors();
        if(s == 4)
            run.SertMinSigma(0.02);
        run.DoIt();
        auto* c {new TCanvas};
        run.Draw(c);
        std::cout << "Sil index : " << s << " hist name : " << hs[s]->GetName() << '\n';
        run.PrintRes();
        std::cout << '\n';
        auto sigma {run.GetRes("241Am")};
        auto usigma {run.GetURes("241Am")};
        gr->SetPoint(s, adcChannel, (sigma < 0 ? 0 : sigma) * 1e3);
        gr->SetPointError(s, 0, (usigma < 0 ? 0 : usigma) * 1e3);
        hfs.push_back(run.GetHistFinal());

        // Save calibration in file
        auto label {TString::Format("Sil_%s_%d_E", which.c_str(), idx)};
        auto labelp {TString::Format("Sil_%s_%d_P", which.c_str(), idx)};
        auto [p0, p1] {runners.back().GetParameters()};
        streamer << label << " " << p0 << " " << p1 << '\n';
        auto [ped, peds] {runners.back().GetPedestal()};
        streamer << labelp << " " << ped << " " << peds << '\n';
    }
    streamer.close();

    // Plot
    auto* c0 {new TCanvas {"c0", "Raw silicon data"}};
    c0->DivideSquare(hs.size());
    for(int i = 0; i < hs.size(); i++)
    {
        c0->cd(i + 1);
        gPad->SetLogy();
        hs[i]->GetXaxis()->SetRangeUser(0, 3000);
        hs[i]->Draw();
    }

    auto* c1 {new TCanvas {"c11", "Resolution canvas"}};
    gr->SetMarkerStyle(25);
    gr->SetLineWidth(2);
    gr->GetXaxis()->SetNdivisions(515);
    gr->Draw("apl");
    for(int i = 0; i < hs.size(); i++)
    {
        auto* ax {gr->GetXaxis()};
        auto bin {ax->FindBin(i)};
        ax->SetBinLabel(bin, hs[i]->GetTitle());
    }

    auto* c2 {new TCanvas {"c2", "Final his canvas"}};
    // gROOT->SetSelectedPad(nullptr);
    c2->DivideSquare(hfs.size());
    for(int p = 0; p < hfs.size(); p++)
    {
        c2->cd(p + 1);
        if(!hfs[p])
            continue;
        // hfs[p]->SetTitle(TString::Format("%s_%d", label.c_str(), p + 1));
        hfs[p]->SetTitle(hs[p]->GetTitle());
        hfs[p]->DrawClone();
        for(auto* o : *(hfs[p]->GetListOfFunctions()))
            if(o)
                o->DrawClone("same");
    }

    // Save to disk
    auto fout {std::make_unique<TFile>("./Outputs/histos_cal_f0.root", "recreate")};
    // Write for silicon 5
    hfs[5]->Write();
    for(auto& [key, funcs] : runners[5].GetFinalSat())
        for(auto& func : funcs)
            func->Write();
}
