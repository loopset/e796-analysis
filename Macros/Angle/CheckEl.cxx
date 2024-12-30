#include "ActKinematics.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"
#include "Rtypes.h"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "TGraphErrors.h"
#include "TH2.h"
#include "THStack.h"
#include "TMath.h"
#include "TMultiGraph.h"
#include "TString.h"
#include "TVirtualPad.h"

#include <memory>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"

void DrawFuncs(THStack* s)
{
    for(auto* o : *s->GetHists())
    {
        auto* h {(TH1D*)o};
        for(auto* e : *h->GetListOfFunctions())
            e->Draw("same");
    }
}

void CheckEl()
{
    // Read data
    ROOT::EnableImplicitMT();
    // pp
    ROOT::RDataFrame pp {"Final_Tree", gSelector->GetAnaFile(2, "20O", "1H", "1H", false)};
    // dt
    ROOT::RDataFrame dd {"Final_Tree", gSelector->GetAnaFile(2, "20O", "2H", "2H", false)};
    std::vector<ROOT::RDF::RNode> dfs {pp, dd};
    std::vector<std::string> labels {"(p,p)", "(d,d)"};

    // Create kinematics
    ActPhysics::Kinematics kpd {"20O(p,p)@700"};
    ActPhysics::Kinematics kdt {"20O(d,d)@700"};
    std::vector<std::vector<ActPhysics::Kinematics>> vks;
    for(int i = 0; i < dfs.size(); i++)
    {
        ActPhysics::Kinematics k;
        if(i == 0)
            k = kpd;
        else
            k = kdt;
        vks.push_back(std::vector<ActPhysics::Kinematics>(dfs[i].GetNSlots()));
        for(auto& e : vks.back())
            e = k;
    }
    // Read correction functions
    auto file {std::make_unique<TFile>("./Outputs/angle_corr_side_v1.root")};
    auto* func1 {file->Get<TF1>("func1")};
    auto* func2 {file->Get<TF1>("func2")};
    file->Close();

    // And define and plot
    auto* sEx {new THStack};
    sEx->SetTitle(HistConfig::Ex.fTitle);
    auto* sExLeg {new THStack};
    sExLeg->SetTitle(HistConfig::ChangeTitle(HistConfig::Ex, "No correction E_{x}").fTitle);
    auto* sExJuan {new THStack};
    sExJuan->SetTitle(HistConfig::ChangeTitle(HistConfig::Ex, "Juan's correction E_{x}").fTitle);
    auto* sDiff {new THStack};
    sDiff->SetTitle("Diff in #theta");
    std::vector<TH1D*> hsEx, hsDiff;
    std::vector<TH2D*> hsExRPx;
    // TGraphs for sigmas
    std::vector<TGraphErrors*> gs, gsj;
    std::vector<int> colors {9, 46};
    int idx {};
    for(int i = 0; i < dfs.size(); i++)
    {
        auto node =
            dfs[i]
                .Define("ThetaLightOK",
                        [&](float rpx, float thetalegacy)
                        {
                            // 1-> RP.X correction
                            auto temp {thetalegacy + func1->Eval(rpx)};
                            // 2-> Self correction not needed for elastic
                            return temp + func2->Eval(temp);
                        },
                        {"fRP.fCoordinates.fX", "fThetaLegacy"})
                .Define("Diff", "ThetaLightOK - fThetaLight")
                .Define("ThetaLightJuan",
                        [&](float rpx, float thetalegacy)
                        {
                            // 1-> RP.X correction
                            auto temp {thetalegacy + (-2.14353 + 0.0114464 * rpx - 8.52223e-5 * rpx * rpx)};
                            return temp + (-1.58175 + 0.0889058 * temp);
                        },
                        {"fRP.fCoordinates.fX", "fThetaLegacy"})
                .DefineSlot("ExOK",
                            [&, i](unsigned int slot, double EBeam, double EVertex, double thetaOK)
                            {
                                vks[i][slot].SetBeamEnergy(EBeam);
                                return vks[i][slot].ReconstructExcitationEnergy(EVertex, thetaOK * TMath::DegToRad());
                            },
                            {"EBeam", "EVertex", "ThetaLightOK"})
                .DefineSlot("ExJuan",
                            [&, i](unsigned int slot, double EBeam, double EVertex, double thetaJuan)
                            {
                                vks[i][slot].SetBeamEnergy(EBeam);
                                return vks[i][slot].ReconstructExcitationEnergy(EVertex, thetaJuan * TMath::DegToRad());
                            },
                            {"EBeam", "EVertex", "ThetaLightJuan"})
                .DefineSlot("ThetaCMOK",
                            [&, i](unsigned int slot, double EBeam, double EVertex, double thetaOK)
                            {
                                vks[i][slot].SetBeamEnergy(EBeam);
                                return vks[i][slot].ReconstructTheta3CMFromLab(EVertex, thetaOK * TMath::DegToRad()) *
                                       TMath::RadToDeg();
                            },
                            {"EBeam", "EVertex", "ThetaLightOK"});

        // Save histograms
        auto hEx {node.Histo1D(HistConfig::Ex, "ExOK")};
        hEx->SetTitle(labels[idx].c_str());
        hsEx.push_back((TH1D*)hEx->Clone());
        hEx->Scale(1. / hEx->Integral());
        // Fit
        std::vector<double> Exs {0., 1.6};
        if(idx == 0)
            Exs.pop_back();
        gs.push_back(new TGraphErrors);
        gs[idx]->SetTitle(labels[idx].c_str());
        double w {1.75};
        int p {};
        for(auto ex : Exs)
        {
            auto name {TString::Format("fit%d", p)};
            auto* func {new TF1 {name, "gaus", ex - w / 2, ex + w / 2}};
            func->SetLineColor(colors[idx]);
            hEx->Fit(func, "0QR+");
            hEx->GetFunction(name)->ResetBit(TF1::kNotDraw);
            gs[idx]->AddPoint(func->GetParameter(1), func->GetParameter(2));
            gs[idx]->SetPointError(gs[idx]->GetN() - 1, func->GetParError(1), func->GetParError(2));
            p++;
        }
        hEx->SetLineWidth(2);
        sEx->Add((TH1D*)hEx->Clone(), "hist");

        // Legacy Ex to compare
        auto hExLeg {node.Histo1D(HistConfig::Ex, "Ex")};

        // With Juan's correction for angle
        auto hExJuan {node.Histo1D(HistConfig::Ex, "ExJuan")};
        for(auto h : {hExLeg, hExJuan})
        {
            h->Scale(1. / h->Integral());
            h->SetLineWidth(2);
        }

        // Fit
        gsj.push_back(new TGraphErrors);
        gsj[idx]->SetTitle(labels[idx].c_str());
        p = 0;
        for(auto ex : Exs)
        {
            auto name {TString::Format("fitJ%d", p)};
            auto* func {new TF1 {name, "gaus", ex - w / 2, ex + w / 2}};
            func->SetLineColor(colors[idx]);
            hExLeg->Fit(func, "0QR+");
            hExLeg->GetFunction(name)->ResetBit(TF1::kNotDraw);
            gsj[idx]->AddPoint(func->GetParameter(1), func->GetParameter(2));
            gsj[idx]->SetPointError(gsj[idx]->GetN() - 1, func->GetParError(1), func->GetParError(2));
            p++;
        }
        sExLeg->Add((TH1D*)hExLeg->Clone(), "hist");
        sExJuan->Add((TH1D*)hExJuan->Clone(), "hist");

        // Ex vs RPx
        auto hExRPx {node.Histo2D(HistConfig::ExThetaCM, "ThetaCMOK", "ExOK")};
        hExRPx->SetTitle(labels[idx].c_str());
        hsExRPx.push_back((TH2D*)hExRPx->Clone());

        // Diff with new computed values
        auto hDiff {node.Histo1D("Diff")};
        hDiff->Scale(1. / hDiff->Integral());
        sDiff->Add((TH1D*)hDiff->Clone(), "hist");

        idx++;
    }

    // Build multigraphs
    auto* mg {new TMultiGraph};
    mg->SetTitle("#sigma comparison;E_{x} [MeV];#sigma [MeV]");
    int i {};
    for(auto& g : gs)
    {
        g->SetLineWidth(2);
        g->SetMarkerStyle(24);
        g->SetLineColor(colors[i]);
        g->SetMarkerColor(colors[i]);
        mg->Add(g);
        i++;
    }
    i = 0;
    for(auto& g : gsj)
    {
        g->SetLineWidth(2);
        g->SetLineStyle(2);
        g->SetMarkerStyle(25);
        g->SetLineColor(colors[i]);
        g->SetMarkerColor(colors[i]);
        mg->Add(g);
        i++;
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Check angle correction"}};
    c0->DivideSquare(6);
    c0->cd(1);
    sEx->Draw("nostack plc pmc");
    gPad->BuildLegend();
    DrawFuncs(sEx);
    c0->cd(2);
    sExLeg->Draw("nostack plc pmc");
    DrawFuncs(sExLeg);
    c0->cd(3);
    sExJuan->Draw("nostack plc pmc");
    c0->cd(4);
    mg->Draw("apl");
    c0->cd(5);
    sDiff->Draw("nostack plc pmc");
    // hsExRPx.front()->Draw("colz");
    // c0->cd(6);
    // hsExRPx.back()->Draw("colz");
}
