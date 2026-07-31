#ifndef Yield_cxx
#define Yield_cxx

#include "ActCrossSection.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "THStack.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TString.h"

#include "AngComparator.h"
#include "AngIntervals.h"
#include "Interpolators.h"

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "/media/Data/E796v2/Geant/Analyser.cxx"
#include "/media/Data/E796v2/Geant/Plotter.cxx"
#include "/media/Data/E796v2/PostAnalysis/HistConfig.h"
#include "yaml-cpp/yaml.h"

void ParseYAML(const std::string& file, std::vector<double>& exs, std::vector<std::string>& xs)
{
    auto node {YAML::LoadFile(file)};
    // Exs as string
    auto exsString {node["exs"].as<std::vector<std::string>>()};
    // xs path
    auto xspath {node["xspath"].as<std::string>()};
    // Dict
    auto dict {node["dic"].as<std::map<std::string, std::string>>()};
    // Build vector with xs and exs as doubles
    exs.clear();
    xs.clear();
    for(const auto& ex : exsString)
    {
        exs.push_back(std::stod(ex));
        auto xsfile {xspath + "/fort." + dict[ex]};
        xs.push_back(xsfile);
    }
}

void ResetBinErrors(TH1* h)
{
    for(int bin = 1; bin <= h->GetNbinsX(); bin++)
        h->SetBinError(bin, TMath::Sqrt(h->GetBinContent(bin)));
}

void Yield(const std::string& beam, const std::string& target, const std::string& light, double ebeam,
           const std::string& yaml, double Nb, double Nt, double Nit)
{
    ROOT::EnableImplicitMT();

    // Parse config file
    std::vector<double> exs;
    std::vector<std::string> xsfiles;
    ParseYAML(yaml, exs, xsfiles);

    // Parse each ex
    std::vector<TH1D*> hsEx {};
    std::vector<TH2D*> hsKin {};
    // And build intervals
    std::deque<Angular::Intervals> ivs {}; // deque bc vector has issues with mutex inside Intervals
    double thetaCMMin {5};
    double thetaCMMax {40};
    double thetaCMStep {2};
    // Efficiencies
    std::vector<Interpolators::Efficiency> effs;
    for(const auto& ex : exs)
    {
        auto df {ROOT::RDataFrame {"AnaTree", GetAnaFile(beam, target, light, ebeam, ex)}};
        // Ex
        auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
        hEx->SetTitle(TString::Format("E_{x} = %.2f", ex));
        // Kin
        auto hKin {df.Histo2D(HistConfig::KinSimu, "thetaLab", "EVertex")};

        // Fill ivs
        ivs.emplace_back(thetaCMMin, thetaCMMax, HistConfig::Ex, thetaCMStep, 0);
        auto& iv {ivs.back()};
        df.Foreach([&](double thetacm, double ex) { iv.Fill(thetacm, ex); }, {"thetaCM", "Ex"});

        // Clone and save
        hsEx.push_back((TH1D*)hEx->Clone());
        hsKin.push_back((TH2D*)hKin->Clone());

        // Read other objects
        auto file {std::make_unique<TFile>(GetAnaFile(beam, target, light, ebeam, ex))};
        auto* eff {file->Get<TEfficiency>("eff")};
        Interpolators::Efficiency ieff {};
        ieff.Add("g0", eff);
        effs.push_back(ieff);
    }

    // Copy first
    auto* hExAdd {(TH1D*)hsEx[0]->Clone()};
    hExAdd->Reset();
    hExAdd->SetTitle("E_{x}");
    auto* hKinAdd {(TH2D*)hsKin[0]->Clone()};
    hKinAdd->Reset();
    hKinAdd->SetTitle("Kin");
    // Create stack
    auto* stack {new THStack};


    // Scale, add and store
    auto* gall {new TGraphErrors};
    gall->SetTitle("Total counts per state;E_{x} [MeV];Counts");
    auto* mgtheta {new TMultiGraph};
    mgtheta->SetTitle("Rec xs;#theta_{CM} [#circ];xs [mb/sr]");
    auto* mgtheo {new TMultiGraph};
    mgtheo->SetTitle("Theo xs;#theta_{CM} [#circ];xs [mb/sr]");
    // Theoretical xs
    std::vector<TGraph*> theoxs;
    // Comparators
    std::vector<Angular::Comparator> comps;
    // Reconstructed xs
    std::vector<TGraphErrors*> recxs;
    // Resolution
    auto* gres {new TGraphErrors};
    for(int i = 0; i < exs.size(); i++)
    {
        const auto& ex {exs[i]};
        auto& iv {ivs[i]};
        auto& eff {effs[i]};

        // Compute scaling factor
        ActSim::CrossSection xs;
        xs.ReadUsingTGraph(xsfiles[i]);
        mgtheo->Add(xs.GetTheoXSGraph());
        theoxs.push_back(xs.GetTheoXSGraph());
        auto xsIntegral {xs.GetTotalXScm2()};
        auto alpha {xsIntegral * Nb * Nt / Nit};
        std::cout << "Scaling factor for Ex = " << exs[i] << " : " << alpha << '\n';


        // Scale
        auto& h {hsEx[i]};
        h->Scale(alpha);
        ResetBinErrors(h);
        // Calculate integral
        auto integral {h->Integral()};
        gall->AddPointError(ex, integral, 0, TMath::Sqrt(integral));

        // Also for Intervals
        auto* git {new TGraphErrors};
        for(int j = 0; j < iv.GetSize(); j++)
        {
            auto hiv {iv.GetHistos()[j]};
            hiv->Scale(alpha);
            ResetBinErrors(hiv);
            auto integral {hiv->Integral()};
            auto uintegral {TMath::Sqrt(integral)};
            auto Omega {iv.GetOmega(j)};
            auto eps {eff.GetPointEff("g0", iv.GetCenter(j))};
            if(eps == 0)
                continue;
            integral /= (Nt * Nb * eps * Omega * 1e-27);
            uintegral /= (Nt * Nb * eps * Omega * 1e-27);
            git->AddPointError(iv.GetCenter(j), integral, 0, uintegral);
        }
        mgtheta->Add(git);
        recxs.push_back(git);

        // Eval once again resolution
        Fit(h, gres);

        // Comparator
        Angular::Comparator comp {TString::Format("E_{x} = %.2f", ex).Data(), git};
        comp.Add("fresco", xsfiles[i]);
        comp.Fit();
        comps.push_back(comp);

        // Kinematics
        // hsKin[i]->Scale(alpha);
        hKinAdd->Add(hsKin[i]);

        // Add to containers
        hExAdd->Add(h);
        stack->Add(h);
    }

    // Write to file
    auto fout {std::make_unique<TFile>(
        TString::Format("./Outputs/yield_%s_%s_%s_%.1f.root", beam.c_str(), target.c_str(), light.c_str(), ebeam)
            .Data(),
        "recreate")};
    // Kin
    hKinAdd->Write("hKinAll");
    // Exs
    hExAdd->Write("hExAll");
    // Total counts
    gall->Write("gCounts");
    // Resolution
    gres->Write("gRes");
    // Individual Exs
    for(int i = 0; i < hsEx.size(); i++)
        hsEx[i]->Write(TString::Format("hEx%d", i));
    // Efficiencies
    for(int i = 0; i < effs.size(); i++)
        effs[i].GetGraph("g0")->Write(TString::Format("eff%d", i));
    // Theo xs
    for(int i = 0; i < theoxs.size(); i++)
        theoxs[i]->Write(TString::Format("theo%d", i));
    // Reconstructed xs
    for(int i = 0; i < recxs.size(); i++)
        recxs[i]->Write(TString::Format("rec%d", i));
    fout->Close();

    // Draw
    auto* c0 {new TCanvas {"cYield", "Yields"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hKinAdd->Draw("colz");
    c0->cd(2);
    hExAdd->Draw("histe");
    stack->Draw("histe same nostack plc pmc");
    c0->cd(3);
    gall->Draw("a*l");
    c0->cd(4);
    mgtheta->Draw("a*l plc pmc");
    c0->cd(5);
    mgtheo->Draw("a*l plc pmc");

    auto* c1 {new TCanvas {"c1", "Comparator canvas"}};
    c1->DivideSquare(comps.size());
    for(int i = 0; i < comps.size(); i++)
    {
        c1->cd(i + 1);
        comps[i].Draw("", false, true, 3, gPad);
    }
}
#endif