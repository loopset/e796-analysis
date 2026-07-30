#ifndef Yield_cxx
#define Yield_cxx

#include "ActCrossSection.h"

#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TGraphErrors.h"
#include "THStack.h"
#include "TMath.h"
#include "TMultiGraph.h"

#include "AngIntervals.h"

#include <iostream>
#include <string>
#include <vector>

#include "/media/Data/E796v2/Geant/Analyser.cxx"
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
    // And build intervals
    std::deque<Angular::Intervals> ivs {}; // deque bc vector has issues with mutex inside Intervals
    double thetaCMMin {0};
    double thetaCMMax {60};
    double thetaCMStep {1};
    for(const auto& ex : exs)
    {
        auto df {ROOT::RDataFrame {"AnaTree", GetAnaFile(beam, target, light, ebeam, ex)}};
        auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
        hEx->SetTitle(TString::Format("E_{x} = %.2f", ex));
        // Fill ivs
        ivs.emplace_back(thetaCMMin, thetaCMMax, HistConfig::Ex, thetaCMStep, 0);
        auto& iv {ivs.back()};
        df.Foreach([&](double thetacm, double ex) { iv.Fill(thetacm, ex); }, {"thetaCM", "Ex"});

        // Clone and save
        hsEx.push_back((TH1D*)hEx->Clone());
    }

    // Copy first
    auto* hExAdd {(TH1D*)hsEx[0]->Clone()};
    hExAdd->Reset();
    // Create stack
    auto* stack {new THStack};

    // Scale, add and store
    auto* gall {new TGraphErrors};
    gall->SetTitle("Total counts per state;E_{x} [MeV];Counts");
    auto* mgtheta {new TMultiGraph};
    mgtheta->SetTitle("Rec xs;#theta_{CM} [#circ];xs [mb/sr]");
    auto* mgtheo {new TMultiGraph};
    mgtheo->SetTitle("Theo xs;#theta_{CM} [#circ];xs [mb/sr]");
    for(int i = 0; i < exs.size(); i++)
    {
        const auto& ex {exs[i]};
        auto& iv {ivs[i]};

        // Compute scaling factor
        ActSim::CrossSection xs;
        xs.ReadUsingTGraph(xsfiles[i]);
        mgtheo->Add(xs.GetTheoXSGraph());
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
            integral /= (Nt * Nb * Omega * 1e-27);
            uintegral /= (Nt * Nb * Omega * 1e-27);
            git->AddPointError(iv.GetCenter(j), integral, 0, uintegral);
        }
        mgtheta->Add(git);

        // Add to containers
        hExAdd->Add(h);
        stack->Add(h);
    }

    // Draw
    auto* c0 {new TCanvas {"cYield", "Yields"}};
    c0->DivideSquare(4);
    c0->cd(1);
    hExAdd->Draw("histe");
    stack->Draw("histe same nostack plc pmc");
    c0->cd(2);
    gall->Draw("a*l");
    c0->cd(3);
    mgtheta->Draw("a*l plc pmc");
    c0->cd(4);
    mgtheo->Draw("a*l plc pmc");
}
#endif