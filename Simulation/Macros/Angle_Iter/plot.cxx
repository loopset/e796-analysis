#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TEfficiency.h"
#include "TF1.h"
#include "TGraphErrors.h"
#include "TH1.h"
#include "THStack.h"
#include "TMultiGraph.h"
#include "TString.h"

#include "AngComparator.h"
#include "AngDifferentialXS.h"
#include "AngIntervals.h"
#include "Interpolators.h"
#include "PhysExperiment.h"

#include <memory>
#include <vector>

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

void plot()
{
    std::string target {"1H"};
    std::string light {"2H"};
    gSelector->SetTarget(target);
    gSelector->SetLight(light);

    // And set excitation energies
    std::vector<double> Exs;
    std::vector<std::string> keys;
    if(auto str {gSelector->GetShortStr()}; str == "pd")
    {
        Exs = {0};
        keys = {"g0"};
    }
    else if(str == "dt")
    {
        Exs = {0, 3.24};
        keys = {"g0", "g2"};
    }
    else
        throw std::runtime_error("Channels other than pd or dt not implemented yet");

    // Read analysis
    TH1::AddDirectory(false);
    ROOT::EnableImplicitMT();
    ROOT::RDataFrame ana {"Sel_Tree", gSelector->GetAnaFile(3)};
    auto hEx {ana.Histo1D(HistConfig::Ex, "Ex")};
    std::vector<TH1D*> hsAnaRPx, hsAnaEx;
    // Fit analysis
    for(const auto& ex : Exs)
    {
        auto sigma {Fit(hEx.GetPtr(), ex, 1.5)};
        auto node {ana.Filter([=](double e) { return std::abs(e - ex) <= 3 * sigma; }, {"Ex"})};
        auto hRPx {node.Histo1D(HistConfig::RPx, "fRP.fCoordinates.fX")};
        auto hEx {node.Histo1D(HistConfig::Ex, "Ex")};
        hRPx->SetLineColor(8);
        hEx->SetLineColor(8);
        hsAnaRPx.push_back((TH1D*)hRPx->Clone());
        hsAnaEx.push_back((TH1D*)hEx->Clone());
    }

    // Compute cross-sections for g.s
    auto path {TString::Format("/media/Data/E796v2/Fits/%s/", gSelector->GetShortStr().c_str())};
    auto fxs {std::make_unique<TFile>(path + "Outputs/counts.root")};
    std::vector<TGraphErrors*> gexps;
    for(const auto& key : keys)
    {
        auto* gexp {fxs->Get<TGraphErrors>(("g" + key).c_str())};
        if(!gexp)
            throw std::runtime_error("Cannot read experimental count graph");
        gexps.push_back(gexp);
    }
    // Experimental info
    PhysUtils::Experiment exp {};
    if(target == "1H")
        exp.Read("/media/Data/E796v2/Fits/norms/p_target.dat");
    else
        exp.Read("/media/Data/E796v2/Fits/norms/d_target.dat");
    // Intervals
    Angular::Intervals ivs;
    ivs.Read((path + "Outputs/ivs.root").Data());
    ROOT::EnableImplicitMT();

    // Read simulation
    gSelector->SetFlag("iter_angle");
    // Angles
    std::vector<double> angles;
    for(double a = 0; a < 10; a += 2)
        angles.push_back(a);

    // Get histograms
    std::vector<std::vector<TH1D*>> hsRPx;
    std::vector<std::vector<TH2D*>> hsSP;
    std::vector<std::vector<TEfficiency*>> effs;
    for(const auto& angle : angles)
    {
        auto tag {TString::Format("ang_%.2f", angle)};
        gSelector->SetTag(tag.Data());
        hsRPx.push_back({});
        hsSP.push_back({});
        effs.push_back({});
        for(const auto& ex : Exs)
        {
            auto file {gSelector->GetApproxSimuFile("20O", target, light, ex)};
            std::cout << "File : " << file << '\n';
            ROOT::RDataFrame df {"SimulationTTree", file};
            auto hRPx {df.Histo1D(HistConfig::RPx, "RPx")};
            hRPx->SetTitle(TString::Format("#theta = %.2f#circ E_{x} = %.2f", angle, ex));
            hsRPx.back().push_back((TH1D*)hRPx->Clone());
            // SP histogram
            auto f {std::make_unique<TFile>(file.c_str())};
            auto* hSP {f->Get<TH2D>("hSP")};
            hSP->SetDirectory(nullptr);
            hsSP.back().push_back(hSP);
            auto* eff {f->Get<TEfficiency>("eff")};
            eff->SetDirectory(nullptr);
            eff->SetTitle(TString::Format("%.2f", angle));
            effs.back().push_back(eff);
            f->Close();
        }
    }
    // Important: normalize histograms
    for(auto& h : hsAnaRPx)
    {
        h->Rebin(2);
        h->Scale(1. / h->Integral());
    }
    for(auto& vec : hsRPx)
        for(auto& h : vec)
        {
            h->Rebin(2);
            h->Scale(1. / h->Integral());
        }

    // Compute cross section for g.s
    std::vector<std::vector<Angular::Comparator>> comps(angles.size());
    for(int i = 0; i < angles.size(); i++)
    {
        const auto& angle {angles[i]};
        for(int j = 0; j < Exs.size(); j++)
        {
            auto key {keys[j]};
            Interpolators::Efficiency aux;
            aux.Add(key, effs[i][j]);
            Angular::DifferentialXS xs {&ivs, &aux, &exp};
            xs.DoFor(gexps[j], key);
            auto gxs {xs.Get(key)};
            auto str {gSelector->GetShortStr()};
            Angular::Comparator comp {TString::Format("%s %s #theta=%.2f#circ", str.c_str(), key.c_str(), angle).Data(),
                                      gxs};
            if(str == "pd")
                comp.Add(key, (path + "Inputs/g0_2FNR/21.g0").Data());
            else if(str == "dt")
            {
                if(key == "g0")
                    comp.Add(key, (path + "Inputs/gs/Fresco/fort.202").Data());
                else if(key == "g2")
                    comp.Add(key, (path + "Inputs/ex_3.16/l1/fort.202").Data());
                else
                    throw std::runtime_error("No Comparator for dt in this channel");
            }
            else
                throw std::runtime_error("Comparator file not specified");
            comp.Fit();
            comps[i].push_back(comp);
        }
    }

    // Draw
    for(int ic = 0; ic < Exs.size(); ic++)
    {
        auto* cRPx {new TCanvas {TString::Format("cRPx%d", ic), TString::Format("Ex = %.2f MeV", Exs[ic])}};
        cRPx->DivideSquare(angles.size());
        int ip {1};
        for(int id = 0; id < angles.size(); id++)
        {
            cRPx->cd(ip);
            auto* stack {new THStack};
            stack->SetTitle(TString::Format("%s;RP.X() [mm];Normalized counts", hsRPx[id][ic]->GetTitle()));
            stack->Add(hsRPx[id][ic]);
            stack->Add(hsAnaRPx[ic]);
            stack->Draw("nostack histe");
            ip++;
        }
        auto* cxs {new TCanvas {TString::Format("cxs%d", ic), TString::Format("Ex = %.2f MeV", Exs[ic])}};
        cxs->DivideSquare(angles.size());
        ip = 1;
        for(int id = 0; id < angles.size(); id++)
        {
            auto pad {cxs->cd(ip)};
            comps[id][ic].Draw("", false, true, 3, pad);
            ip++;
        }
    }

    gROOT->SetSelectedPad(nullptr);
    auto* c0 {new TCanvas {"cEff", "Ex canvas"}};
    c0->DivideSquare(6);
    c0->cd(1);
    hEx->DrawClone();
    for(auto h : hsAnaEx)
        h->Draw("same");
    // Draw all effs together
    for(int j = 0; j < Exs.size(); j++)
    {
        Interpolators::Efficiency inter;
        for(int i = 0; i < angles.size(); i++)
            inter.Add(TString::Format("%.0f", angles[i]).Data(), (TEfficiency*)effs[i][j]->Clone());
        inter.Draw(true, keys[j], c0->cd(4 + j));
    }
}
