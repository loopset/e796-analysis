#include "ActKinematics.h"

#include "ROOT/RDF/InterfaceUtils.hxx"
#include "ROOT/RDataFrame.hxx"

#include "TCanvas.h"
#include "TF1.h"
#include "TFile.h"
#include "THStack.h"
#include "TMath.h"
#include "TVirtualPad.h"

#include <memory>
#include <vector>

#include "../../PostAnalysis/HistConfig.h"
#include "../../Selector/Selector.h"

void Check()
{
    // Read data
    ROOT::EnableImplicitMT();
    // pd
    ROOT::RDataFrame pd {"Final_Tree", gSelector->GetAnaFile(2, "20O", "1H", "2H", false)};
    // dt
    ROOT::RDataFrame dt {"Final_Tree", gSelector->GetAnaFile(2, "20O", "2H", "3H", false)};
    std::vector<ROOT::RDF::RNode> dfs {pd, dt};
    std::vector<std::string> labels {"(p,d)", "(d,t)"};

    // Create kinematics
    ActPhysics::Kinematics kpd {"20O(p,d)@700"};
    ActPhysics::Kinematics kdt {"20O(d,t)@700"};
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
    auto file {std::make_unique<TFile>("./Outputs/angle_corr_v0.root")};
    auto* func1 {file->Get<TF1>("func1")};
    auto* func2 {file->Get<TF1>("func2")};
    file->Close();

    // And define and plot
    auto* sEx {new THStack};
    sEx->SetTitle(HistConfig::Ex.fTitle);
    std::vector<TH1D*> hsEx;
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
                            // 2-> Self correction
                            return temp + func2->Eval(temp);
                        },
                        {"fRP.fCoordinates.fX", "fThetaLegacy"})
                .DefineSlot("ExOK",
                            [&, i](unsigned int slot, double EBeam, double EVertex, double thetaOK)
                            {
                                vks[i][slot].SetBeamEnergy(EBeam);
                                return vks[i][slot].ReconstructExcitationEnergy(EVertex, thetaOK * TMath::DegToRad());
                            },
                            {"EBeam", "EVertex", "ThetaLightOK"})
                .DefineSlot("ThetaCMOK",
                            [&, i](unsigned int slot, double EBeam, double EVertex, double thetaOK)
                            {
                                vks[i][slot].SetBeamEnergy(EBeam);
                                return vks[i][slot].ReconstructTheta3CMFromLab(EVertex, thetaOK * TMath::DegToRad());
                            },
                            {"EBeam", "EVertex", "ThetaLightOK"});

        // Save histograms
        auto hEx {node.Histo1D(HistConfig::Ex, "ExOK")};
        hEx->SetTitle(labels[idx].c_str());
        hsEx.push_back((TH1D*)hEx->Clone());
        // Another clone
        auto* cl {(TH1D*)hEx->Clone()};
        cl->SetLineWidth(2);
        cl->Scale(1. / cl->Integral());
        sEx->Add(cl, "hist");
        idx++;
    }

    // Draw
    auto* c0 {new TCanvas {"c0", "Check angle correction"}};
    c0->DivideSquare(4);
    c0->cd(1);
    sEx->Draw("nostack plc");
    gPad->BuildLegend();
    c0->cd(2);
    hsEx[0]->Draw();
    c0->cd(3);
    hsEx[1]->Draw();
}
