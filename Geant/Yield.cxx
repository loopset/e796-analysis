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
    auto exsString {node["exs"] ? node["exs"].as<std::vector<std::string>>() : std::vector<std::string> {}};

    // xs path
    auto xspath {node["xspath"] ? node["xspath"].as<std::string>() : std::string {}};

    // Dict
    auto dict {node["dic"] ? node["dic"].as<std::map<std::string, std::string>>()
                           : std::map<std::string, std::string> {}};

    // Build vector with xs and exs as doubles
    exs.clear();
    xs.clear();
    for(const auto& ex : exsString)
    {
        exs.push_back(std::stod(ex));
        if(xspath.length())
        {
            auto xsfile {xspath + "/" + dict[ex] + ".dat"};
            xs.push_back(xsfile);
        }
    }
}

void ResetBinErrors(TH1* h)
{
    for(int bin = 1; bin <= h->GetNbinsX(); bin++)
        h->SetBinError(bin, TMath::Sqrt(h->GetBinContent(bin)));
}

void Yield(const std::string& beam, const std::string& target, const std::string& light, double ebeam,
           const std::string& yaml, double Nb, double Nt, double Nit, double averageSF)
{
    std::cout << "--- Yield ----" << '\n';
    std::cout << " Nb : " << Nb << '\n';
    std::cout << " Nt : " << Nt << '\n';
    std::cout << " Nit : " << Nit << '\n';
    std::cout << " averageSF : " << averageSF << '\n';
    ROOT::EnableImplicitMT();

    // Parse config file
    std::vector<double> exs;
    std::vector<std::string> xsfiles;
    ParseYAML(yaml, exs, xsfiles);
    // if no theo xs provided in simu, xsfiles is EMPTY

    // Parse each ex
    std::vector<TH1D*> hsEx {};
    std::vector<TH2D*> hsKin {};
    // And build intervals
    std::deque<Angular::Intervals> ivs {}; // deque bc vector has issues with mutex inside Intervals
    double thetaCMMin {0};
    double thetaCMMax {60};
    double thetaCMStep {5};
    // Efficiencies
    std::vector<Interpolators::Efficiency> effs;
    for(const auto& ex : exs)
    {
        auto df {ROOT::RDataFrame {"AnaTree", GetAnaFile(beam, target, light, ebeam, ex)}};
        // Ex
        auto hEx {df.Histo1D(HistConfig::Ex, "Ex")};
        hEx->SetTitle(TString::Format("E_{x} = %.2f", ex));
        // Kin
        auto hKin {df.Histo2D(HistConfig::KinGeant, "thetaLab", "EVertex")};

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
    auto* gallXS {new TGraphErrors};
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
    // Relative uncertainty on counts
    std::vector<TGraphErrors*> unccounts;
    // Reconstructed counts
    std::vector<TGraphErrors*> reccounts;

    // If theoretical xs
    for(int i = 0; i < exs.size(); i++)
    {
        const auto& ex {exs[i]};
        auto& iv {ivs[i]};
        auto& eff {effs[i]};

        // Compute scaling factor alpha
        double alpha {1.};
        ActSim::CrossSection xs;
        if(xsfiles.size())
        {
            xs.ReadUsingTGraph(xsfiles[i]);
            mgtheo->Add(xs.GetTheoXSGraph());
            theoxs.push_back(xs.GetTheoXSGraph());
            auto xsIntegral {xs.GetTotalXScm2()};
            std::cout << "Integrated xs : " << xs.GetTotalXSmbarn() << '\n';
            alpha = averageSF * xsIntegral * Nb * Nt / Nit;
            std::cout << "Scaling factor for Ex = " << exs[i] << " : " << alpha << '\n';
        }
        else
            std::cout << "Assuming alpha = 1 as no theo xs provided" << '\n';


        // Scale
        auto& h {hsEx[i]};
        h->Scale(alpha);
        ResetBinErrors(h);
        // Calculate integral
        auto integral {h->Integral()};
        gall->AddPointError(ex, integral, 0, TMath::Sqrt(integral));

        // Also for Intervals
        // std::cout << "===================" << '\n';
        auto* git {new TGraphErrors};
        auto* gunc {new TGraphErrors};
        gunc->SetTitle(
            TString::Format("#sigma(N)/N for E_{x} = %.2f;#theta_{CM} [#circ];#sigma(N)/N [percent]", ex).Data());
        auto* grec {new TGraphErrors};
        grec->SetTitle(TString::Format("Rec counts E_{x} = %.2f;#theta_{CM} [#circ];Counts", ex).Data());
        for(int j = 0; j < iv.GetSize(); j++)
        {
            auto hiv {iv.GetHistos()[j]};
            hiv->Scale(alpha);
            ResetBinErrors(hiv);
            auto integral {hiv->Integral()};
            auto uintegral {TMath::Sqrt(integral)};
            // To avoid rel unc > 100 %
            if(integral > 2)
            {
                grec->AddPointError(iv.GetCenter(j), integral, 0, uintegral);
                gunc->AddPoint(iv.GetCenter(j), TMath::Sqrt(integral) / integral * 100);
            }
            // std::cout << ".............." << '\n';
            // std::cout << "theta : " << iv.GetCenter(j) << '\n';
            // std::cout << " N : " << integral << " uN : " << uintegral << '\n';
            auto Omega {iv.GetOmega(j)};
            auto eps {eff.GetMeanEff("g0", iv.GetLow(j), iv.GetUp(j))};
            if(eps <= 0.05)
                continue;
            integral /= (Nt * Nb * eps * Omega * 1e-27);
            uintegral /= (Nt * Nb * eps * Omega * 1e-27);
            git->AddPointError(iv.GetCenter(j), integral, 0, uintegral);
            // std::cout << "Eval xs : " << integral << " +/- " << uintegral << '\n';
            // std::cout << " eps : " << eps << " Omega : " << Omega
            //           << " theo xs : " << averageSF * xs.GetTheoXSGraph()->Eval(iv.GetCenter(j)) << '\n';
            // Theoretical counts per bin
            // auto theoN {Nt * Nb * eps * Omega * 1e-27 * averageSF * xs.GetTheoXSGraph()->Eval(iv.GetCenter(j))};
            // std::cout << " theoN : " << theoN << '\n';
            // sum += theoN;
        }
        // std::cout<< "Total theo counts : " << sum << '\n';
        mgtheta->Add(git);
        recxs.push_back(git);
        unccounts.push_back(gunc);
        reccounts.push_back(grec);

        // Eval once again resolution
        Fit(h, gres);

        // Comparator
        Angular::Comparator comp {TString::Format("E_{x} = %.2f", ex).Data(), git};
        if(xsfiles.size())
        {
            comp.Add("theo", xsfiles[i]);
            comp.Fit();
            comps.push_back(comp);
        }

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
    gallXS->SetLineColor(kRed);
    gallXS->Draw("pl same");
    c0->cd(4);
    mgtheta->Draw("a*l plc pmc");
    c0->cd(5);
    mgtheo->Draw("a*l plc pmc");
    c0->cd(6);
    gres->Draw("apl");

    auto* c1 {new TCanvas {"c1", "Comparator canvas"}};
    c1->DivideSquare(comps.size());
    for(int i = 0; i < comps.size(); i++)
    {
        c1->cd(i + 1);
        comps[i].Draw("", false, true, 3, gPad);
    }

    auto* c2 {new TCanvas {"c2", "Uncertainty counts canvas"}};
    c2->DivideSquare(unccounts.size());
    for(int i = 0; i < unccounts.size(); i++)
    {
        c2->cd(i + 1);
        unccounts[i]->Draw("apl");
    }

    auto* c3 {new TCanvas {"c3", "Rec counts canvas"}};
    c3->DivideSquare(reccounts.size());
    for(int i = 0; i < reccounts.size(); i++)
    {
        c3->cd(i + 1);
        reccounts[i]->Fit("pol0", "Q");
        reccounts[i]->Draw("apl");
        auto* f {reccounts[i]->GetFunction("pol0")};
        auto counts {f->GetParameter(0)};
        std::cout << "Ex = " << exs[i] << " counts in " << thetaCMStep << " : " << counts << " +/- "
                  << TMath::Sqrt(counts) / counts * 100 << " % " << " and total counts : " << gall->GetPointY(i)
                  << " +/- " << gall->GetErrorY(i) << '\n';
    }
}
#endif
